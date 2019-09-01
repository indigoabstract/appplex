#pragma once

#include "mws-mod.hxx"


class mod_multi_touch_input : public mws_mod
{
public:
   static mws_sp<mod_multi_touch_input> nwi();

   virtual void init_mws() override;

private:
   mod_multi_touch_input();
};
