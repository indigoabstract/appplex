#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_GEN_TEXTURE

#include "mod.hxx"


class mod_gen_texture : public mws_mod
{
public:
   static mws_sp<mod_gen_texture> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_gen_texture();
};

#endif
