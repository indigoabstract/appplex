#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_DYN_GEOMETRY

#include "mws-mod.hxx"


class mod_dyn_geometry_impl;

class mod_dyn_geometry : public mws_mod
{
public:
   static mws_sp<mod_dyn_geometry> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_dyn_geometry();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<mod_dyn_geometry_impl> p;
};

#endif
