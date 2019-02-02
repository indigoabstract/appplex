#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_FFMPEG

#include "vid/video-enc.hxx"
#include "pfm.hxx"

extern "C"
{
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>

#include <ffmpeg/libavcodec/avcodec.h>
#include <ffmpeg/libavdevice/avdevice.h>
#include <ffmpeg/libavformat/avformat.h>
#include <ffmpeg/libavutil/opt.h>
#include <ffmpeg/libavutil/imgutils.h>
#include <ffmpeg/libswscale/swscale.h>
#include <ffmpeg/libswresample/swresample.h>
}

#include <string>


class mws_ffmpeg_reencoder_impl;
class mws_pbo_bundle;
class gfx_camera;


// a wrapper around a single output AVStream
class output_stream_ffmpeg
{
public:
   output_stream_ffmpeg()
   {
      st = nullptr;
      next_pts = 0;
      samples_count = 0;
      frame = nullptr;
      tmp_frame = nullptr;
      t = tincr = tincr2 = 0;
      sws_ctx = nullptr;
      swr_ctx = nullptr;
   }

   AVStream* st;

   // pts of the next frame that will be generated
   int64_t next_pts;
   int samples_count;

   AVFrame* frame;
   AVFrame* tmp_frame;

   float t, tincr, tincr2;

   struct SwsContext* sws_ctx;
   struct SwrContext* swr_ctx;
};


class venc_ffmpeg : public mws_video_enc
{
public:
   venc_ffmpeg();
   virtual ~venc_ffmpeg() {}
   mws_vid_enc_st get_state() const override;
   mws_vid_enc_method get_enc_method() const override;
   std::string get_video_path() override;
   void set_video_path(std::string i_video_path) override;
   void start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method) override;
   void encode_frame_impl(AVFrame* i_frame);
   void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) override;
   void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length) override;
   void encode_frame_m2_rbga(mws_sp<gfx_tex> i_frame_tex) override;
   void stop_encoding() override;
   void update();

private:
   void encode_frame_m0_yuv420_impl(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame);
   void encode_frame_m1_yuv420_impl(const char* iframe_data, int iframe_data_length);
   void encode_frame_m2_rbga_impl(mws_sp<gfx_tex> i_frame_tex);

   void open_audio(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void open_video(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void add_stream(output_stream_ffmpeg *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);

   mws_vid_enc_method enc_method;
   mws_sp<mws_video_params> params;
   output_stream_ffmpeg* ost;
   AVOutputFormat* fmt;
   AVDictionary* opt;
	FILE* f;
	AVFrame* frame;
   AVFormatContext* oc;
	std::string video_path;
	int pts_idx;
   bool have_video, have_audio;
   bool encode_video, encode_audio;
   output_stream_ffmpeg video_st;
   output_stream_ffmpeg audio_st;
   AVCodec* audio_codec;
   AVCodec* video_codec;
   mws_vid_enc_st state;
   // async gpu readback
   mws_sp<gfx_camera> ortho_cam;
   mws_sp<mws_pbo_bundle> pbo_b_y;
   mws_sp<mws_pbo_bundle> pbo_b_u;
   mws_sp<mws_pbo_bundle> pbo_b_v;
};


class mws_ffmpeg_reencoder : public mws_video_reencoder
{
public:
   static mws_sp<mws_ffmpeg_reencoder> nwi();

   virtual ~mws_ffmpeg_reencoder() {}
   virtual mws_vdec_state get_dec_state() const override;
   virtual mws_vid_enc_st get_enc_state() const override;
   std::string get_src_video_path() override;
   void set_src_video_path(std::string i_video_path) override;
   std::string get_dst_video_path() override;
   void set_dst_video_path(std::string i_video_path) override;
   void start_encoding(const mws_video_params& i_prm) override;
   void stop_encoding() override;
   void update() override;
   void set_listener(mws_sp<mws_vdec_listener> i_listener) override;
   void set_reencode_listener(mws_sp<mws_vreencoder_listener> i_listener) override;

protected:
   mws_ffmpeg_reencoder() {}

private:
   mws_sp<mws_ffmpeg_reencoder_impl> p;
};

#endif
