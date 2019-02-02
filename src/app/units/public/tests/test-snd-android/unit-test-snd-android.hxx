#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_SND_ANDROID

#include "unit.hxx"


class unit_test_snd_android : public unit
{
public:
   static mws_sp<unit_test_snd_android> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

private:
   unit_test_snd_android();
};

#endif
