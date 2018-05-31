#pragma once

#include <memory>
#include <string>


class gfx;
class gfx_tex;
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
   static std::shared_ptr<mws_video_enc> nwi();

   virtual ~mws_video_enc() {}
   virtual bool is_encoding() const { return get_state() == mws_vid_enc_st::e_st_encoding; }
   virtual mws_vid_enc_st get_state() const = 0;
   virtual mws_vid_enc_method get_enc_method() const = 0;
   virtual std::string get_video_path() = 0;
   virtual void set_video_path(std::string i_video_path) = 0;
   virtual void start_encoding(std::shared_ptr<gfx> i_gi, const mws_video_params& i_prm, mws_vid_enc_method i_enc_method) = 0;
   virtual void encode_frame_m0_yuv420(const uint8* y_frame, const uint8* u_frame, const uint8* v_frame) = 0;
   virtual void encode_frame_m1_yuv420(const char* iframe_data, int iframe_data_length) = 0;
   virtual void encode_frame_m2_rbga(std::shared_ptr<gfx> i_gi, std::shared_ptr<gfx_tex> i_frame_tex) = 0;
   virtual void stop_encoding() = 0;

protected:
   mws_video_enc() {}
};


class mws_video_reencoder_listener
{
public:
   virtual void on_decoding_started(std::shared_ptr<gfx> i_gi, const mws_video_params& i_params) {}
   // i_gfx_inst - gl context instance
   // i_frame_tex - texture containing the decoded frame, with the same resolution as the source video
   // returns - a texture with the same resolution as the specified mws_video_params (and created with the same context instance), containing the frame to be encoded
   // or nullptr, to use leave the current frame unchanged
   virtual std::shared_ptr<gfx_tex> on_frame_decoded(std::shared_ptr<gfx> i_gfx_inst, std::shared_ptr<gfx_tex> i_frame_tex) = 0;
   virtual void on_decoding_stopped() {}
   virtual void on_decoding_finished() {}
};


class mws_video_reencoder
{
public:
   static std::shared_ptr<mws_video_reencoder> nwi();

   virtual ~mws_video_reencoder() {}
   virtual bool is_decoding() const;
   virtual bool is_encoding() const;
   virtual mws_vdec_state get_dec_state() const = 0;
   virtual mws_vid_enc_st get_enc_state() const = 0;
   virtual std::string get_src_video_path() = 0;
   virtual void set_src_video_path(std::string i_video_path) = 0;
   virtual std::string get_dst_video_path() = 0;
   virtual void set_dst_video_path(std::string i_video_path) = 0;
   virtual void start_encoding(const mws_video_params& i_prm) = 0;
   virtual void stop_encoding() = 0;
   virtual void update() = 0;
   virtual void set_listener(std::shared_ptr<mws_video_reencoder_listener> listener) = 0;

protected:
   mws_video_reencoder() {}
};
