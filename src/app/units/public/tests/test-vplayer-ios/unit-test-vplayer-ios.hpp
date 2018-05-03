#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_VPLAYER_IOS

#include "unit.h"


class unit_test_vplayer_ios : public unit
{
public:
	static shared_ptr<unit_test_vplayer_ios> nwi(){ return shared_ptr<unit_test_vplayer_ios>(); }

    virtual void init();
    virtual void load();
	virtual bool update();

private:
	unit_test_vplayer_ios();
};

#endif
