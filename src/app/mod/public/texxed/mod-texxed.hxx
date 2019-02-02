#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEXXED

#include "mws-mod.hxx"


class mod_texxed : public mws_mod
{
public:
   static mws_sp<mod_texxed> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_texxed();
};

#endif
