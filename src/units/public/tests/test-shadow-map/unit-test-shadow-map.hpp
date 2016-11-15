#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_SHADOW_MAP

#include "unit.hpp"


class unit_test_shadow_map_impl;

class unit_test_shadow_map : public unit
{
public:
	static shared_ptr<unit_test_shadow_map> new_instance();

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_shadow_map();

	shared_ptr<unit_test_shadow_map_impl> p;
};

#endif
