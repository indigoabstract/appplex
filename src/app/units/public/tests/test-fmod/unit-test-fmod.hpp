#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FMOD

#include "unit.hpp"

class unit_test_fmod : public unit
{
public:
   static mws_sp<unit_test_fmod> nwi();

   virtual void init();
   virtual void init_mws();

private:
   unit_test_fmod();
};

#endif
