#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_INPUT

#include "mws-mod.hxx"


class mod_input : public mws_mod
{
public:
   static mws_sp<mod_input> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_input();
};

#endif
