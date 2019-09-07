#pragma once

#include "mws-mod.hxx"


class mod_tex_atlas : public mws_mod
{
public:
   static mws_sp<mod_tex_atlas> nwi();

   virtual void init_mws() override;

private:
   mod_tex_atlas();
};
