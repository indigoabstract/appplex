#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_RES_LD

#include "unit.hpp"


class unit_test_res_ld : public unit
{
public:
   static shared_ptr<unit_test_res_ld> nwi();

   virtual void init();
   virtual void load();
   void update_view(int update_count) override;

private:
   unit_test_res_ld();
};

#endif
