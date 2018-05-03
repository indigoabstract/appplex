#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FREETYPE_GL

#include "unit.hpp"


class unit_test_freetype_gl_impl;

class unit_test_freetype_gl : public unit
{
public:
	static shared_ptr<unit_test_freetype_gl> nwi();

	void init_mws() override;
	void load() override;
	bool update() override;

private:
	unit_test_freetype_gl();
	void receive(shared_ptr<iadp> idp) override;

	shared_ptr<unit_test_freetype_gl_impl> p;
};

#endif
