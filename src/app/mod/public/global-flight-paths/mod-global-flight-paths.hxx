#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_GLOBAL_FLIGHT_PATHS

#include "mod.hxx"


class mod_global_flight_paths : public mws_mod
{
public:
   static mws_sp<mod_global_flight_paths> nwi();

   virtual void init_mws() override;

private:
   mod_global_flight_paths();
};

#endif
