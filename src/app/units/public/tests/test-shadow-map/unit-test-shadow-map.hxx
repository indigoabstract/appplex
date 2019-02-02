#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_SHADOW_MAP

#include "unit.hxx"


class unit_test_shadow_map_impl;

class unit_test_shadow_map : public unit
{
public:
   static mws_sp<unit_test_shadow_map> nwi();

   virtual void init();
   virtual void load();
   virtual bool update();

private:
   unit_test_shadow_map();

   mws_sp<unit_test_shadow_map_impl> p;
};

#endif
