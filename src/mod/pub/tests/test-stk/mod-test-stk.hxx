#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_STK

#include "mws-mod.hxx"

class mod_test_stk : public mws_mod
{
public:
   static mws_sp<mod_test_stk> nwi();

   virtual void init();
   virtual void init_mws();

private:
   mod_test_stk();
};

#endif
