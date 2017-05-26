#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_GL_FRAG_SHADER_DEMO

#include "unit.hpp"

class unit_gl_frag_shader_demo_impl;
class unit_gl_frag_shader_demo_page;


class unit_gl_frag_shader_demo : public unit
{
public:
	static shared_ptr<unit_gl_frag_shader_demo> new_instance();

    virtual void init();
	virtual void init_ux();
    virtual void load();

private:
	unit_gl_frag_shader_demo();

	shared_ptr<unit_gl_frag_shader_demo_impl> p;
	friend class unit_gl_frag_shader_demo_page;
};

#endif
