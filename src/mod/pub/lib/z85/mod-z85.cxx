#include "stdafx.hxx"

#include "mod-z85.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_z85::mod_z85() : mws_mod(mws_stringify(MOD_Z85)) {}

mws_sp<mod_z85> mod_z85::nwi()
{
   return mws_sp<mod_z85>(new mod_z85());
}

void mod_z85::init()
{
}

void mod_z85::init_mws()
{
   mws_cam->clear_color = true;
}

void mod_z85::load()
{
}
