#include "stdafx.h"

#include "appplex-conf.hpp"
#include "video-enc.hpp"

#if defined MOD_FFMPEG

#include "app/units/public/tests/test-ffmpeg/ffmpeg/venc-ffmpeg.hpp"


std::shared_ptr<mws_video_enc> mws_video_enc::nwi()
{
   return std::make_shared<venc_ffmpeg>();
}

#elif defined PLATFORM_IOS

#include "ios/vid/ios-video-enc.hpp"


std::shared_ptr<mws_video_dec> mws_video_enc::nwi()
{
   return ios_video_enc::nwi();
}

#endif