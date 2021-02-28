#pragma once

#include "mws-mod.hxx"


class mod_z85 : public mws_mod
{
public:
   static mws_sp<mod_z85> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_z85();
};
