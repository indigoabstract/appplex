#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_UTIL

#include "mod-util.hxx"


mod_util::mod_util() : mws_mod(mws_stringify(MOD_UTIL)) {}

mws_sp<mod_util> mod_util::nwi()
{
   return mws_sp<mod_util>(new mod_util());
}

void mod_util::init() {}
void mod_util::init_mws() {}
void mod_util::load() {}

#endif
