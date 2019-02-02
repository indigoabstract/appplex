#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_KXMD

#include "mod-kxmd.hxx"


mod_kxmd::mod_kxmd() : mws_mod(mws_stringify(MOD_KXMD)) {}

mws_sp<mod_kxmd> mod_kxmd::nwi()
{
   return mws_sp<mod_kxmd>(new mod_kxmd());
}

void mod_kxmd::init() {}
void mod_kxmd::init_mws() {}
void mod_kxmd::load() {}

#endif
