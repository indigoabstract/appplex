#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "recursive-dir.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "min.hpp"
#include <boost/algorithm/string.hpp>
#include <string>

using namespace boost::algorithm;
using namespace boost::filesystem;


void recursive_dir_op::on_start(shared_ptr<dir_node> dir)
{
}

void recursive_dir_op::on_finish(shared_ptr<dir_node> dir)
{
}

bool recursive_dir_op::on_entering_dir(shared_ptr<dir_node> dir)
{
	return true;
}

void recursive_dir_op::on_leaving_dir(shared_ptr<dir_node> dir)
{
}

void recursive_dir_op::apply_to_file(shared_ptr<file_node> file)
{
}


rdo_list_files::rdo_list_files()
{
}

void rdo_list_files::on_start(shared_ptr<dir_node> dir)
{
	file_count = directory_count = total_file_size = 0;
	utrx(untr("started file list for directory [%1%]")) % path2string(dir->abs_dir_path);
}

void rdo_list_files::on_finish(shared_ptr<dir_node> dir)
{
	utrx(untr("finished file list for directory [%1%]")) % path2string(dir->abs_dir_path);
	utrx(untr("total directories [%1%], total files [%2%], total file size [%3%]")) % directory_count % file_count % total_file_size;
}

bool rdo_list_files::on_entering_dir(shared_ptr<dir_node> dir)
{
	utrx(untr("directory [%1%]\n[")) % path2string(dir->rel_dir_path);

	return true;
}

void rdo_list_files::on_leaving_dir(shared_ptr<dir_node> dir)
{
	utrx(untr("]"));
	directory_count++;
}

void rdo_list_files::apply_to_file(shared_ptr<file_node> file)
{
	utrx(untr("[%1%]")) % path2string(file->rel_file_path.filename());
	file_count++;
	total_file_size += file->file_size;
}


directory_tree::directory_tree()
{
}

shared_ptr<directory_tree> directory_tree::new_directory_tree(path& iroot_path, const std::vector<unicodestring>& iexclude_path)
{
	if (!exists(iroot_path))
	{
		utrx(untr("error [%1% doesn't exist]")) % iroot_path;
		return shared_ptr<directory_tree>();
	}

	if (!is_directory(iroot_path))
	{
		utrx(untr("error [%1% is not a directory]")) % iroot_path;
		return shared_ptr<directory_tree>();
	}

	shared_ptr<directory_tree> d(new directory_tree());
	shared_ptr<dir_node> rootNode(new dir_node());

	rootNode->abs_dir_path = iroot_path;
	rootNode->root = rootNode;
	d->root_node = rootNode;

	for(unicodestring s : iexclude_path)
	{
		path& p = path(s);

		if (p.is_relative())
		{
			d->exclude_path.push_back(p);
		}
		else
		{
			throw ia_exception(trs("path [%1%] is not relative to [%2%]") % p.string() % iroot_path.string());
		}
	}

	directory_iterator dit(iroot_path);

	d->recursive_read(d, d->root_node, dit);

	return d;
}

void directory_tree::recursive_apply(recursive_dir_op& rdo)
{
	rdo.on_start(root_node);
	recursive_apply(root_node, rdo);
	rdo.on_finish(root_node);
}

shared_ptr<dir_node> directory_tree::get_root_node()
{
	return root_node;
}

void directory_tree::recursive_apply(shared_ptr<dir_node> dtn, recursive_dir_op& rdo)
{
	for (int k = 0; k < dtn->directories.size(); k++)
	{
		shared_ptr<dir_node> rd = dtn->directories[k];

		bool apply = rdo.on_entering_dir(rd);

		if (apply)
		{
			recursive_apply(rd, rdo);
		}

		rdo.on_leaving_dir(rd);
	}

	for (int k = 0; k < dtn->files.size(); k++)
	{
		shared_ptr<file_node> fn = dtn->files[k];

		rdo.apply_to_file(fn);
	}
}

void directory_tree::recursive_read(shared_ptr<directory_tree> dt, shared_ptr<dir_node> dtn, directory_iterator& di)
{
	static directory_iterator end_iter;

	while (di != end_iter)
	{
		const path& abs_path = di->path();
		unicodestring relativePath = path2string(abs_path);
		shared_ptr<dir_node> root = dtn->root.lock();

		erase_all(relativePath, path2string(root->abs_dir_path));
		path rel_path(relativePath);

		if (is_regular_file(abs_path))
		{
			shared_ptr<file_node> fn(new file_node());

			fn->abs_file_path = abs_path;
			fn->rel_file_path = rel_path.relative_path();
			fn->file_size = file_size(abs_path);
			fn->parent_dir = dtn;

			dtn->files.push_back(fn);
			//trx("file [%1%]") % fn->rel_file_path.string();
		}
		else if (is_directory(abs_path))
		{
			shared_ptr<dir_node> dx(new dir_node());
			directory_iterator dit(abs_path);

			dx->abs_dir_path = abs_path;
			dx->rel_dir_path = rel_path.relative_path();
			dx->parent = dtn;
			dx->root = dtn->root;

			dtn->directories.push_back(dx);
			//trx("dir [%1%]") % dx->rel_dir_path.string();

			bool skip = false;

			for(path p : dt->exclude_path)
			{
				if (p == dx->rel_dir_path)
				{
					skip = true;
					//trx("dir [%1%]") % dx->rel_dir_path.string();
					break;
				}
			}

			if (!skip)
			{
				recursive_read(dt, dx, dit);
			}
		}

		di++;
	}
}

#endif
