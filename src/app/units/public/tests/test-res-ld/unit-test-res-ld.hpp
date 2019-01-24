#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_RES_LD

#include "unit.hpp"


class unit_test_res_ld : public unit
{
public:
   static mws_sp<unit_test_res_ld> nwi();

   virtual void init() override;
   virtual void load() override;
   virtual void update_view(int update_count) override;

private:
   unit_test_res_ld();
};

#endif
