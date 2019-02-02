#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_ABSTRACT_RACING

#include "unit.hxx"

class unit_abstract_racing : public unit
{
public:
   static mws_sp<unit_abstract_racing> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   unit_abstract_racing();
};

#endif
