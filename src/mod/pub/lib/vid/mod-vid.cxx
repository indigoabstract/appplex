#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_VID

#include "mod-vid.hxx"


mod_vid::mod_vid() : mws_mod(mws_stringify(MOD_VID)) {}

mws_sp<mod_vid> mod_vid::nwi()
{
   return mws_sp<mod_vid>(new mod_vid());
}

void mod_vid::init() {}
void mod_vid::init_mws() {}
void mod_vid::load() {}

#endif
