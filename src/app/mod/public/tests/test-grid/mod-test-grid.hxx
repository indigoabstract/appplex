#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_GRID

#include "mod.hxx"


class mod_test_grid_impl;

class mod_test_grid : public mws_mod
{
public:
   static mws_sp<mod_test_grid> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_grid();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<mod_test_grid_impl> p;
};

#endif
