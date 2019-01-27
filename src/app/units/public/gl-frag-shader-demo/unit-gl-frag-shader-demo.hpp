#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_GL_FRAG_SHADER_DEMO

#include "unit.hpp"

class unit_gl_frag_shader_demo_impl;
class unit_gl_frag_shader_demo_page;


class unit_gl_frag_shader_demo : public unit
{
public:
	static mws_sp<unit_gl_frag_shader_demo> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	unit_gl_frag_shader_demo();

	mws_sp<unit_gl_frag_shader_demo_impl> p;
	friend class unit_gl_frag_shader_demo_page;
};

#endif
