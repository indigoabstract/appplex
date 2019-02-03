#pragma once

#include "mws-mod.hxx"


class mod_gfx_ext : public mws_mod
{
public:
   static mws_sp<mod_gfx_ext> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_gfx_ext();
};
