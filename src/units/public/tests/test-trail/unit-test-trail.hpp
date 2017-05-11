#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_TRAIL

#include "unit.hpp"


class unit_test_trail_impl;

class unit_test_trail : public unit
{
public:
	static shared_ptr<unit_test_trail> new_instance();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_trail();
	virtual void receive(shared_ptr<iadp> idp);

	shared_ptr<unit_test_trail_impl> p;
};

#endif
