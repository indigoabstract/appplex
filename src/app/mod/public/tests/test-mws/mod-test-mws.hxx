#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_MWS

#include "mod.hxx"


class mod_test_mws : public mws_mod
{
public:
	static mws_sp<mod_test_mws> nwi();

	virtual void init_mws();

private:
	mod_test_mws();
};

#endif
