#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_ABSTRACT_RACING

#include "mws-mod.hxx"

class mod_abstract_racing : public mws_mod
{
public:
   static mws_sp<mod_abstract_racing> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   mod_abstract_racing();
};

#endif
