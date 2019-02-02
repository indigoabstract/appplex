#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_RES_LD

#include "mod-res-ld.hxx"


mod_res_ld::mod_res_ld() : mws_mod(mws_stringify(MOD_RES_LD)) {}

mws_sp<mod_res_ld> mod_res_ld::nwi()
{
   return mws_sp<mod_res_ld>(new mod_res_ld());
}

void mod_res_ld::init() {}
void mod_res_ld::init_mws() {}
void mod_res_ld::load() {}

#endif
