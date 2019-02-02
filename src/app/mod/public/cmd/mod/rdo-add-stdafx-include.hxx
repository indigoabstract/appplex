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


class mod_cmd_add_stdafx_include : public mod_cmd
{
public:
	std::string get_module_name();
	boost::program_options::options_description get_options_description();
	mws_sp<long_operation> run(const std::vector<unicodestring>& iargs);
};


class long_op_add_stdafx_include: public long_operation
{
public:
	long_op_add_stdafx_include(unicodestring isrc_path, std::vector<unicodestring> iexclude_path);
	void run();

private:
	bfs::path src_path;
	std::vector<unicodestring> exclude_path;
};

#endif
