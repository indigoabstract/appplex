#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_MWS

#include "unit.hpp"


class unit_test_mws : public unit
{
public:
	static mws_sp<unit_test_mws> nwi();

	virtual void init_mws();

private:
	unit_test_mws();
};

#endif
