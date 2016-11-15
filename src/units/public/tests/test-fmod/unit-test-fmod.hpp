#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FMOD

#include "unit.hpp"

class unit_test_fmod : public unit
{
public:
	static shared_ptr<unit_test_fmod> new_instance();

    virtual void init();
	virtual void init_ux();

private:
	unit_test_fmod();
};

#endif
