#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEMPLATE

#include "mod.hxx"


class mod_template : public mws_mod
{
public:
   static mws_sp<mod_template> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_template();
};

#endif
