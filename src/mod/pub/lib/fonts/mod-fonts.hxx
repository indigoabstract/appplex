#pragma once

#include "mws-mod.hxx"


class mod_fonts : public mws_mod
{
public:
   static mws_sp<mod_fonts> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_fonts();
};
