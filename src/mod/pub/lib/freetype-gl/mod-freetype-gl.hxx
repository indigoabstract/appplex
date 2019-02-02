#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_FREETYPE_GL

#include "mws-mod.hxx"


class mod_freetype_gl : public mws_mod
{
public:
   static mws_sp<mod_freetype_gl> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_freetype_gl();
};

#endif
