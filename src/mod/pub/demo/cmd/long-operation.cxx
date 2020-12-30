#include "stdafx.hxx"

#include "long-operation.hxx"
#include "krn.hxx"
#include <thread>
#include <exception>


long_operation::long_operation()
{
   active = false;
}

void long_operation::run_on_separate_thread(mws_sp<long_operation> lop)
{
   if (lop->is_active())
   {
      mws_throw mws_exception("thread is still active");
   }

   lop->active = true;
   lop->threadp = mws_sp<std::thread>(new std::thread(&long_operation::run_impl, lop.get()));
}

void long_operation::run()
{
}

void long_operation::join()
{
   if (threadp)
   {
      threadp->join();
      threadp.reset();
   }

   active = false;
}

void long_operation::run_impl()
{
   mws_try
   {
      run();
      active = false;
   }
   mws_catch(mws_exception& e)
   {
      active = false;
      trx("error[ {} ]", e.what());
   }
   mws_catch(std::exception& e)
   {
      active = false;
      trx("thread finished with error[ {} ]", e.what());
   }
   mws_catch(...)
   {
      active = false;
      trx("exception of unknown type. raising error and terminating app");
      mws_throw;
   }
}

bool long_operation::is_active()
{
   return active;
}
