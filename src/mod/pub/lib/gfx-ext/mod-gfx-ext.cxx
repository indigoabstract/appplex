#include "stdafx.hxx"

#include "mod-gfx-ext.hxx"


mod_gfx_ext::mod_gfx_ext() : mws_mod(mws_stringify(MOD_GFX_EXT)) {}

mws_sp<mod_gfx_ext> mod_gfx_ext::nwi()
{
   return mws_sp<mod_gfx_ext>(new mod_gfx_ext());
}

void mod_gfx_ext::init() {}
void mod_gfx_ext::init_mws() {}
void mod_gfx_ext::load() {}
