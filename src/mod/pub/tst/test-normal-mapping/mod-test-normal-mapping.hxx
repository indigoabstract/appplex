#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_NORMAL_MAPPING

#include "mws-mod.hxx"


namespace ns_mod_test_normal_mapping
{
	class impl;
}


class mod_test_normal_mapping : public mws_mod
{
public:
	static mws_sp<mod_test_normal_mapping> nwi();

	virtual void init();
	virtual void load();
	virtual bool update();

private:
	mod_test_normal_mapping();
	virtual void receive(mws_sp<mws_dp> idp);

	mws_sp<ns_mod_test_normal_mapping::impl> p;
};

#endif
