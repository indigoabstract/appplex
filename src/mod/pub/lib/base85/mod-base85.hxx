#pragma once

#include "mws-mod.hxx"


class mod_base85 : public mws_mod
{
public:
   static mws_sp<mod_base85> nwi();

   virtual void init_mws() override;

private:
   mod_base85();
};
