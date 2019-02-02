#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_FMT

#include "mws-mod.hxx"


class mod_fmt : public mws_mod
{
public:
   static mws_sp<mod_fmt> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_fmt();
};

#endif
