#include "stdafx.h"

#include "unit-test-vplayer-android.hpp"

#ifdef UNIT_TEST_VPLAYER_ANDROID

using std::string;


unit_test_vplayer_android::unit_test_vplayer_android() : unit(mws_stringify(UNIT_TEST_VPLAYER_ANDROID)) {}

shared_ptr<unit_test_vplayer_android> unit_test_vplayer_android::nwi()
{
	return shared_ptr<unit_test_vplayer_android>(new unit_test_vplayer_android());
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
