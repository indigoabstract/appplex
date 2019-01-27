#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_NORMAL_MAPPING

#include "unit.hpp"


namespace ns_unit_test_normal_mapping
{
	class impl;
}


class unit_test_normal_mapping : public unit
{
public:
	static mws_sp<unit_test_normal_mapping> nwi();

	virtual void init();
	virtual void load();
	virtual bool update();

private:
	unit_test_normal_mapping();
	virtual void receive(mws_sp<mws_dp> idp);

	mws_sp<ns_unit_test_normal_mapping::impl> p;
};

#endif
