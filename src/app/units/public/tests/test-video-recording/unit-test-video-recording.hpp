#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_VIDEO_RECORDING

#include "unit.hpp"

class unit_test_video_recording : public unit
{
public:
   static std::shared_ptr<unit_test_video_recording> nwi();

   void init() override;
   void load() override;
   void update_view(int update_count) override;
   void post_update_view() override;
   void receive(std::shared_ptr<iadp> idp) override;

private:
   unit_test_video_recording();
};

#endif
