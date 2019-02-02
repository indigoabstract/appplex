#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_FREETYPE_GL

#include "mod.hxx"


class mod_test_freetype_gl_impl;

class mod_test_freetype_gl : public mws_mod
{
public:
	static mws_sp<mod_test_freetype_gl> nwi();

	void load() override;
	bool update() override;

private:
	mod_test_freetype_gl();
	void receive(mws_sp<mws_dp> idp) override;

	mws_sp<mod_test_freetype_gl_impl> p;
};

#endif
