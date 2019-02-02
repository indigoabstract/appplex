#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_VPLAYER_ANDROID

#include "mws-mod.hxx"


class mod_test_vplayer_android : public mws_mod
{
public:
   static mws_sp<mod_test_vplayer_android> nwi() { return mws_sp<mod_test_vplayer_android>(); }

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_vplayer_android();
};

#endif
