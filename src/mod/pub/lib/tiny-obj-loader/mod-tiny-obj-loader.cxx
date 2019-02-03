#include "stdafx.hxx"

#include "mod-tiny-obj-loader.hxx"


mod_tiny_obj_loader::mod_tiny_obj_loader() : mws_mod(mws_stringify(MOD_TINY_OBJ_LOADER)) {}

mws_sp<mod_tiny_obj_loader> mod_tiny_obj_loader::nwi()
{
   return mws_sp<mod_tiny_obj_loader>(new mod_tiny_obj_loader());
}

void mod_tiny_obj_loader::init() {}
void mod_tiny_obj_loader::init_mws() {}
void mod_tiny_obj_loader::load() {}
