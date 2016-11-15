#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_OPENVG

#include "unit.hpp"


class unit_test_openvg : public unit
{
public:
	static shared_ptr<unit_test_openvg> new_instance();

    virtual void init();
	virtual void on_destroy();
	virtual void init_ux();
    virtual void load();

private:
	unit_test_openvg();

	void init_shivavg();
	void destroy_shivavg();
	void init_monkvg();
	void destroy_monkvg();
};

#endif
