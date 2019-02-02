#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_GFX

#include "mws-mod.hxx"


class mod_gfx : public mws_mod
{
public:
   static mws_sp<mod_gfx> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_gfx();
};

#endif
