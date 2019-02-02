#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_TRAIL

#include "mod.hxx"


class mod_test_trail_impl;

class mod_test_trail : public mws_mod
{
public:
   static mws_sp<mod_test_trail> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_trail();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<mod_test_trail_impl> p;
};

#endif
