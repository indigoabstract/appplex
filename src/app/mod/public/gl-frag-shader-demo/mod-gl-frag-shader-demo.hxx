#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_GL_FRAG_SHADER_DEMO

#include "mod.hxx"

class mod_gl_frag_shader_demo_impl;
class mod_gl_frag_shader_demo_page;


class mod_gl_frag_shader_demo : public mws_mod
{
public:
	static mws_sp<mod_gl_frag_shader_demo> nwi();

    virtual void init();
	virtual void init_mws();
    virtual void load();

private:
	mod_gl_frag_shader_demo();

	mws_sp<mod_gl_frag_shader_demo_impl> p;
	friend class mod_gl_frag_shader_demo_page;
};

#endif
