#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "rdo-directory-statistics.hpp"
#include "min.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include "rdo-recursive-copy.hpp"
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


class rec_dir_op_dir_statistics: public recursive_dir_op
{
public:
	rec_dir_op_dir_statistics(bfs::path& isrc_dir);
	void on_start(shared_ptr<dir_node> dir);
	void on_finish(shared_ptr<dir_node> dir);
	bool on_entering_dir(shared_ptr<dir_node> dir);
	void on_leaving_dir(shared_ptr<dir_node> dir);
	void apply_to_file(shared_ptr<file_node> file);

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


std::string mod_cmd_dir_statistics::get_module_name()
{
	return "directory-statistics";
}


const string SOURCE_PATH			= "source-path";


boost::program_options::options_description mod_cmd_dir_statistics::get_options_description()
{
	options_description desc(trs("available options for module [%1%]") % get_module_name());

	desc.add_options()
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path. must be an absolute path")
	;

	return desc;
}

shared_ptr<long_operation> mod_cmd_dir_statistics::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).run();

	store(parsed, vm);
	notify(vm);

	boost::filesystem::path srcPath(vm[SOURCE_PATH].as<unicodestring>());

	shared_ptr<long_operation> lop = shared_ptr<long_operation>(new long_op_dir_statistics(srcPath));

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
			utrx(untr("starting longOpDirStatistics in directory [%1%]")) % path2string(src_path);

			shared_ptr<directory_tree> dirtree = directory_tree::new_directory_tree(src_path);
			rec_dir_op_dir_statistics rdo(src_path);

			dirtree->recursive_apply(rdo);
		}
		else
		{
			throw ia_exception(trs("longOpDirStatistics: %1% is not a directory") % src_path);
		}
	}
	else
	{
		throw ia_exception(trs("longOpDirStatistics: %1% does not exist") % src_path);
	}
}


rec_dir_op_dir_statistics::rec_dir_op_dir_statistics(path& isrc_dir)
{
	src_dir = isrc_dir;
}

void rec_dir_op_dir_statistics::on_start(shared_ptr<dir_node> dir)
{
	file_count = directory_count = total_file_size = 0;
    min_file_size = 0xffffffffU;
	max_file_size = 0;
}

void rec_dir_op_dir_statistics::on_finish(shared_ptr<dir_node> dir)
{
	trx("directories [%1%], files [%2%], all-file size [%3%]") % directory_count % file_count % total_file_size;
	utrx(untr("smallest file [%1%] size [%2%], largest file [%3%] size [%4%]")) % path2string(min_file) % min_file_size % path2string(max_file) % max_file_size;
}

bool rec_dir_op_dir_statistics::on_entering_dir(shared_ptr<dir_node> dir)
{
	return true;
}

void rec_dir_op_dir_statistics::on_leaving_dir(shared_ptr<dir_node> dir)
{
	directory_count++;
}

void rec_dir_op_dir_statistics::apply_to_file(shared_ptr<file_node> file)
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

#endif
