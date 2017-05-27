#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"
#include <string> 
#include <vector> 

namespace boost
{
   namespace process
   {
      class child;
   }
}


class start_process
{
public:
   static const int MS_TO_WAIT_FOR_PROCESS = 60 * 1000;

   // make sure you put process location path as first argument
   static void exe_args(unicodestring& exec, std::vector<unicodestring>& args, int ms_to_wait_for_process = MS_TO_WAIT_FOR_PROCESS);
   static void exe_shell(unicodestring& exec, int ms_to_wait_for_process = MS_TO_WAIT_FOR_PROCESS);

private:
   static void print_process_output(boost::process::child& c);
};

#endif
