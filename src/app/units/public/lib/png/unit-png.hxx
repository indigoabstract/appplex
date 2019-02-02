#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_PNG

#include "unit.hxx"


class unit_png : public unit
{
public:
   static mws_sp<unit_png> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   unit_png();
};

#endif
