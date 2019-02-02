#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_MWS

#include "mod-mws.hxx"


mod_mws::mod_mws() : mws_mod(mws_stringify(MOD_MWS)) {}

mws_sp<mod_mws> mod_mws::nwi()
{
   return mws_sp<mod_mws>(new mod_mws());
}

void mod_mws::init() {}
void mod_mws::init_mws() {}
void mod_mws::load() {}

#endif
