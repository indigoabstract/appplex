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


class mod_cmd_std_fmt_filenames : public mod_cmd
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	shared_ptr<long_operation> run(const std::vector<unicodestring>& args);
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
