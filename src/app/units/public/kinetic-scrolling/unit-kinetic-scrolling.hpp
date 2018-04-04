#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_KINETIC_SCROLLING

#include "unit.hpp"

class unit_kinetic_scrolling : public unit
{
public:
	static shared_ptr<unit_kinetic_scrolling> new_instance();

    virtual void init();
	virtual void init_mws();

private:
	unit_kinetic_scrolling();
};

#endif
