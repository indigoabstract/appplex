#include "stdafx.hxx"

#include "mod-test-vplayer-ios.hxx"

#ifdef MOD_TEST_VPLAYER_IOS

using std::string;


mod_test_vplayer_ios::mod_test_vplayer_ios() : mws_mod(mws_stringify(MOD_TEST_VPLAYER_IOS)) {}

mws_sp<mod_test_vplayer_ios> mod_test_vplayer_ios::nwi()
{
	return mws_sp<mod_test_vplayer_ios>(new mod_test_vplayer_ios());
}

void mod_test_vplayer_ios::init()
{
}

void mod_test_vplayer_ios::load()
{
}

bool mod_test_vplayer_ios::update()
{
	return true;
}

#endif
