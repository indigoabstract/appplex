#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_TRAIL

#include "unit.hpp"


class unit_test_trail_impl;

class unit_test_trail : public unit
{
public:
   static mws_sp<unit_test_trail> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   unit_test_trail();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<unit_test_trail_impl> p;
};

#endif
