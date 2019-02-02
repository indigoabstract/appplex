#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_TOUCH_INPUT

#include "mod.hxx"
#include "pfm.hxx"


class mod_test_touch_input : public mws_mod
{
public:
   static mws_sp<mod_test_touch_input> nwi();

   virtual void init_mws() override;

private:
   mod_test_touch_input();
};

#endif
