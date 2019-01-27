#include "stdafx.h"

#include "appplex-conf.hpp"
#include "video-dec.hpp"

#if defined MOD_FFMPEG

#include "app/units/public/tests/test-ffmpeg/ffmpeg/vdec-ffmpeg.hpp"


mws_sp<mws_video_dec> mws_video_dec::nwi()
{
   return std::make_shared<vdec_ffmpeg>();
}

#elif defined PLATFORM_IOS

#include "ios/vid/ios-video-dec.hpp"


mws_sp<mws_video_dec> mws_video_dec::nwi()
{
   return ios_video_dec::nwi();
}

#endif