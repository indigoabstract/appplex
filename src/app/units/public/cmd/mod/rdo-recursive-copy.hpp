#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include "../long-operation.hpp"
#include "../recursive-dir.hpp"
#include "../cmd-line-arg.hpp"
#include <boost/filesystem.hpp>
#include <string>

namespace bfs = ::boost::filesystem;


class mod_cmd_recursive_copy : public mod_cmd
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	mws_sp<long_operation> run(const std::vector<unicodestring>& args);
};


class long_op_recursive_copy: public long_operation
{
public:
	long_op_recursive_copy(bfs::path& isrc_path, bfs::path& idst_path);
	void run();

private:
	bfs::path src_path;
	bfs::path dst_path;
};

#endif
