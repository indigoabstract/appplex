#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG

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


enum class vid_enc_st
{
   e_st_encoding,
   e_st_finished,
};


class video_params_ffmpeg
{
public:
   video_params_ffmpeg();

   // the average bitrate
   int bit_rate;
   // resolution must be a multiple of two
   int width;
   int height;
   // this is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented.For fixed - fps content,
   // timebase should be 1 / framerate and timestamp increments should be identically 1.
   AVRational time_base;
   // set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2
   int ticks_per_frame;
   // the number of pictures in a group of pictures, or 0 for intra_only
   // emits one intra frame every gop_size frames
   int gop_size;
   // maximum number of B-frames between non-B-frames
   int max_b_frames;
   // Pixel format, see AV_PIX_FMT_xxx.
   enum AVPixelFormat pix_fmt;
   enum AVCodecID codec_id;
};


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


class venc_ffmpeg
{
public:
   venc_ffmpeg();
   vid_enc_st get_state() const;
   bool is_encoding() const;
   void start_encoding(const char* ivideo_path, const video_params_ffmpeg& i_params);
   void encode_frame(AVFrame* i_frame);
	void encode_frame(const char* iframe_data, int iframe_data_length);
   void encode_yuv420_frame(const uint8* y_frame, const uint32* uv_frame);
   void stop_encoding();

private:
   void open_audio(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void open_video(AVFormatContext *oc, AVCodec *codec, output_stream_ffmpeg *ost, AVDictionary *opt_arg);
   void add_stream(output_stream_ffmpeg *ost, AVFormatContext *oc, AVCodec **codec, enum AVCodecID codec_id);

   video_params_ffmpeg params;
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
   vid_enc_st state;
};

#endif
