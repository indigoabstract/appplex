#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_OPENVG

#include "mws-mod.hxx"


class mod_test_openvg : public mws_mod
{
public:
   static mws_sp<mod_test_openvg> nwi();

   virtual void init();
   virtual void on_destroy();
   virtual void init_mws();
   virtual void load();

private:
   mod_test_openvg();

   void init_shivavg();
   void destroy_shivavg();
   void init_monkvg();
   void destroy_monkvg();
};

#endif
