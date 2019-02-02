#include "stdafx.hxx"

#include "unit-test-openvg.hxx"

#ifdef UNIT_TEST_OPENVG


unit_test_openvg::unit_test_openvg() : unit(mws_stringify(UNIT_TEST_OPENVG)) {}

mws_sp<unit_test_openvg> unit_test_openvg::nwi()
{
	return mws_sp<unit_test_openvg>(new unit_test_openvg());
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
