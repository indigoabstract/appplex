#pragma once

#include "mws-mod.hxx"


class mod_who_am_i : public mws_mod
{
public:
   static mws_sp<mod_who_am_i> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_who_am_i();
};
