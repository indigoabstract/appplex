#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_STK

#include "unit.hpp"

class unit_test_stk : public unit
{
public:
	static shared_ptr<unit_test_stk> new_instance();

    virtual void init();
	virtual void init_ux();

private:
	unit_test_stk();
};

#endif
