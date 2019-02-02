#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_CMD

#include "unit.hxx"

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
