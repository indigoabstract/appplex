#pragma once

#include "mws-mod.hxx"


class mod_screen_metrix : public mws_mod
{
public:
   static mws_sp<mod_screen_metrix> nwi();

   virtual void init_mws() override;

private:
   mod_screen_metrix();
};
