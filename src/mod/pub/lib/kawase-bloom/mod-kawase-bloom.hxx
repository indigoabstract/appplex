#pragma once

#include "mws-mod.hxx"


class mod_kawase_bloom : public mws_mod
{
public:
   static mws_sp<mod_kawase_bloom> nwi();

   virtual void init_mws() override;

private:
   mod_kawase_bloom();
};
