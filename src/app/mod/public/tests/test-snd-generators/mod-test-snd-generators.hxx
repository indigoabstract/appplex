#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_SND_GENERATORS

#include "mws-mod.hxx"


class mod_test_snd_generators : public mws_mod
{
public:
   static mws_sp<mod_test_snd_generators> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_test_snd_generators();
};

#endif // MOD_TEST_SND_GENERATORS
