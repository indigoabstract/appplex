#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_SHADOW_MAP

#include "mws-mod.hxx"


class mod_test_shadow_map_impl;

class mod_test_shadow_map : public mws_mod
{
public:
   static mws_sp<mod_test_shadow_map> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_shadow_map();

   mws_sp<mod_test_shadow_map_impl> p;
};

#endif
