#pragma once

#include "pfm.hxx"
#include "long-operation.hxx"
#include "recursive-dir.hxx"
#include "cmd-line-arg.hxx"
#include <filesystem>
#include <string>
#include <vector>


namespace fs = std::filesystem;


class cmd_mod_bin2c : public cmd_mod_line
{
public:
   std::string get_module_name();
   boost::program_options::options_description get_options_description();
   mws_sp<long_operation> run(const std::vector<unicodestring>& i_args);
};


class long_op_bin2c : public long_operation
{
public:
   long_op_bin2c(unicodestring i_src_path, unicodestring i_output_path);
   void run();

private:
   fs::path src_path;
   fs::path output_path;
};
