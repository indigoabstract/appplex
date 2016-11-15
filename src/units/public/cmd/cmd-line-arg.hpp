#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include "long-operation.hpp"
#include <boost/program_options.hpp>
#include <string>
#include <vector>


class cmd_line_arg
{
public:
	static shared_ptr<long_operation> run(const std::vector<unicodestring>& args);
};


class mod_cmd
{
public:
	virtual std::string get_module_name() = 0;
	virtual boost::program_options::options_description get_options_description() = 0;
	virtual shared_ptr<long_operation> run(const std::vector<unicodestring>& args) = 0;
};

#endif
