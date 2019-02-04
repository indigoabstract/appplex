#pragma once

#include "pfm.hxx"
#include "long-operation.hxx"
#include "recursive-dir.hxx"
#include "cmd-line-arg.hxx"
#include <filesystem>
#include <string>
#include <vector>

namespace bfs = std::filesystem;


class cmd_mod_dir_statistics : public cmd_mod_line
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	mws_sp<long_operation> run(const std::vector<unicodestring>& args);
};


class long_op_dir_statistics: public long_operation
{
public:
	long_op_dir_statistics(bfs::path& isrc_path);
	void run();

private:
	bfs::path src_path;
};
