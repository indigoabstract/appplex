#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_RNG

#include "mod-rng.hxx"


mod_rng::mod_rng() : mws_mod(mws_stringify(MOD_RNG)) {}

mws_sp<mod_rng> mod_rng::nwi()
{
   return mws_sp<mod_rng>(new mod_rng());
}

void mod_rng::init() {}
void mod_rng::init_mws() {}
void mod_rng::load() {}

#endif
