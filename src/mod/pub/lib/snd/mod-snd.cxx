#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_SND

#include "mod-snd.hxx"


mod_snd::mod_snd() : mws_mod(mws_stringify(MOD_SND)) {}

mws_sp<mod_snd> mod_snd::nwi()
{
   return mws_sp<mod_snd>(new mod_snd());
}

void mod_snd::init() {}
void mod_snd::init_mws() {}
void mod_snd::load() {}

#endif
