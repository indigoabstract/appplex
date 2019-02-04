#pragma once

#include "pfm.hxx"


namespace std
{
	class thread;
}


class long_operation
{
public:
	long_operation();
	static void run_on_separate_thread(mws_sp<long_operation> lop);
    virtual void run();
	bool is_active();
	// call join() after the thread finishes to reclaim memory, or else a memory leak will result
	void join();

private:
	void run_impl();

	mws_sp<std::thread> threadp;
	volatile bool active;
};
