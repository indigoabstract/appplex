#include "stdafx.hxx"

#include "cmd-mod-bin2c.hxx"
#include "krn.hxx"
#include "util/unicode/boost-filesystem-util.hxx"
#include "util/unicode/boost-program-options-util.hxx"
#include "rdo-recursive-copy.hxx"
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/find.hpp>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <locale>
#include <string>
#include <vector>

using namespace boost::algorithm;
using namespace boost::program_options;


std::string cmd_mod_bin2c::get_module_name()
{
   return "bin2c";
}


const std::string SOURCE_PATH = "source";
const std::string OUTPUT_PATH = "output";


boost::program_options::options_description cmd_mod_bin2c::get_options_description()
{
   options_description desc(trs("converts a binary file to a C array. available options for module [{}]", get_module_name()));

   desc.add_options()
      (SOURCE_PATH.c_str(), unicodevalue<unicodestring>()->required(), "path to the binary source file.")
      (OUTPUT_PATH.c_str(), unicodevalue<unicodestring>(), "output file path. optional. name & path to the output file")
      ;

   return desc;
}

mws_sp<long_operation> cmd_mod_bin2c::run(const std::vector<unicodestring>& i_args)
{
   options_description desc = get_options_description();
   variables_map vm;
   positional_options_description p;

   p.add(OUTPUT_PATH.c_str(), -1);
   unicode_parsed_options parsed = unicode_command_line_parser(i_args).options(desc).positional(p).run();

   store(parsed, vm);
   notify(vm);

   unicodestring output_path;

   if (vm.count(OUTPUT_PATH))
   {
      output_path = vm[OUTPUT_PATH].as<unicodestring>();
   }

   mws_sp<long_operation> lop = mws_sp<long_operation>(new long_op_bin2c(vm[SOURCE_PATH].as<unicodestring>(), output_path));

   return lop;
}


long_op_bin2c::long_op_bin2c(unicodestring i_src_path, unicodestring i_output_path)
{
   src_path = std::filesystem::path(i_src_path);
   output_path = i_output_path;

   if (output_path.empty())
   {
      output_path = "bin-array.c";
   }
}

void convert_bin_2_c_array(const std::string& i_src_path, const std::string& i_dst_path)
{
   FILE* f = fopen(i_src_path.c_str(), "rb");
   FILE* fw = fopen(i_dst_path.c_str(), "w");
   uint32_t max_column_count = 0;
   uintmax_t size = fs::file_size(fs::path(i_src_path));

   fprintf(fw, "unsigned int bin_res_size = %d;\n", (uint32_t)size);
   fprintf(fw, "unsigned char bin_res[] =\n{\n");

   while ((!feof(f)))
   {
      uint8_t c;

      if (fread(&c, 1, 1, f) == 0)
      {
         break;
      }

      fprintf(fw, "%d,", (uint32_t)c);
      ++max_column_count;

      if (max_column_count % 50 == 0)
      {
         fprintf(fw, "\n");
      }
   }

   // go back one char
   fseek(fw, -1, SEEK_CUR);

   fprintf(fw, "\n};\n");
   fclose(f);
   fclose(fw);
}

void long_op_bin2c::run()
{
   if (fs::exists(src_path))
   {
      if (fs::is_regular_file(src_path))
      {
         utrx(untr("starting conversion of [{}]"), path2string(src_path));
         convert_bin_2_c_array(src_path.generic_string(), output_path.generic_string());
      }
      else
      {
         mws_throw mws_exception(trs("{} is not a file", src_path.string()));
      }
   }
   else
   {
      mws_throw mws_exception(trs("path {} does not exist", src_path.string()));
   }
}
