#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEMPLATE

#include "unit.hpp"


class unit_template : public unit
{
public:
   static shared_ptr<unit_template> new_instance();

   void init() override;
   void init_mws() override;
   void load() override;

private:
   unit_template();
};

#endif
