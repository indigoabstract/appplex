#include "stdafx.hxx"

#include "appplex-conf.hxx"

#ifdef UNIT_CMD

#include "rdo-recursive-copy.hxx"
#include "min.hxx"
#include "com/util/unicode/boost-filesystem-util.hxx"
#include "com/util/unicode/boost-program-options-util.hxx"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <exception>
#include <locale>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace boost::filesystem;
using namespace boost::program_options;
using std::string;
using std::vector;


class rec_dir_op_copy: public recursive_dir_op
{
public:
	rec_dir_op_copy(bfs::path& isrc_path, bfs::path& idst_path);
	void on_start(mws_sp<dir_node> dir);
	void on_finish(mws_sp<dir_node> dir);
	bool on_entering_dir(mws_sp<dir_node> dir);
	void on_leaving_dir(mws_sp<dir_node> dir);
	void apply_to_file(mws_sp<file_node> file);

private:
	static void copy_path(bfs::path& srcPath, bfs::path& dstPath, bool isDirectory);

	bfs::path src_path;
	bfs::path dst_path;
	int file_count;
	int directory_count;
	int total_file_size;
};


std::string mod_cmd_recursive_copy::get_module_name()
{
	return "recursive-copy";
}


const string SOURCE_PATH			= "source-path";
const string DESTINATION_PATH		= "destination-path";


boost::program_options::options_description mod_cmd_recursive_copy::get_options_description()
{
	options_description desc(trs("available options for module [{}]", get_module_name()));

	desc.add_options()
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
		(DESTINATION_PATH.c_str(), unicodevalue<unicodestring>()->default_value(untr(""), ""),
		(trs("destination path. a relative path is considered relative to the [{}] directory", SOURCE_PATH)).c_str())
		;

	return desc;
}

mws_sp<long_operation> mod_cmd_recursive_copy::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).run();

	store(parsed, vm);
	notify(vm);

	boost::filesystem::path srcPath(vm[SOURCE_PATH].as<unicodestring>());
	boost::filesystem::path dstPath;

	utrx(untr("source-path was set to {}"), vm[SOURCE_PATH].as<unicodestring>());
	utrx(untr("destination-path was set to {}"), vm[DESTINATION_PATH].as<unicodestring>());

	dstPath = vm[DESTINATION_PATH].as<unicodestring>();

	if (dstPath.is_relative())
	{
		dstPath = srcPath / dstPath;
	}

	mws_sp<long_operation> lop = mws_sp<long_operation>(new long_op_recursive_copy(srcPath, dstPath));

	return lop;
}


long_op_recursive_copy::long_op_recursive_copy(path& isrc_path, path& idst_path)
{
	src_path = isrc_path;
	dst_path = idst_path;
}

void long_op_recursive_copy::run()
{
	if (exists(src_path))
	{
		if (is_directory(src_path))
		{
			if(src_path == dst_path)
			{
            mws_throw mws_exception(trs("longOpRecursiveCopy: cannot copy a directory to itself. [{0}] must not be the same as [{1}]", SOURCE_PATH, DESTINATION_PATH));
			}

			utrx(untr("starting longOpRecursiveCopy from [{0}] to [{1}]"), path2string(src_path), path2string(dst_path));

			mws_sp<directory_tree> dirtree = directory_tree::new_directory_tree(src_path);
			rec_dir_op_copy rdo(src_path, dst_path);

			dirtree->recursive_apply(rdo);
		}
		else
		{
         mws_throw mws_exception(trs("longOpRecursiveCopy: {} is not a directory", src_path.string()));
		}
	}
	else
	{
      mws_throw mws_exception(trs("longOpRecursiveCopy: {} does not exist", src_path.string()));
	}
}


rec_dir_op_copy::rec_dir_op_copy(path& iSrcPath, path& iDstPath)
{
	src_path = iSrcPath;
	dst_path = iDstPath;
}

void rec_dir_op_copy::on_start(mws_sp<dir_node> dir)
{
	file_count = directory_count = total_file_size = 0;

	bool result = create_directory(dst_path);

	if(!result && !exists(dst_path))
	{
      mws_throw mws_exception(trs("failed to create directory {}!", dst_path.string()));
	}

	utrx(untr("copying files from directory [{0}] to [{1}]"), path2string(src_path), path2string(dst_path));
}

void rec_dir_op_copy::on_finish(mws_sp<dir_node> dir)
{
	utrx(untr("total directories [{0}], total files [{1}], total file size [{2}]"), directory_count, file_count, total_file_size);
}

bool rec_dir_op_copy::on_entering_dir(mws_sp<dir_node> dir)
{
	path sp = src_path / dir->rel_dir_path;
	path dp = dst_path / dir->rel_dir_path;

	copy_path(sp, dp, true);

	return true;
}

void rec_dir_op_copy::on_leaving_dir(mws_sp<dir_node> dir)
{
	path sp = src_path / dir->rel_dir_path;
	path dp = dst_path / dir->rel_dir_path;

	if(exists(dp))
		// retain timestamp on dirs
	{
		std::time_t tt = last_write_time(sp);
		last_write_time(dp, tt);
	}

	directory_count++;
}

void rec_dir_op_copy::apply_to_file(mws_sp<file_node> file)
{
	path sp = src_path / file->rel_file_path;
	path dp = dst_path / file->rel_file_path;

	copy_path(sp, dp, false);
	file_count++;
	total_file_size += file->file_size;
}

void rec_dir_op_copy::copy_path(bfs::path& srcp, bfs::path& dstp, bool iis_directory)
{
	if(!exists(dstp))
	{
		if(iis_directory)
		{
			utrx(untr("copying directory from [{0}] to [{1}]"), path2string(srcp), path2string(dstp));
			copy_directory(srcp, dstp);
		}
		else
		{
			utrx(untr("copying file from [{0}] to [{1}]"), path2string(srcp), path2string(dstp));
			copy_file(srcp, dstp);
		}
	}
}

#endif
