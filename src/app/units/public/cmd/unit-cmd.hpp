#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_CMD

#include "unit.hpp"

class long_operation;


class unit_cmd : public unit
{
public:
	static mws_sp<unit_cmd> nwi();

    virtual bool update();
    virtual void load();

private:
	unit_cmd();

	mws_sp<long_operation> lop;
};

#endif
