#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEXXED

#include "unit.hpp"


class unit_texxed : public unit
{
public:
	static shared_ptr<unit_texxed> new_instance();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_texxed();
};

#endif
