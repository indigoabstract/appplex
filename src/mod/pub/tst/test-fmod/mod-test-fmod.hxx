#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_FMOD

#include "mws-mod.hxx"

class mod_test_fmod : public mws_mod
{
public:
   static mws_sp<mod_test_fmod> nwi();

   virtual void init();
   virtual void init_mws();

private:
   mod_test_fmod();
};

#endif
