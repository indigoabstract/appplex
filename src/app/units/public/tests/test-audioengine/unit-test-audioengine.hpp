#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_AUDIOENGINE

#include "unit.hpp"


class unit_test_audioengine : public unit
{
public:
	static shared_ptr<unit_test_audioengine> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_test_audioengine();
};

#endif
