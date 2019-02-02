#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_KXMD

#include "mws-mod.hxx"


class mod_kxmd : public mws_mod
{
public:
   static mws_sp<mod_kxmd> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_kxmd();
};

#endif
