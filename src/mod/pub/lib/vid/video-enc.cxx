#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "video-enc.hxx"
#include "video-dec.hxx"


bool mws_video_reencoder::is_decoding() const
{
   return get_dec_state() == mws_vdec_state::st_playing;
}

bool mws_video_reencoder::is_encoding() const
{
   return get_enc_state() == mws_vid_enc_st::e_st_encoding;
}
bool mws_video_reencoder::is_running() const
{
   return is_decoding() || is_encoding();
}


#if defined MOD_FFMPEG

#include "tst/test-ffmpeg/ffmpeg/venc-ffmpeg.hxx"


mws_video_params::mws_video_params()
{
    bit_rate = 3000000;
    width = 0;
    height = 0;
    // frames per second
    time_base_numerator = 1;
    time_base_denominator = 24;
    ticks_per_frame = 1;
    // emit one intra frame every ten frames
    gop_size = 10;
    max_b_frames = 1;
    pix_fmt = AV_PIX_FMT_YUV420P;
    codec_id = AV_CODEC_ID_H264;
    preset = "ultrafast";
    tune = "film";
    crf = 0;
}


mws_sp<mws_video_enc> mws_video_enc::nwi()
{
   return std::make_shared<venc_ffmpeg>();
}

mws_sp<mws_video_reencoder> mws_video_reencoder::nwi()
{
   return mws_ffmpeg_reencoder::nwi();
}

#elif defined MWS_PFM_IOS

#include "ios/vid/ios-video-enc.hxx"


mws_video_params::mws_video_params()
{
    bit_rate = 3000000;
    width = 0;
    height = 0;
    // frames per second
    time_base_numerator = 1;
    time_base_denominator = 24;
    ticks_per_frame = 1;
    // emit one intra frame every ten frames
    gop_size = 10;
    max_b_frames = 1;
    pix_fmt = 0;
    codec_id = 0;
    preset = "ultrafast";
    tune = "film";
    crf = 0;
}


mws_sp<mws_video_enc> mws_video_enc::nwi()
{
   return ios_video_enc::nwi();
}

mws_sp<mws_video_reencoder> mws_video_reencoder::nwi()
{
   return ios_video_reencoder::nwi();
}

#else

mws_sp<mws_video_enc> mws_video_enc::nwi()
{
   return nullptr;
}

#endif
