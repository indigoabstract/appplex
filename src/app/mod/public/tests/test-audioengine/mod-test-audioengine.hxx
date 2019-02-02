#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_AUDIOENGINE

#include "mod.hxx"


class mod_test_audioengine : public mws_mod
{
public:
   static mws_sp<mod_test_audioengine> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_test_audioengine();
};

#endif
