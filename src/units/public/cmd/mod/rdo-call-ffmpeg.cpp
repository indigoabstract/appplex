#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "rdo-call-ffmpeg.hpp"
#include "min.hpp"
#include "com/util/unicode/conversions-util.hpp"
#include "com/util/unicode/boost-filesystem-util.hpp"
#include "com/util/unicode/boost-program-options-util.hpp"
#include "../start-process.hpp"
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <string>
#include <vector>

using namespace boost::filesystem;
using namespace boost::program_options;
using std::vector;
using std::string;


class rec_dir_op_flac_to_mp3: public recursive_dir_op
{
public:
	rec_dir_op_flac_to_mp3(bfs::path& iprocess_path, unicodestring& iprocess_arguments, bfs::path& isrc_path, bfs::path& idst_path,
		unicodestring& iformat_extension, int imilliseconds_to_wait);
	void on_start(shared_ptr<dir_node> dir);
	bool on_entering_dir(shared_ptr<dir_node> dir);
	void on_leaving_dir(shared_ptr<dir_node> dir);
	void apply_to_file(shared_ptr<file_node> file);

private:
	bfs::path process_path;
	unicodestring process_arguments;
	bfs::path src_path;
	bfs::path dst_path;
	unicodestring format_extension;
	int milliseconds_to_wait;
};


std::string mod_cmd_start_process::get_module_name()
{
	return "start-process";
}


const string PROCESS_PATH			= "process-path";
const string PROCESS_ARGUMENTS		= "process-arguments";
const string SOURCE_PATH			= "source-path";
const string DESTINATION_PATH		= "destination-path";
const string FORMAT_EXTENSION		= "format-extension";
const string WAIT_TIME				= "wait-time";

boost::program_options::options_description mod_cmd_start_process::get_options_description()
{
	options_description desc(trs("available options for module [{}]", get_module_name()));

	desc.add_options()
		(PROCESS_PATH.c_str(), unicodevalue<unicodestring>()->required(), "process path")
		(PROCESS_ARGUMENTS.c_str(), unicodevalue<unicodestring>()->required(), "process arguments")
		(SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "source path")
		(DESTINATION_PATH.c_str(), unicodevalue<unicodestring>()->required(), (trs("destination path."
		" a relative path is considered relative to the [{}] directory", SOURCE_PATH)).c_str())
		(FORMAT_EXTENSION.c_str(), unicodevalue<unicodestring>(), "format extension (without dot)")
		(WAIT_TIME.c_str(), value<int>()->default_value(60), "number of seconds to wait for the process to finish")
		;

	return desc;
}

shared_ptr<long_operation> mod_cmd_start_process::run(const vector<unicodestring>& args)
{
	options_description desc = get_options_description();
	variables_map vm;
	unicode_parsed_options parsed = unicode_command_line_parser(args).options(desc).run();
	unicodestring format_extension = untr("");

	store(parsed, vm);
	notify(vm);

	utrx(untr("process-path was set to {}"), vm[PROCESS_PATH].as<unicodestring>());
	utrx(untr("process-arguments was set to {}"), vm[PROCESS_ARGUMENTS].as<unicodestring>());
	trx("wait-time was set to {}", vm[WAIT_TIME].as<int>());

	if(!vm[FORMAT_EXTENSION].empty())
	{
		format_extension = vm[FORMAT_EXTENSION].as<unicodestring>();
		utrx(untr("format extension was set to {}"), format_extension);
	}

	boost::filesystem::path process_path(vm[PROCESS_PATH].as<unicodestring>());
	unicodestring process_arguments = vm[PROCESS_ARGUMENTS].as<unicodestring>();
	boost::filesystem::path src_path(vm[SOURCE_PATH].as<unicodestring>());
	boost::filesystem::path dst_path(vm[DESTINATION_PATH].as<unicodestring>());
	int milliseconds_to_wait = 1000 * vm[WAIT_TIME].as<int>();

	if (dst_path.is_relative())
	{
		dst_path = src_path / dst_path;
	}

	shared_ptr<long_operation> lop = shared_ptr<long_operation>(new long_op_ffmpeg(process_path, process_arguments, src_path, dst_path, format_extension, milliseconds_to_wait));

	return lop;
}


long_op_ffmpeg::long_op_ffmpeg(bfs::path& iprocess_path, unicodestring& iprocess_arguments, bfs::path& isrc_path, bfs::path& idst_path,
	unicodestring& iformat_extension, int imilliseconds_to_wait)
{
	process_path = iprocess_path;
	process_arguments = iprocess_arguments;
	src_path = isrc_path;
	dst_path = idst_path;
	format_extension = iformat_extension;
	milliseconds_to_wait = imilliseconds_to_wait;
}

void long_op_ffmpeg::run()
{
	if (exists(process_path))
	{
		if (!is_regular_file(process_path))
		{
			throw ia_exception(trs("longOpFfmpeg: {} is not a regular file", process_path.string()));
		}
	}
	else
	{
		throw ia_exception(trs("longOpFfmpeg: {} does not exist", process_path.string()));
	}

	if (exists(src_path))
	{
		if (is_directory(src_path))
		{
			utrx(untr("starting longOpFfmpeg from [{0}] to [{1}]"), path2string(src_path), path2string(dst_path));

			shared_ptr<directory_tree> dirtree = directory_tree::new_directory_tree(src_path);
			rec_dir_op_flac_to_mp3 rdo(process_path, process_arguments, src_path, dst_path, format_extension, milliseconds_to_wait);

			dirtree->recursive_apply(rdo);
		}
		else
		{
			throw ia_exception(trs("longOpFfmpeg: {} is not a directory", src_path.string()));
		}
	}
	else
	{
		throw ia_exception(trs("longOpFfmpeg: {} does not exist", src_path.string()));
	}
}


rec_dir_op_flac_to_mp3::rec_dir_op_flac_to_mp3(bfs::path& iprocess_path, unicodestring& iprocess_arguments, bfs::path& isrc_path, bfs::path& idst_path,
	unicodestring& iformat_extension, int imilliseconds_to_wait)
{
	process_path = iprocess_path;
	process_arguments = iprocess_arguments;
	src_path = isrc_path;
	dst_path = idst_path;
	format_extension = iformat_extension;
	milliseconds_to_wait = imilliseconds_to_wait;
}

void rec_dir_op_flac_to_mp3::on_start(shared_ptr<dir_node> dir)
{
	bool result = create_directory(dst_path);

	if(!result && !exists(dst_path))
	{
		throw ia_exception(trs("failed to create directory {}!", dst_path.string()));
	}
}

bool rec_dir_op_flac_to_mp3::on_entering_dir(shared_ptr<dir_node> dir)
{
	if(dir->abs_dir_path != dst_path)
	{
		path tp = dst_path / dir->rel_dir_path;

		//trx("onEnteringDir %1%") % tp;
		bool result = create_directory(tp);

		if(!result && !exists(dst_path))
		{
			throw ia_exception(trs("failed to create directory {}!", tp.string()));
		}

		return true;
	}

	return false;
}

void rec_dir_op_flac_to_mp3::on_leaving_dir(shared_ptr<dir_node> dir)
{
}

void rec_dir_op_flac_to_mp3::apply_to_file(shared_ptr<file_node> file)
{
	static const unicodestring REPL_SOURCE_PATH			= utrs(untr("[{}]"), string2unicodestring(SOURCE_PATH));
	static const unicodestring REPL_DESTINATION_PATH	= utrs(untr("[{}]"), string2unicodestring(DESTINATION_PATH));

	unicodestring ext = path2string(file->rel_file_path.extension());

	//if(ext == untr(".flac") || ext == untr(".mp3"))
	{
		path ts = src_path / file->rel_file_path;
		path td = dst_path / file->rel_file_path;

		if(format_extension.length() > 0)
		{
			td.replace_extension(format_extension);
		}

		unicodestring tss = path2string(ts);
		unicodestring tds = path2string(td);
		unicodestring exec = process_arguments;

		boost::algorithm::replace_all(exec, REPL_SOURCE_PATH, tss);
		boost::algorithm::replace_all(exec, REPL_DESTINATION_PATH, tds);
		exec = utrs(untr("{0} {1}"), process_path.native(), exec);
		//utrx(exec);
		trn();

		start_process::exe_shell(exec, milliseconds_to_wait);
	}
}

#endif
