#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEMPLATE

#include "unit.hpp"


class unit_template : public unit
{
public:
   static mws_sp<unit_template> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   unit_template();
};

#endif
