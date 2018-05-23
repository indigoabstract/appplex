#pragma once

#include "media/vid/video-dec.hpp"
#include <memory>
#include <string>


class ios_video_dec_impl;


class ios_media_info : public mws_media_info
{
public:
   virtual int get_width() override;
   virtual int get_height() override;
   virtual int get_current_frame_index() override;
   virtual int64 get_frame_count() override;
   virtual double get_frame_rate() override;
   virtual unsigned long long get_duration_us() override;
   virtual std::shared_ptr<gfx_tex> get_current_frame() override;
   // get the total pixel content for a frame, including the padding
   virtual int get_total_width() override;
   virtual mws_vdec_state get_dec_state() override;
};


class ios_video_dec : public mws_video_dec
{
public:
   static std::shared_ptr<ios_video_dec> nwi();

   virtual ~ios_video_dec() {}
   virtual bool is_playing() const { return get_state() == mws_vdec_state::st_playing; }
   virtual std::string get_video_path() override;
   virtual void set_video_path(std::string i_video_path) override;
   virtual std::shared_ptr<mws_media_info> get_media_info() override;
   virtual int start_decoding() override;
   virtual void stop() override;
   virtual mws_vdec_state get_state() const override;
   virtual void update(std::shared_ptr<gfx_camera> i_mws_cam = nullptr) override;
   virtual void play() override;
   virtual void replay() override;
   virtual void pause() override;
   virtual void play_pause() override;
   virtual void goto_frame(int iframe_idx) override;
   virtual void next_frame() override;
   virtual void prev_frame() override;
   virtual void set_frame_limit(float iframe_limit) override;
   virtual void set_listener(std::shared_ptr<mws_vdec_listener> listener) override;

protected:
   ios_video_dec();
   
   std::unique_ptr<ios_video_dec_impl> p;
};
