#pragma once

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


class cmd_mod_line
{
public:
	virtual std::string get_module_name() = 0;
	virtual boost::program_options::options_description get_options_description() = 0;
	virtual mws_sp<long_operation> run(const std::vector<unicodestring>& args) = 0;
};
