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
	static shared_ptr<unit_test_normal_mapping> nwi();

	virtual void init();
	virtual void load();
	virtual bool update();

private:
	unit_test_normal_mapping();
	virtual void receive(shared_ptr<mws_dp> idp);

	shared_ptr<ns_unit_test_normal_mapping::impl> p;
};

#endif
