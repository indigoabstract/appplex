#include "stdafx.h"

#include "unit-test-vplayer-ios.hpp"

#ifdef UNIT_TEST_VPLAYER_IOS

using std::string;


unit_test_vplayer_ios::unit_test_vplayer_ios()
{
	set_name("test-vplayer-ios");
}

shared_ptr<unit_test_vplayer_ios> unit_test_vplayer_ios::new_instance()
{
	return shared_ptr<unit_test_vplayer_ios>(new unit_test_vplayer_ios());
}

void unit_test_vplayer_ios::init()
{
}

void unit_test_vplayer_ios::load()
{
}

bool unit_test_vplayer_ios::update()
{
	return true;
}

#endif
