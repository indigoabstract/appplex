#pragma once

#include "mws-mod.hxx"


class mod_storage_dir_tst : public mws_mod
{
public:
   static mws_sp<mod_storage_dir_tst> nwi();

   virtual void init() override;

private:
   mod_storage_dir_tst();
};
