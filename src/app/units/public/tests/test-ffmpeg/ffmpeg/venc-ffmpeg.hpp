#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG

#include "media/vid/video-enc.hpp"

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
   virtual mws_vid_enc_st get_state() const override;
   virtual std::string get_video_path() override;
   virtual void set_video_path(std::string i_video_path) override;
   virtual void start_encoding(const mws_video_params& i_params) override;
   void encode_frame(AVFrame* i_frame);
   virtual void encode_frame(const char* iframe_data, int iframe_data_length) override;
   virtual void encode_yuv420_frame(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) override;
   virtual void stop_encoding() override;

private:
   void open_audio(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void open_video(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void add_stream(output_stream_ffmpeg *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);

   mws_video_params params;
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
};

#endif
