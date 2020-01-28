#include "stdafx.hxx"

#include "mod-fonts.hxx"


mod_fonts::mod_fonts() : mws_mod(mws_stringify(MOD_FONTS)) {}

mws_sp<mod_fonts> mod_fonts::nwi()
{
   return mws_sp<mod_fonts>(new mod_fonts());
}

void mod_fonts::init() {}
void mod_fonts::init_mws() {}
void mod_fonts::load() {}
