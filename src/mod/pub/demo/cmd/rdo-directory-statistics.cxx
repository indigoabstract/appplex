#include "stdafx.hxx"

#include "rdo-directory-statistics.hxx"
#include "min.hxx"
#include "util/unicode/boost-filesystem-util.hxx"
#include "util/unicode/boost-program-options-util.hxx"
#include "rdo-recursive-copy.hxx"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <exception>
#include <filesystem>
#include <locale>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace std::filesystem;
using namespace boost::program_options;
using std::string;
using std::vector;


class rec_dir_op_dir_statistics: public recursive_dir_op
{
public:
	rec_dir_op_dir_statistics(bfs::path& isrc_dir);
	void on_start(mws_sp<dir_node> dir);
	void on_finish(mws_sp<dir_node> dir);
	bool on_entering_dir(mws_sp<dir_node> dir);
	void on_leaving_dir(mws_sp<dir_node> dir);
	void apply_to_file(mws_sp<file_node> file);

private:
	bfs::path src_dir;
	int file_count;
	int directory_count;
	uint64 total_file_size;
	uint64 max_file_size;
	path max_file;
	uint64 min_file_size;
	path min_file;
};


std::string cmd_mod_dir_statistics::get_module_name()
{
	return "directory-statistics";
}


const string SOURCE_PATH			= "source-path";


boost::program_options::options_description cmd_mod_dir_statistics::get_options_description()
{
	options_description desc(trs("available options for module [{}]", get_module_name()));

	desc.add_options()
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
	;

	return desc;
}

mws_sp<long_operation> cmd_mod_dir_statistics::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).run();

	store(parsed, vm);
	notify(vm);

   std::filesystem::path srcPath(vm[SOURCE_PATH].as<unicodestring>());

	mws_sp<long_operation> lop = mws_sp<long_operation>(new long_op_dir_statistics(srcPath));

	return lop;
}


long_op_dir_statistics::long_op_dir_statistics(path& isrc_path)
{
	src_path = isrc_path;
}

void long_op_dir_statistics::run()
{
	if (exists(src_path))
	{
		if (is_directory(src_path))
		{
			utrx(untr("starting longOpDirStatistics in directory [{}]"), path2string(src_path));

			mws_sp<directory_tree> dirtree = directory_tree::new_directory_tree(src_path);
			rec_dir_op_dir_statistics rdo(src_path);

			dirtree->recursive_apply(rdo);
		}
		else
		{
         mws_throw mws_exception(trs("longOpDirStatistics: {} is not a directory", src_path.string()));
		}
	}
	else
	{
      mws_throw mws_exception(trs("longOpDirStatistics: {} does not exist", src_path.string()));
	}
}


rec_dir_op_dir_statistics::rec_dir_op_dir_statistics(path& isrc_dir)
{
	src_dir = isrc_dir;
}

void rec_dir_op_dir_statistics::on_start(mws_sp<dir_node> dir)
{
	file_count = directory_count = total_file_size = 0;
    min_file_size = 0xffffffffU;
	max_file_size = 0;
}

void rec_dir_op_dir_statistics::on_finish(mws_sp<dir_node> dir)
{
	trx("directories [{0}], files [{1}], all-file size [{2}]", directory_count, file_count, total_file_size);
	utrx(untr("smallest file [{0}] size [{1}], largest file [{2}] size [{3}]"), path2string(min_file), min_file_size, path2string(max_file), max_file_size);
}

bool rec_dir_op_dir_statistics::on_entering_dir(mws_sp<dir_node> dir)
{
	return true;
}

void rec_dir_op_dir_statistics::on_leaving_dir(mws_sp<dir_node> dir)
{
	directory_count++;
}

void rec_dir_op_dir_statistics::apply_to_file(mws_sp<file_node> file)
{
	uint64 fileSize = file_size(file->abs_file_path);

	if(fileSize > max_file_size)
	{
		max_file_size = fileSize;
		max_file = file->rel_file_path;
	}
	else if(fileSize < min_file_size)
	{
		min_file_size = fileSize;
		min_file = file->rel_file_path;
	}

	total_file_size += fileSize;
	file_count++;
}
