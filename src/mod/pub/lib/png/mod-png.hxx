#pragma once

#include "mws-mod.hxx"


class mod_png : public mws_mod
{
public:
   static mws_sp<mod_png> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_png();
};
