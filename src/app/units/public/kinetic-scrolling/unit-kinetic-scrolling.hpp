#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_KINETIC_SCROLLING

#include "unit.hpp"

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
