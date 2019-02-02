#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_KINETIC_SCROLLING

#include "unit.hxx"

class unit_kinetic_scrolling : public unit
{
public:
   static mws_sp<unit_kinetic_scrolling> nwi();

   virtual void init();
   virtual void init_mws();

private:
   unit_kinetic_scrolling();
};

#endif
