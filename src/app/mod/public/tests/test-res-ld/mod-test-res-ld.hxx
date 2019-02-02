#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_RES_LD

#include "mod.hxx"


class mod_test_res_ld : public mws_mod
{
public:
   static mws_sp<mod_test_res_ld> nwi();

   virtual void init() override;
   virtual void load() override;
   virtual void update_view(int update_count) override;

private:
   mod_test_res_ld();
};

#endif
