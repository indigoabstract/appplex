#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_VPLAYER_ANDROID

#include "unit.h"


class unit_test_vplayer_android : public unit
{
public:
	static shared_ptr<unit_test_vplayer_android> nwi(){ return shared_ptr<unit_test_vplayer_android>(); }

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_vplayer_android();
};

#endif
