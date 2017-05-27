#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include <boost/process.hpp>
#include "start-process.hpp"
#include "long-operation.hpp"
#include <chrono>
#include <thread>
#include <exception> 

using std::vector;
namespace bp = ::boost::process;


#if defined UNICODE_USING_STD_STRING

#define unicode_context				bp::context
#define launch_unicode_process		bp::launch
#define launch_unicode_shell		bp::launch_shell

#elif defined UNICODE_USING_STD_WSTRING

#define unicode_context				bp::contextw
#define launch_unicode_process		bp::launchw
#define launch_unicode_shell		bp::launch_shellw

#endif


class long_op_print_process_output : public long_operation
{
public:
   long_op_print_process_output(bp::child& c);
   void run();

private:
   bp::child& chp;
};


long_op_print_process_output::long_op_print_process_output(bp::child& c) : chp(c) {}

void long_op_print_process_output::run()
{
   std::string line;
   shared_ptr<bp::pistream> isStdout = chp.get_stdout();
   shared_ptr<bp::pistream> isStderr = chp.get_stderr();


   if (isStderr)
   {
      while (std::getline(*isStderr, line))
      {
         trx("{}", line);
      }
   }

   if (isStdout)
   {
      while (std::getline(*isStdout, line))
      {
         trx("{}", line);
      }
   }
}


void start_process::exe_args(unicodestring& exec, vector<unicodestring>& args, int ms_to_wait_for_process)
// make sure you put process location path as first argument
{
   utrc(untr("exe-args [{}"), exec);

   for (int k = 1; k < args.size(); k++)
   {
      utrc(untr(" {}"), args[k]);
   }

   utrx(untr("]"));

   unicode_context ctx;
   ctx.stdout_behavior = bp::capture_stream();
   ctx.stderr_behavior = bp::capture_stream();

   bp::child c = launch_unicode_process(exec, args, ctx);
   bp::status s = c.wait(ms_to_wait_for_process);

   if (s.exited())
   {
      trx("exit-code [{}]", s.exit_status());
   }
   else
   {
      trx("exit-status-fail!");
   }

   print_process_output(c);
}

void start_process::exe_shell(unicodestring& exec, int ms_to_wait_for_process)
{
   utrx(untr("exe-shell [{}]"), exec);

   unicode_context ctx;
   ctx.stdout_behavior = bp::capture_stream();
   ctx.stderr_behavior = bp::capture_stream();

   bp::child c = launch_unicode_shell(exec, ctx);
   bp::status s = c.wait(ms_to_wait_for_process);

   if (s.exited())
   {
      trx("exit-code [{}]", s.exit_status());
   }
   else
   {
      trx("exit-status-fail!");
   }

   print_process_output(c);
}

void start_process::print_process_output(bp::child& c)
{
   shared_ptr<long_operation> lop = shared_ptr<long_operation>(new long_op_print_process_output(c));

   long_operation::run_on_separate_thread(lop);
   // sleep a bit to let the process output thread finish
   std::this_thread::sleep_for(std::chrono::seconds(1));

   if (lop->is_active())
      // something went wrong. the process output thread hasn't finished yet. give it more time
   {
      std::this_thread::sleep_for(std::chrono::seconds(15));

      if (!lop->is_active())
         // the process output thread is done and we can join it
      {
         lop->join();
      }
      else
         // the process output thread has hanged, causing a memory leak
      {
         trx("the process output thread has hanged!");
      }
   }
}

#endif
