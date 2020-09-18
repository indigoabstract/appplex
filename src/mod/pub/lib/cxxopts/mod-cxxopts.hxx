#pragma once

#include "mws-mod.hxx"


class mod_cxxopts : public mws_mod
{
public:
   static mws_sp<mod_cxxopts> nwi();

   virtual bool update() override;
   virtual void load() override;

private:
   mod_cxxopts();
};
