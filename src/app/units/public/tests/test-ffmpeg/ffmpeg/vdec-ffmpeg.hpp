#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG
#include <memory>

class gfx_camera;
class gfx_tex;
class vdec_ffmpeg_impl;
class video_params_ffmpeg;
struct AVFrame;


enum vdec_state
{
	st_stopped,
	st_playing,
	st_paused,
};


class media_info
{
public:
	media_info(std::shared_ptr<vdec_ffmpeg_impl> iimpl) : impl(iimpl){}
	int get_width();
	int get_height();
	int get_current_frame_index();
	int64 get_frame_count();
	double get_frame_rate();
	unsigned long long get_duration_us();
	std::shared_ptr<gfx_tex> get_current_frame();
	// get the total pixel content for a frame, including the padding
	int get_total_width();
	vdec_state get_dec_state();

private:
	std::weak_ptr<vdec_ffmpeg_impl> impl;
};


class vdec_ffmpeg_listener
{
public:
   virtual void on_decoding_started(std::shared_ptr<video_params_ffmpeg> i_params) {}
   virtual void on_frame_decoded(AVFrame* i_frame) {}
   virtual void on_decoding_stopped() {}
   virtual void on_decoding_finished() {}
};


class vdec_ffmpeg
{
public:
	vdec_ffmpeg();
	~vdec_ffmpeg();
	std::shared_ptr<media_info> get_media_info();
	int start_decoding(const char* ivideo_path);
	void stop();
	vdec_state get_state();
	void update(std::shared_ptr<gfx_camera> i_mws_cam);
	void play();
	void replay();
	void pause();
	void play_pause();
	void goto_frame(int iframe_idx);
	void next_frame();
	void prev_frame();
	void set_frame_limit(float iframe_limit);
   void set_listener(std::shared_ptr<vdec_ffmpeg_listener> listener);

private:
	std::shared_ptr<vdec_ffmpeg_impl> impl;
};

#endif
