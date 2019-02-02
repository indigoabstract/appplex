#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "unit.hxx"


class unit_test_dyn_geometry_impl;

class unit_test_dyn_geometry : public unit
{
public:
   static mws_sp<unit_test_dyn_geometry> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   unit_test_dyn_geometry();
   virtual void receive(mws_sp<mws_dp> idp);

   mws_sp<unit_test_dyn_geometry_impl> p;
};

#endif
