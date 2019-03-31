#pragma once

#include "mws-mod.hxx"


class mod_vrn : public mws_mod
{
public:
   static mws_sp<mod_vrn> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_vrn();
};
