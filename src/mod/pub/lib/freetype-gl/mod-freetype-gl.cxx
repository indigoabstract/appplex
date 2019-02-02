#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_FREETYPE_GL

#include "mod-freetype-gl.hxx"


mod_freetype_gl::mod_freetype_gl() : mws_mod(mws_stringify(MOD_FREETYPE_GL)) {}

mws_sp<mod_freetype_gl> mod_freetype_gl::nwi()
{
   return mws_sp<mod_freetype_gl>(new mod_freetype_gl());
}

void mod_freetype_gl::init() {}
void mod_freetype_gl::init_mws() {}
void mod_freetype_gl::load() {}

#endif
