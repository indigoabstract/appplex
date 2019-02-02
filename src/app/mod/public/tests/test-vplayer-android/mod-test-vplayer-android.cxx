#include "stdafx.hxx"

#include "mod-test-vplayer-android.hxx"

#ifdef MOD_TEST_VPLAYER_ANDROID

using std::string;


mod_test_vplayer_android::mod_test_vplayer_android() : mws_mod(mws_stringify(MOD_TEST_VPLAYER_ANDROID)) {}

mws_sp<mod_test_vplayer_android> mod_test_vplayer_android::nwi()
{
	return mws_sp<mod_test_vplayer_android>(new mod_test_vplayer_android());
}

void mod_test_vplayer_android::init()
{
}

void mod_test_vplayer_android::load()
{
}

bool mod_test_vplayer_android::update()
{
	return true;
}

#endif
