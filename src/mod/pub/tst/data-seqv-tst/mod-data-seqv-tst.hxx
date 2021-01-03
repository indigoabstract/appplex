#pragma once

#include "mws-mod.hxx"


class mod_data_seqv_tst : public mws_mod
{
public:
   static mws_sp<mod_data_seqv_tst> nwi();

   virtual void init() override;

private:
   mod_data_seqv_tst();
};
