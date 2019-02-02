#include "stdafx.hxx"

#include "unit-test-vplayer-android.hxx"

#ifdef UNIT_TEST_VPLAYER_ANDROID

using std::string;


unit_test_vplayer_android::unit_test_vplayer_android() : unit(mws_stringify(UNIT_TEST_VPLAYER_ANDROID)) {}

mws_sp<unit_test_vplayer_android> unit_test_vplayer_android::nwi()
{
	return mws_sp<unit_test_vplayer_android>(new unit_test_vplayer_android());
}

void unit_test_vplayer_android::init()
{
}

void unit_test_vplayer_android::load()
{
}

bool unit_test_vplayer_android::update()
{
	return true;
}

#endif
