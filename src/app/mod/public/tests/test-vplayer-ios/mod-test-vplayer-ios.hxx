#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_VPLAYER_IOS

#include "mod.hxx"


class mod_test_vplayer_ios : public mws_mod
{
public:
   static mws_sp<mod_test_vplayer_ios> nwi() { return mws_sp<mod_test_vplayer_ios>(); }

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   mod_test_vplayer_ios();
};

#endif
