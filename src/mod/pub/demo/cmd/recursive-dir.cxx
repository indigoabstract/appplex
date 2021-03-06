#include "stdafx.hxx"

#include "recursive-dir.hxx"
#include "util/unicode/boost-filesystem-util.hxx"
#include "krn.hxx"
#include <boost/algorithm/string.hpp>
#include <string>

using namespace boost::algorithm;
using namespace std::filesystem;


void recursive_dir_op::on_start(mws_sp<dir_node> dir)
{
}

void recursive_dir_op::on_finish(mws_sp<dir_node> dir)
{
}

bool recursive_dir_op::on_entering_dir(mws_sp<dir_node> dir)
{
	return true;
}

void recursive_dir_op::on_leaving_dir(mws_sp<dir_node> dir)
{
}

void recursive_dir_op::apply_to_file(mws_sp<file_node> file)
{
}


rdo_list_files::rdo_list_files()
{
}

void rdo_list_files::on_start(mws_sp<dir_node> dir)
{
	file_count = directory_count = total_file_size = 0;
	utrx(untr("started file list for directory [{}]"), path2string(dir->abs_dir_path));
}

void rdo_list_files::on_finish(mws_sp<dir_node> dir)
{
	utrx(untr("finished file list for directory [{}]"), path2string(dir->abs_dir_path));
	utrx(untr("total directories [{0}], total files [{1}], total file size [{2}]"), directory_count, file_count, total_file_size);
}

bool rdo_list_files::on_entering_dir(mws_sp<dir_node> dir)
{
	utrx(untr("directory [{}]\n["), path2string(dir->rel_dir_path));

	return true;
}

void rdo_list_files::on_leaving_dir(mws_sp<dir_node> dir)
{
	utrx(untr("]"));
	directory_count++;
}

void rdo_list_files::apply_to_file(mws_sp<file_node> file)
{
	utrx(untr("[{}]"), path2string(file->rel_file_path.filename()));
	file_count++;
	total_file_size += file->file_size;
}


directory_tree::directory_tree()
{
}

mws_sp<directory_tree> directory_tree::new_directory_tree(path& iroot_path, const std::vector<unicodestring>& iexclude_path)
{
	if (!exists(iroot_path))
	{
		utrx(untr("error [{} doesn't exist]"), iroot_path.native());
		return mws_sp<directory_tree>();
	}

	if (!is_directory(iroot_path))
	{
		utrx(untr("error [{} is not a directory]"), iroot_path.native());
		return mws_sp<directory_tree>();
	}

	mws_sp<directory_tree> d(new directory_tree());
	mws_sp<dir_node> rootNode(new dir_node());

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
         mws_throw mws_exception(trs("path [{0}] is not relative to [{1}]", p.string(), iroot_path.string()));
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

mws_sp<dir_node> directory_tree::get_root_node()
{
	return root_node;
}

void directory_tree::recursive_apply(mws_sp<dir_node> dtn, recursive_dir_op& rdo)
{
	for (int k = 0; k < dtn->directories.size(); k++)
	{
		mws_sp<dir_node> rd = dtn->directories[k];

		bool apply = rdo.on_entering_dir(rd);

		if (apply)
		{
			recursive_apply(rd, rdo);
		}

		rdo.on_leaving_dir(rd);
	}

	for (int k = 0; k < dtn->files.size(); k++)
	{
		mws_sp<file_node> fn = dtn->files[k];

		rdo.apply_to_file(fn);
	}
}

void directory_tree::recursive_read(mws_sp<directory_tree> dt, mws_sp<dir_node> dtn, directory_iterator& di)
{
	static directory_iterator end_iter;

	while (di != end_iter)
	{
		const path& abs_path = di->path();
		unicodestring relativePath = path2string(abs_path);
		mws_sp<dir_node> root = dtn->root.lock();

		erase_all(relativePath, path2string(root->abs_dir_path));
		path rel_path(relativePath);

		if (is_regular_file(abs_path))
		{
			mws_sp<file_node> fn(new file_node());

			fn->abs_file_path = abs_path;
			fn->rel_file_path = rel_path.relative_path();
			fn->file_size = file_size(abs_path);
			fn->parent_dir = dtn;

			dtn->files.push_back(fn);
			//trx("file [%1%]") % fn->rel_file_path.string();
		}
		else if (is_directory(abs_path))
		{
			mws_sp<dir_node> dx(new dir_node());
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
