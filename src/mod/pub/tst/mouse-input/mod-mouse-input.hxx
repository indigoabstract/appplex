#pragma once

#include "mws-mod.hxx"


class mod_mouse_input : public mws_mod
{
public:
   static mws_sp<mod_mouse_input> nwi();

   virtual void init_mws() override;

private:
   mod_mouse_input();
};
