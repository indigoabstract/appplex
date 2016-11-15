#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_REFLECTIONS

#include "unit.hpp"


class unit_test_reflections_impl;

class unit_test_reflections : public unit
{
public:
	static shared_ptr<unit_test_reflections> new_instance();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_reflections();
	virtual void receive(shared_ptr<iadp> idp);

	shared_ptr<unit_test_reflections_impl> p;
};

#endif
