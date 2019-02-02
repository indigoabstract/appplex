#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_GEN_TEXTURE

#include "unit.hxx"


class unit_gen_texture : public unit
{
public:
   static mws_sp<unit_gen_texture> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   unit_gen_texture();
};

#endif
