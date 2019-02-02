#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_UTIL

#include "mws-mod.hxx"


class mod_util : public mws_mod
{
public:
   static mws_sp<mod_util> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_util();
};

#endif
