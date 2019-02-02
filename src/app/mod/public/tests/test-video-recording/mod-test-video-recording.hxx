#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_VIDEO_RECORDING

#include "mod.hxx"

class mod_test_video_recording : public mws_mod
{
public:
   static mws_sp<mod_test_video_recording> nwi();

   void init() override;
   void load() override;
   void update_view(int update_count) override;
   void post_update_view() override;
   void receive(mws_sp<mws_dp> idp) override;

private:
   mod_test_video_recording();
};

#endif
