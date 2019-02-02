#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_INPUT

#include "mod-input.hxx"


mod_input::mod_input() : mws_mod(mws_stringify(MOD_INPUT)) {}

mws_sp<mod_input> mod_input::nwi()
{
   return mws_sp<mod_input>(new mod_input());
}

void mod_input::init() {}
void mod_input::init_mws() {}
void mod_input::load() {}

#endif
