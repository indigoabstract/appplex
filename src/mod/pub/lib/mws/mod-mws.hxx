#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_MWS

#include "mws-mod.hxx"


class mod_mws : public mws_mod
{
public:
   static mws_sp<mod_mws> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_mws();
};

#endif
