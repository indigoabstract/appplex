#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "unit.hpp"


class unit_test_dyn_geometry_impl;

class unit_test_dyn_geometry : public unit
{
public:
	static shared_ptr<unit_test_dyn_geometry> nwi();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_dyn_geometry();
	virtual void receive(shared_ptr<iadp> idp);

	shared_ptr<unit_test_dyn_geometry_impl> p;
};

#endif
