#pragma once

#include "pfm.hxx"
#include <memory>
#include <string>


class gfx;
class gfx_rt;
class gfx_tex;
class mws_vdec_listener;
enum class mws_vdec_state;


enum class mws_vid_enc_st
{
   e_st_encoding,
   e_st_finished,
};

enum class mws_vid_enc_method
{
   e_enc_m0,
   e_enc_m1,
   e_enc_m2,
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
   static mws_sp<mws_video_enc> nwi();

   virtual ~mws_video_enc() {}
   virtual bool is_encoding() const { return get_state() == mws_vid_enc_st::e_st_encoding; }
   virtual mws_vid_enc_st get_state() const = 0;
   virtual mws_vid_enc_method get_enc_method() const = 0;
   virtual mws_path get_video_path() = 0;
   virtual void set_video_path(mws_path i_video_path) = 0;
   virtual void start_encoding(const mws_video_params& i_prm, mws_vid_enc_method i_enc_method) = 0;
   virtual void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) = 0;
   virtual void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length) = 0;
   virtual void encode_frame_m2_rbga(mws_sp<gfx_tex> i_frame_tex) = 0;
   virtual void stop_encoding() = 0;

protected:
   mws_video_enc() {}
};


class mws_vreencoder_listener
{
public:
   // i_rt - you can draw to this FBO and use the attached texture as the next frame to be encoded
   // i_video_frame - a frame of the original video
   // return true to use as the frame the texture attached to i_rt, false will discard the frame in i_rt and use i_video_frame for encoding the next frame
   virtual bool on_reencode_frame(mws_sp<gfx_rt> i_rt, mws_sp<gfx_tex> i_video_frame) { return false; }
};


class mws_video_reencoder
{
public:
   static mws_sp<mws_video_reencoder> nwi();

   virtual ~mws_video_reencoder() {}
   virtual bool is_decoding() const;
   virtual bool is_encoding() const;
   virtual bool is_running() const;
   virtual mws_vdec_state get_dec_state() const = 0;
   virtual mws_vid_enc_st get_enc_state() const = 0;
   virtual mws_path get_src_video_path() = 0;
   virtual void set_src_video_path(mws_path i_video_path) = 0;
   virtual mws_path get_dst_video_path() = 0;
   virtual void set_dst_video_path(mws_path i_video_path) = 0;
   virtual void start_encoding(const mws_video_params& i_prm) = 0;
   virtual void stop_encoding() = 0;
   virtual void update() = 0;
   virtual void set_listener(mws_sp<mws_vdec_listener> i_listener) = 0;
   virtual void set_reencode_listener(mws_sp<mws_vreencoder_listener> i_listener) = 0;

protected:
   mws_video_reencoder() {}
};
