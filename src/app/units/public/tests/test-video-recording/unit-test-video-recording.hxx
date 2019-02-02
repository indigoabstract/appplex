#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_VIDEO_RECORDING

#include "unit.hxx"

class unit_test_video_recording : public unit
{
public:
   static mws_sp<unit_test_video_recording> nwi();

   void init() override;
   void load() override;
   void update_view(int update_count) override;
   void post_update_view() override;
   void receive(mws_sp<mws_dp> idp) override;

private:
   unit_test_video_recording();
};

#endif
