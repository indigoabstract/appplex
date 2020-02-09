#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_FFMPEG

#include "vid/video-dec.hxx"
#include <memory>

class gfx_camera;
class gfx_tex;
class vdec_ffmpeg_impl;
class video_params_ffmpeg;
struct AVFrame;


class ffmpeg_media_info : public mws_media_info
{
public:
   ffmpeg_media_info(mws_sp<vdec_ffmpeg_impl> iimpl) : impl(iimpl){}
	int get_width();
	int get_height();
	int get_current_frame_index();
	int64 get_frame_count();
	double get_frame_rate();
	unsigned long long get_duration_us();
	mws_sp<gfx_tex> get_current_frame();
	// get the total pixel content for a frame, including the padding
	int get_total_width();
   mws_vdec_state get_dec_state();

private:
	mws_wp<vdec_ffmpeg_impl> impl;
};


class vdec_ffmpeg : public mws_video_dec
{
public:
	vdec_ffmpeg();
	virtual ~vdec_ffmpeg();
   virtual mws_path get_video_path() override;
   virtual void set_video_path(mws_path i_video_path) override;
   virtual mws_sp<mws_media_info> get_media_info() override;
   virtual int start_decoding() override;
   virtual void stop() override;
   virtual  mws_vdec_state get_state() const override;
   virtual void update(mws_sp<gfx_camera> i_mws_cam = nullptr) override;
   virtual void play() override;
   virtual void replay() override;
   virtual void pause() override;
   virtual void play_pause() override;
   virtual void goto_frame(int iframe_idx) override;
   virtual void next_frame() override;
   virtual void prev_frame() override;
   virtual void set_frame_limit(float iframe_limit) override;
   virtual void set_listener(mws_sp<mws_vdec_listener> listener) override;

private:
	mws_sp<vdec_ffmpeg_impl> impl;
};

#endif
