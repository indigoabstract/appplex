#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "video-dec.hxx"

#if defined MOD_FFMPEG

#include "tst/test-ffmpeg/ffmpeg/vdec-ffmpeg.hxx"


mws_sp<mws_video_dec> mws_video_dec::nwi()
{
   return std::make_shared<vdec_ffmpeg>();
}

#elif defined PLATFORM_IOS

#include "ios/vid/ios-video-dec.hxx"


mws_sp<mws_video_dec> mws_video_dec::nwi()
{
   return ios_video_dec::nwi();
}

#else

mws_sp<mws_video_dec> mws_video_dec::nwi()
{
   return nullptr;
}

#endif