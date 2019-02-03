#pragma once

#include "mws-mod.hxx"


class mod_mws_vkb : public mws_mod
{
public:
   static mws_sp<mod_mws_vkb> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_mws_vkb();
};
