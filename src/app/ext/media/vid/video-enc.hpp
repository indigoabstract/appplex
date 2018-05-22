#pragma once

#include <memory>
#include <string>

class mws_video_enc_impl;


enum class mws_vid_enc_st
{
   e_st_encoding,
   e_st_finished,
};


class mws_video_params
{
public:
   mws_video_params();

   // the average bitrate
   int bit_rate;
   // resolution must be a multiple of two
   int width;
   int height;
   // this is the fundamental unit of time (in seconds) in terms of which frame timestamps are represented.For fixed - fps content,
   // timebase should be 1 / framerate and timestamp increments should be identically 1.
   int time_base_numerator;
   int time_base_denominator;
   // set to time_base ticks per frame. Default 1, e.g., H.264/MPEG-2 set it to 2
   int ticks_per_frame;
   // the number of pictures in a group of pictures, or 0 for intra_only
   // emits one intra frame every gop_size frames
   int gop_size;
   // maximum number of B-frames between non-B-frames
   int max_b_frames;
   // Pixel format, see AV_PIX_FMT_xxx.
   int pix_fmt;
   int codec_id;
   // see venc_ffmpeg::open_video or H.264 Video Encoding Guide for details
   std::string preset;
   std::string tune;
   int crf;
};


class mws_video_enc
{
public:
   static std::shared_ptr<mws_video_enc> nwi();

   virtual ~mws_video_enc() {}
   virtual bool is_encoding() const { return get_state() == mws_vid_enc_st::e_st_encoding; }
   virtual mws_vid_enc_st get_state() const = 0;
   virtual std::string get_video_path() = 0;
   virtual void set_video_path(std::string i_video_path) = 0;
   virtual void start_encoding(const mws_video_params& i_params) = 0;
   virtual void encode_frame(const char* iframe_data, int iframe_data_length) = 0;
   virtual void encode_yuv420_frame(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) = 0;
   virtual void stop_encoding() = 0;

protected:
   mws_video_enc() {}
};
