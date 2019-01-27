#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEXXED

#include "unit.hpp"


class unit_texxed : public unit
{
public:
   static mws_sp<unit_texxed> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   unit_texxed();
};

#endif
