#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_SND_ANDROID

#include "unit.hpp"


class unit_test_snd_android : public unit
{
public:
	static shared_ptr<unit_test_snd_android> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_test_snd_android();
};

#endif
