#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_GFX

#include "mod-gfx.hxx"


mod_gfx::mod_gfx() : mws_mod(mws_stringify(MOD_GFX)) {}

mws_sp<mod_gfx> mod_gfx::nwi()
{
   return mws_sp<mod_gfx>(new mod_gfx());
}

void mod_gfx::init() {}
void mod_gfx::init_mws() {}
void mod_gfx::load() {}

#endif
