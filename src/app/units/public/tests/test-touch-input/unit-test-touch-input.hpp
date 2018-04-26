#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_TOUCH_INPUT

#include "unit.hpp"
#include "pfm.hpp"


class unit_test_touch_input : public unit
{
public:
   static mws_sp<unit_test_touch_input> new_instance();

   virtual void init_mws() override;

private:
   unit_test_touch_input();
};

#endif
