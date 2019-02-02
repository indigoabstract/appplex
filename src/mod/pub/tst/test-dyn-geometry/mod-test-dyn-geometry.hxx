#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_DYN_GEOMETRY

#include "mws-mod.hxx"


class mod_test_dyn_geometry_impl;

class mod_test_dyn_geometry : public mws_mod
{
public:
   static mws_sp<mod_test_dyn_geometry> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_dyn_geometry();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<mod_test_dyn_geometry_impl> p;
};

#endif
