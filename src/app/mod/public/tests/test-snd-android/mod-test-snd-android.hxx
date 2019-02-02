#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_SND_ANDROID

#include "mod.hxx"


class mod_test_snd_android : public mws_mod
{
public:
   static mws_sp<mod_test_snd_android> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_test_snd_android();
};

#endif
