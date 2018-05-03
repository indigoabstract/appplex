#include "stdafx.h"

#include "unit-test-openvg.hpp"

#ifdef UNIT_TEST_OPENVG


unit_test_openvg::unit_test_openvg()
{
	set_name("test-openvg");
}

shared_ptr<unit_test_openvg> unit_test_openvg::nwi()
{
	return shared_ptr<unit_test_openvg>(new unit_test_openvg());
}

void unit_test_openvg::init()
{
}

void unit_test_openvg::on_destroy()
{
	unit::on_destroy();

	destroy_shivavg();
	destroy_monkvg();
}

void unit_test_openvg::init_mws()
{
	init_shivavg();
	init_monkvg();
}

void unit_test_openvg::load()
{
}

#endif
