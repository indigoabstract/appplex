#pragma once

#include <memory>
#include <string>

class mws_video_params;
class mws_media_info;
class gfx;
class gfx_camera;
class gfx_tex;


enum class mws_vdec_state
{
   st_stopped,
   st_playing,
   st_paused,
};


class mws_vdec_listener
{
public:
   virtual void on_decoding_started(std::shared_ptr<gfx> i_gi, std::shared_ptr<mws_video_params> i_params) {}
   virtual void on_frame_decoded(void* i_frame) {}
   virtual void on_frame_decoded(std::shared_ptr<gfx> i_gi, std::shared_ptr<gfx_tex> i_frame_tex) {}
   virtual void on_decoding_stopped() {}
   virtual void on_decoding_finished() {}
};


class mws_media_info
{
public:
   virtual int get_width() = 0;
   virtual int get_height() = 0;
   virtual int get_current_frame_index() = 0;
   virtual int64 get_frame_count() = 0;
   virtual double get_frame_rate() = 0;
   virtual unsigned long long get_duration_us() = 0;
   virtual std::shared_ptr<gfx_tex> get_current_frame() = 0;
   // get the total pixel content for a frame, including the padding
   virtual int get_total_width() = 0;
   virtual mws_vdec_state get_dec_state() = 0;
};


class mws_video_dec
{
public:
   static std::shared_ptr<mws_video_dec> nwi();

   virtual ~mws_video_dec() {}
   virtual bool is_playing() const { return get_state() == mws_vdec_state::st_playing; }
   virtual std::string get_video_path() = 0;
   virtual void set_video_path(std::string i_video_path) = 0;
   virtual std::shared_ptr<mws_media_info> get_media_info() = 0;
   virtual int start_decoding() = 0;
   virtual void stop() = 0;
   virtual mws_vdec_state get_state() const = 0;
   virtual void update(std::shared_ptr<gfx_camera> i_mws_cam = nullptr) = 0;
   virtual void play() = 0;
   virtual void replay() = 0;
   virtual void pause() = 0;
   virtual void play_pause() = 0;
   virtual void goto_frame(int iframe_idx) = 0;
   virtual void next_frame() = 0;
   virtual void prev_frame() = 0;
   virtual void set_frame_limit(float iframe_limit) = 0;
   virtual void set_listener(std::shared_ptr<mws_vdec_listener> listener) = 0;

protected:
   mws_video_dec() {}
};
