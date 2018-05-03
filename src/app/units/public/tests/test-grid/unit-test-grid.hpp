#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GRID

#include "unit.hpp"


class unit_test_grid_impl;

class unit_test_grid : public unit
{
public:
	static shared_ptr<unit_test_grid> nwi();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_grid();
	virtual void receive(shared_ptr<iadp> idp);

	shared_ptr<unit_test_grid_impl> p;
};

#endif
