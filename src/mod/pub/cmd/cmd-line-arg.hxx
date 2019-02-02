#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_CMD

#include "pfm.hxx"
#include "long-operation.hxx"
#include <boost/program_options.hpp>
#include <string>
#include <vector>


class cmd_line_arg
{
public:
	static mws_sp<long_operation> run(const std::vector<unicodestring>& args);
};


class mod_cmd
{
public:
	virtual std::string get_module_name() = 0;
	virtual boost::program_options::options_description get_options_description() = 0;
	virtual mws_sp<long_operation> run(const std::vector<unicodestring>& args) = 0;
};

#endif
