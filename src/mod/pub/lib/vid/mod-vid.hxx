#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_VID

#include "mws-mod.hxx"


class mod_vid : public mws_mod
{
public:
   static mws_sp<mod_vid> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_vid();
};

#endif
