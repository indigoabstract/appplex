#include "stdafx.hxx"

#include "mod-test-openvg.hxx"


mod_test_openvg::mod_test_openvg() : mws_mod(mws_stringify(MOD_TEST_OPENVG)) {}

mws_sp<mod_test_openvg> mod_test_openvg::nwi()
{
	return mws_sp<mod_test_openvg>(new mod_test_openvg());
}

void mod_test_openvg::init()
{
}

void mod_test_openvg::on_destroy()
{
	mws_mod::on_destroy();

	destroy_shivavg();
	destroy_monkvg();
}

void mod_test_openvg::init_mws()
{
	init_shivavg();
	init_monkvg();
}

void mod_test_openvg::load()
{
}
