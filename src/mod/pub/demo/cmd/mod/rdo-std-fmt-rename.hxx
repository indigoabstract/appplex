#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_CMD

#include "pfm.hxx"
#include "../long-operation.hxx"
#include "../recursive-dir.hxx"
#include "../cmd-line-arg.hxx"
#include <boost/filesystem.hpp>
#include <string>
#include <vector>

namespace bfs = ::boost::filesystem;


class mod_cmd_std_fmt_filenames : public mod_cmd
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	mws_sp<long_operation> run(const std::vector<unicodestring>& args);
};


class long_op_std_fmt_rename: public long_operation
{
public:
	long_op_std_fmt_rename(bfs::path& isrc_path, bfs::path& idst_path, bool icopy_only = false);
	void run();

private:
	bfs::path src_path;
	bfs::path dst_path;
	bool copy_only;
};

#endif