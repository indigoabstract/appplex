#pragma once

#include "mws-mod.hxx"


class mod_tiny_obj_loader : public mws_mod
{
public:
   static mws_sp<mod_tiny_obj_loader> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   mod_tiny_obj_loader();
};
