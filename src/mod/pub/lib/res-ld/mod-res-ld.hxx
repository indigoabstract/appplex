#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_RES_LD

#include "mws-mod.hxx"


class mod_res_ld : public mws_mod
{
public:
   static mws_sp<mod_res_ld> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_res_ld();
};

#endif
