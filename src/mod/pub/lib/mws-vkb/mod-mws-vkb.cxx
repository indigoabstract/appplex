#include "stdafx.hxx"

#include "mod-mws-vkb.hxx"


mod_mws_vkb::mod_mws_vkb() : mws_mod(mws_stringify(MOD_MWS_VKB)) {}

mws_sp<mod_mws_vkb> mod_mws_vkb::nwi()
{
   return mws_sp<mod_mws_vkb>(new mod_mws_vkb());
}

void mod_mws_vkb::init() {}
void mod_mws_vkb::init_mws() {}
void mod_mws_vkb::load() {}
