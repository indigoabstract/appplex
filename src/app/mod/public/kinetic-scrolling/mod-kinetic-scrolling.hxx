#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_KINETIC_SCROLLING

#include "mod.hxx"

class mod_kinetic_scrolling : public mws_mod
{
public:
   static mws_sp<mod_kinetic_scrolling> nwi();

   virtual void init();
   virtual void init_mws();

private:
   mod_kinetic_scrolling();
};

#endif
