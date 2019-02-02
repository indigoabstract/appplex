#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_STK

#include "unit.hxx"

class unit_test_stk : public unit
{
public:
   static mws_sp<unit_test_stk> nwi();

   virtual void init();
   virtual void init_mws();

private:
   unit_test_stk();
};

#endif
