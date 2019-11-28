#pragma once

#include "mws-mod.hxx"


class mod_font_metrix : public mws_mod
{
public:
   static mws_sp<mod_font_metrix> nwi();

   virtual void init_mws() override;

private:
   mod_font_metrix();
};
