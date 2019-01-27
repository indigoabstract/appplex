#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GRID

#include "unit.hpp"


class unit_test_grid_impl;

class unit_test_grid : public unit
{
public:
   static mws_sp<unit_test_grid> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   unit_test_grid();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<unit_test_grid_impl> p;
};

#endif
