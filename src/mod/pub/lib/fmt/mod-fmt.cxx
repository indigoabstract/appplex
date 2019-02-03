#include "stdafx.hxx"

#include "mod-fmt.hxx"


mod_fmt::mod_fmt() : mws_mod(mws_stringify(MOD_FMT)) {}

mws_sp<mod_fmt> mod_fmt::nwi()
{
   return mws_sp<mod_fmt>(new mod_fmt());
}

void mod_fmt::init() {}
void mod_fmt::init_mws() {}
void mod_fmt::load() {}
