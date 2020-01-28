#include "stdafx.hxx"

#include "mod-noise.hxx"


mod_noise::mod_noise() : mws_mod(mws_stringify(MOD_NOISE)) {}

mws_sp<mod_noise> mod_noise::nwi()
{
   return mws_sp<mod_noise>(new mod_noise());
}

void mod_noise::init() {}
void mod_noise::init_mws() {}
void mod_noise::load() {}
