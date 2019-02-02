#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_FFMPEG

#include "unit.hxx"


class unit_test_ffmpeg : public unit
{
public:
   static mws_sp<unit_test_ffmpeg> nwi();

   virtual void init();
   virtual void load();
   virtual void update_view(int update_count);

private:
   unit_test_ffmpeg();
   virtual void receive(mws_sp<mws_dp> idp);
};

#endif
