#include "stdafx.h"

#include "appplex-conf.hpp"
#include "video-enc.hpp"
#include "video-dec.hpp"


bool mws_video_reencoder::is_decoding() const
{
   return get_dec_state() == mws_vdec_state::st_playing;
}

bool mws_video_reencoder::is_encoding() const
{
   return get_enc_state() == mws_vid_enc_st::e_st_encoding;
}


#if defined MOD_FFMPEG

#include "app/units/public/tests/test-ffmpeg/ffmpeg/venc-ffmpeg.hpp"


std::shared_ptr<mws_video_enc> mws_video_enc::nwi()
{
   return std::make_shared<venc_ffmpeg>();
}

std::shared_ptr<mws_video_reencoder> mws_video_reencoder::nwi()
{
   return mws_ffmpeg_reencoder::nwi();
}

#elif defined PLATFORM_IOS

#include "ios/vid/ios-video-enc.hpp"


std::shared_ptr<mws_video_enc> mws_video_enc::nwi()
{
   //return ios_video_enc::nwi();
    return nullptr;
}

std::shared_ptr<mws_video_reencoder> mws_video_reencoder::nwi()
{
   return nullptr;
}

#endif
