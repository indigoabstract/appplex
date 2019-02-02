#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_SND

#include "mws-mod.hxx"


class mod_snd : public mws_mod
{
public:
   static mws_sp<mod_snd> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_snd();
};

#endif
