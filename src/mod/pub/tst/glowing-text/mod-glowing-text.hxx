#pragma once

#include "mws-mod.hxx"


class mod_glowing_text : public mws_mod
{
public:
   static mws_sp<mod_glowing_text> nwi();

   virtual void init_mws() override;

private:
   mod_glowing_text();
};
