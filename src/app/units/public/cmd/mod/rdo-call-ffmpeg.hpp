#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include "../long-operation.hpp"
#include "../recursive-dir.hpp"
#include "../cmd-line-arg.hpp"
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace bfs = ::boost::filesystem;
using std::vector;


class mod_cmd_start_process : public mod_cmd
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	shared_ptr<long_operation> run(const vector<unicodestring>& args);
};


class long_op_ffmpeg: public long_operation
{
public:
	long_op_ffmpeg(bfs::path& iprocess_path, unicodestring& iprocess_arguments, bfs::path& isrc_path, bfs::path& idst_path,
		unicodestring& format_extension, int imilliseconds_to_wait);
	void run();

private:
	bfs::path process_path;
	unicodestring process_arguments;
	bfs::path src_path;
	bfs::path dst_path;
	unicodestring format_extension;
	int milliseconds_to_wait;
};

#endif
