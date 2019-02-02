#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_NOTIFICATIONS

#include "mod-notifications.hxx"


mod_notifications::mod_notifications() : mws_mod(mws_stringify(MOD_NOTIFICATIONS)) {}

mws_sp<mod_notifications> mod_notifications::nwi()
{
   return mws_sp<mod_notifications>(new mod_notifications());
}

void mod_notifications::init() {}
void mod_notifications::init_mws() {}
void mod_notifications::load() {}

#endif
