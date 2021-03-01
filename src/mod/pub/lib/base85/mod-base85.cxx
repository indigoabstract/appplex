#include "stdafx.hxx"

#include "mod-base85.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_base85::mod_base85() : mws_mod(mws_stringify(MOD_BASE85)) {}

mws_sp<mod_base85> mod_base85::nwi()
{
   return mws_sp<mod_base85>(new mod_base85());
}

void mod_base85::init_mws()
{
   mws_cam->clear_color = true;
}
