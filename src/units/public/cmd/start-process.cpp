#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "start-process.hpp"
#include "long-operation.hpp"
#include <boost/thread.hpp>
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


class long_op_print_process_output: public long_operation
{
public:
	long_op_print_process_output(bp::child& c);
	void run();

private:
	bp::child& chp;
};


long_op_print_process_output::long_op_print_process_output(bp::child& c) : chp(c){}

void long_op_print_process_output::run()
{
	std::string line;
	shared_ptr<bp::pistream> isStdout = chp.get_stdout();
	shared_ptr<bp::pistream> isStderr = chp.get_stderr();


	if(isStderr)
	{
		while (std::getline(*isStderr, line))
		{
			trx("%1%") % line;
		}
	}

	if(isStdout)
	{
		while (std::getline(*isStdout, line))
		{
			trx("%1%") % line;
		}
	}
}


void start_process::exe_args(unicodestring& exec, vector<unicodestring>& args, int ms_to_wait_for_process)
	// make sure you put process location path as first argument
{
	utrc(untr("exe-args [%1%")) % exec;

	for(int k = 1; k < args.size(); k++)
	{
		utrc(untr(" %1%")) % args[k];
	}

	utrx(untr("]"));

	unicode_context ctx; 
	ctx.stdout_behavior = bp::capture_stream();
	ctx.stderr_behavior = bp::capture_stream();

	bp::child c = launch_unicode_process(exec, args, ctx);
	bp::status s = c.wait(ms_to_wait_for_process);

	if(s.exited())
	{
		trx("exit-code [%1%]") % s.exit_status();
	}
	else
	{
		trx("exit-status-fail!");
	}

	print_process_output(c);

}

void start_process::exe_shell(unicodestring& exec, int ms_to_wait_for_process)
{
	utrx(untr("exe-shell [%1%]")) % exec;

	unicode_context ctx;
	ctx.stdout_behavior = bp::capture_stream();
	ctx.stderr_behavior = bp::capture_stream();

	bp::child c = launch_unicode_shell(exec, ctx);
	bp::status s = c.wait(ms_to_wait_for_process);

	if(s.exited())
	{
		trx("exit-code [%1%]") % s.exit_status();
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
	boost::this_thread::sleep(boost::posix_time::seconds(1));

	if(lop->is_active())
		// something went wrong. the process output thread hasn't finished yet. give it more time
	{
		boost::this_thread::sleep(boost::posix_time::seconds(15));

		if(!lop->is_active())
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
