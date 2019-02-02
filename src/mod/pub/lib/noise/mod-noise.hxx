#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_NOISE

#include "mws-mod.hxx"


class mod_noise : public mws_mod
{
public:
   static mws_sp<mod_noise> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_noise();
};

#endif
