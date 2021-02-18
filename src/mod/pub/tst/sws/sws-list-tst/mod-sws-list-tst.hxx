#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_SWS_LIST_TST

#include "mws-mod.hxx"


class mod_sws_list_tst : public mws_mod
{
public:
	static mws_sp<mod_sws_list_tst> nwi();

	virtual void init_mws();

private:
	mod_sws_list_tst();
};

#endif
