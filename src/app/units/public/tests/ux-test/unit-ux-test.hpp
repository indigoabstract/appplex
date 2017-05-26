#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_UX_TEST

#include "unit.hpp"


class unit_ux_test : public unit
{
public:
	static shared_ptr<unit_ux_test> new_instance();

	virtual void init_ux();

private:
	unit_ux_test();
};

#endif
