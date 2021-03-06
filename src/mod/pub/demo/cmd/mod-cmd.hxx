#pragma once

#include "mws-mod.hxx"

class long_operation;


class mod_cmd : public mws_mod
{
public:
	static mws_sp<mod_cmd> nwi();

    virtual bool update();
    virtual void load();

private:
	mod_cmd();

	mws_sp<long_operation> lop;
};
