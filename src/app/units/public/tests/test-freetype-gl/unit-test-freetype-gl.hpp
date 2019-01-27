#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FREETYPE_GL

#include "unit.hpp"


class unit_test_freetype_gl_impl;

class unit_test_freetype_gl : public unit
{
public:
	static mws_sp<unit_test_freetype_gl> nwi();

	void load() override;
	bool update() override;

private:
	unit_test_freetype_gl();
	void receive(mws_sp<mws_dp> idp) override;

	mws_sp<unit_test_freetype_gl_impl> p;
};

#endif
