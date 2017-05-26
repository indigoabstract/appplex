#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "pfm.hpp"


namespace std
{
	class thread;
}


class long_operation
{
public:
	long_operation();
	static void run_on_separate_thread(shared_ptr<long_operation> lop);
    virtual void run();
	bool is_active();
	// call join() after the thread finishes to reclaim memory, or else a memory leak will result
	void join();

private:
	void run_impl();

	shared_ptr<std::thread> threadp;
	volatile bool active;
};

#endif
