#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GL

#include "unit.hpp"


class unit_test_gl : public unit
{
public:
   static mws_sp<unit_test_gl> nwi();

   virtual void init();
   virtual void load();
   void update_view(int update_count) override;

private:
   unit_test_gl();
};

#endif
