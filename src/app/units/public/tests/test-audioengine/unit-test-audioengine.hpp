#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_AUDIOENGINE

#include "unit.hpp"


class unit_test_audioengine : public unit
{
public:
   static mws_sp<unit_test_audioengine> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   unit_test_audioengine();
};

#endif
