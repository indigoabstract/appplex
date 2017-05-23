#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "long-operation.hpp"
#include "min.hpp"
#include <thread>
#include <exception>


long_operation::long_operation()
{
	active = false;
}

void long_operation::run_on_separate_thread(shared_ptr<long_operation> lop)
{
	if(lop->is_active())
	{
        throw ia_exception("thread is still active");
	}

	lop->active = true;
	lop->threadp = shared_ptr<std::thread>(new std::thread(&long_operation::run_impl, lop.get()));
}

void long_operation::run()
{
}

void long_operation::join()
{
	if(threadp)
	{
		threadp->join();
		threadp.reset();
	}

	active = false;
}

void long_operation::run_impl()
{
	try
	{
		run();
		active = false;
	}
	catch(ia_exception& e)
	{
		active = false;
		trx("op error. [{}]", e.what());
	}
    catch(std::exception& e)
	{
		active = false;
		trx("thread op finished with error");
	}
	catch(...)
	{
		active = false;
		trx("exception of unknown type. raising error and terminating app");
		throw;
	}
}

bool long_operation::is_active()
{
	return active;
}

#endif
