#pragma once

#include "mws-mod.hxx"


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
