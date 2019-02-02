#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_GL

#include "mod.hxx"


class mod_test_gl : public mws_mod
{
public:
   static mws_sp<mod_test_gl> nwi();

   virtual void init();
   virtual void load();
   void update_view(int update_count) override;

private:
   mod_test_gl();
};

#endif
