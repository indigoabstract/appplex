#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_OPENVG

#include "unit.hpp"


class unit_test_openvg : public unit
{
public:
   static mws_sp<unit_test_openvg> nwi();

   virtual void init();
   virtual void on_destroy();
   virtual void init_mws();
   virtual void load();

private:
   unit_test_openvg();

   void init_shivavg();
   void destroy_shivavg();
   void init_monkvg();
   void destroy_monkvg();
};

#endif
