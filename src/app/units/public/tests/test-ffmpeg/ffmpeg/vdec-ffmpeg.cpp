#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef MOD_FFMPEG

#include "vdec-ffmpeg.hpp"
#include "venc-ffmpeg.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-state.hpp"
#include "gfx-camera.hpp"
#include "ext/gfx-surface.hpp"
#include <string>
#include <cstdarg>

extern "C"
{
#define __STDC_CONSTANT_MACROS
#ifdef _STDINT_H
#undef _STDINT_H
#endif
#include <stdint.h>

#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavutil/opt.h"
#include "libavutil/imgutils.h"
}

#pragma comment (lib, "ffmpeg/avformat.lib")
#pragma comment(lib, "ffmpeg/avcodec.lib")
#pragma comment(lib, "ffmpeg/avutil.lib")


class vdec_ffmpeg_impl
{
public:
	vdec_ffmpeg_impl()
	{
		//av_log_set_callback(my_log_callback);
		//av_log_set_level(AV_LOG_VERBOSE);

		codec_ctx = nullptr;
		format_ctx = nullptr;
		av_stream = nullptr;
		video_stream = 0;
		av_codec = nullptr;
		av_frame = nullptr;

		tex_idx = 0;
		state = st_stopped;
		current_frame_idx = 0;
		frame_limit = 0.f;

		trx("ffmpeg: avcodec_register_all - register all formats and codecs");
		av_register_all();
	}

	~vdec_ffmpeg_impl()
	{
		free_memory();
	}

	shared_ptr<media_info> get_media_info()
	{
		return mi;
	}

	static void my_log_callback(void *ptr, int level, const char *fmt, va_list vargs)
	{
		static char message[8192];
		const char* module = NULL;

		// comment back in to filter only "important" messages
		//if (level > AV_LOG_WARNING)return;

		// get module name
		if (ptr)
		{
			AVClass *avc = *(AVClass**)ptr;
			module = avc->item_name(ptr);
		}

		// create the actual message
		vsnprintf(message, sizeof(message), fmt, vargs);
		mws_print("ffmpeg lvl: %d. ", level);

		// print the message
		if (module)
		{
			mws_print("module: %s. ", module);
		}

		mws_print("msg: %s\n", message);
	}

	int start_decoding(const char* ivideo_path)
	{
		free_memory();
		state = st_stopped;
		file_path = ivideo_path;
		current_frame_idx = 0;
		format_ctx = 0;
		int r = 0;
		AVDictionary* options = 0;
		char errbuf[256];

		r = avformat_open_input(&format_ctx, ivideo_path, nullptr, nullptr);

		if(r < 0)
		{
			av_strerror(r, errbuf, sizeof(errbuf));
			trx("vdec_ffmpeg::start_decoding: can't open file. error: {}", errbuf);

			return -1;
		}

		if(avformat_find_stream_info(format_ctx, nullptr) < 0)
		{
			trx("vdec_ffmpeg::start_decoding: can't find stream information");

			return -1;
		}

		// print format info
		av_dump_format(format_ctx, 0, ivideo_path, 0);
		video_stream = -1;

		for(uint32 i = 0; i < format_ctx->nb_streams; i++)
		{
			if(format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
			{
				video_stream = i;

				break;
			}
		}

		if(video_stream == -1)
		{
			trx("vdec_ffmpeg::start_decoding: didn't find a video stream");

			return -1;
		}

		av_stream = format_ctx->streams[video_stream];
		codec_ctx = av_stream->codec;

		av_codec = avcodec_find_decoder(codec_ctx->codec_id);

		if(av_codec == NULL)
		{
			trx("vdec_ffmpeg::start_decoding: unsupported codec");

			return -1;
		}

		if(avcodec_open2(codec_ctx, av_codec, NULL) < 0)
		{
			trx("vdec_ffmpeg::start_decoding: can't open codec");

			return -1;
		}

		av_frame = av_frame_alloc();

		int w = codec_ctx->width;
		int h = codec_ctx->height;
		int cw = codec_ctx->coded_width;
		int ch = codec_ctx->coded_height;

		conv_yuv420sp_2_rgb = gfx::shader::get_program_by_name("conv_yuv420sp_2_rgb");

		if (!conv_yuv420sp_2_rgb)
		{
			conv_yuv420sp_2_rgb = gfx::shader::new_program("conv_yuv420sp_2_rgb", "conv_yuv420sp_2_rgb.vsh", "conv_yuv420sp_2_rgb.fsh");
			mws_report_gfx_errs();
		}

		state = st_playing;
		last_frame_time = uint32(pfm::time::get_time_millis() + 1000.f / (frame_limit + 1.f));

		return 0;
	}

   void finish_decoding()
   {
      clear_rt();

      state = st_stopped;

      if (listener)
      {
         listener->on_decoding_finished();
      }
   }
  
   void stop_decoding()
	{
      if (listener)
      {
         listener->on_decoding_stopped();
      }

      finish_decoding();
	}

	void replay()
	{
		start_decoding(file_path.c_str());
	}

	vdec_state get_state()
	{
		return state;
	}

	void set_state(vdec_state istate)
	{
		switch(istate)
		{
		case st_stopped:
			switch(state)
			{
			case st_stopped:
				break;

			case st_playing:
				break;

			case st_paused:
				break;
			}
			break;

		case st_playing:
			switch(state)
			{
			case st_stopped:
				break;

			case st_playing:
				break;

			case st_paused:
				break;
			}
			break;

		case st_paused:
			switch(state)
			{
			case st_stopped:
				break;

			case st_playing:
				break;

			case st_paused:
				break;
			}
			break;
		}

		state = istate;
	}

	void update(std::shared_ptr<gfx_camera> i_mws_cam)
	{
		uint32 crt_frame_time = pfm::time::get_time_millis();

		if (frame_limit > 0.f)
		{
			float delta = float(crt_frame_time - last_frame_time);
			float limit_dt = 1000.f / frame_limit;

			if (delta < limit_dt)
			{
				return;
			}

			last_frame_time = crt_frame_time;
		}

		shared_ptr<gfx_rt> current_rt = gfx::rt::get_current_render_target();
		shared_ptr<gfx_shader> current_program = gfx::shader::get_current_program();

		gfx::rt::set_current_render_target(rt);
		frame_is_complete = 0;

		while(frame_is_complete == 0)
		{
			int r = decode_frame(i_mws_cam);

			if(r == 0)
			{
            finish_decoding();
            break;
			}
		}

		gfx::rt::set_current_render_target(current_rt);
	}

	void goto_frame(int iframe_idx)
	{
	}

	void next_frame()
	{
	}

	void prev_frame()
	{
	}

	void set_frame_limit(float iframe_limit)
	{
		frame_limit = iframe_limit;
	}

   void set_listener(std::shared_ptr<vdec_ffmpeg_listener> i_listener)
   {
      listener = i_listener;
   }

	shared_ptr<media_info> mi;

private:
	friend class media_info;

	int decode_frame(std::shared_ptr<gfx_camera> i_mws_cam)
	{
		int ret = 0;
		ret = av_read_frame(format_ctx, &av_packet);
		frame_is_complete = 0;

		if(ret >= 0)
		{
			if(av_packet.stream_index == video_stream)
			{
				avcodec_decode_video2(codec_ctx, av_frame, &frame_is_complete, &av_packet);

				if(frame_is_complete)
				{
					int w1 = av_frame->linesize[0];
					int w2 = av_frame->linesize[1];
					int h = codec_ctx->height;

					if (!(y_tex && y_tex->get_width() == w1 && y_tex->get_height() == h))
					{
						std::string tex_idx_str = trs("{}", tex_idx);

						q2d = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());
						//rt = nullptr;
						//rt_tex = y_tex = u_tex = v_tex = nullptr;

						y_tex = gfx::tex::new_tex_2d("u_s2d_y_tex" + tex_idx_str, w1, h, "R8");
						u_tex = gfx::tex::new_tex_2d("u_s2d_u_tex" + tex_idx_str, w2, h / 2, "R8");
						v_tex = gfx::tex::new_tex_2d("u_s2d_v_tex" + tex_idx_str, w2, h / 2, "R8");
						yt.resize(w1 * h);
						ut.resize(w2 * h / 2);
						vt.resize(w2 * h / 2);

						gfx_quad_2d& q_tex = *q2d;
						q_tex[MP_CULL_BACK] = false;
						q_tex[MP_CULL_FRONT] = false;
						q_tex[MP_DEPTH_TEST] = false;
						q_tex.set_dimensions(2, 2);
						q_tex[MP_SHADER_NAME] = "conv_yuv420sp_2_rgb";
						q_tex["u_s2d_y_tex"][MP_TEXTURE_INST] = y_tex;
						q_tex["u_s2d_u_tex"][MP_TEXTURE_INST] = u_tex;
						q_tex["u_s2d_v_tex"][MP_TEXTURE_INST] = v_tex;
						q_tex.scaling = glm::vec3(w1, h, 1.f);
						//q2d->set_v_flip(true);
						rt_tex = gfx::tex::new_tex_2d("u_s2d_tex" + tex_idx_str, w1, h);
						rt = gfx::rt::new_rt();
						rt->set_color_attachment(rt_tex);
						clear_rt();
						tex_idx++;
					}

					shared_ptr<gfx_shader> active_shader = gfx::shader::get_current_program();

					if (!active_shader)
					{
						active_shader = gfx::shader::get_program_by_name("black_shader");
						gfx::shader::set_current_program(active_shader);
					}

					y_tex->send_uniform("u_s2d_y_tex", 0);
					u_tex->send_uniform("u_s2d_u_tex", 1);
					v_tex->send_uniform("u_s2d_v_tex", 2);

					y_tex->update(0, (const char*)av_frame->data[0]);// , codec_ctx->width * codec_ctx->height);
					u_tex->update(1, (const char*)av_frame->data[1]);// , codec_ctx->width / 2 * codec_ctx->height / 2);
					v_tex->update(2, (const char*)av_frame->data[2]);// , codec_ctx->width / 2 * codec_ctx->height / 2);
					q2d->render_mesh(i_mws_cam);

					mws_report_gfx_errs();

               if (listener)
               {
                  if (current_frame_idx == 0)
                  {
                     auto params = std::make_shared<video_params_ffmpeg>();

                     params->bit_rate = codec_ctx->bit_rate;
                     //params->codec_id = codec_ctx->codec_id;
                     params->gop_size = codec_ctx->gop_size;
                     params->width = codec_ctx->width;
                     params->height = codec_ctx->height;
                     params->max_b_frames = codec_ctx->max_b_frames;
                     params->pix_fmt = codec_ctx->pix_fmt;
                     params->ticks_per_frame = codec_ctx->ticks_per_frame;
                     params->time_base = codec_ctx->time_base;
                     listener->on_decoding_started(params);
                  }

                  listener->on_frame_decoded(av_frame);
               }

					//mws_print("decode_frame counter %d\n", current_frame_idx);
					current_frame_idx++;
				}
			}

         av_packet_unref(&av_packet);

			return 1;
		}

		return 0;
	}

	void clear_rt()
	{
		gfx::rt::set_current_render_target(rt);
		shared_ptr<gfx_state> gl_st = gfx::get_gfx_state();
		decl_scgfxpl(pl1)
		{
			{gl::COLOR_CLEAR_VALUE, 0.f, 0.f, 0.f, 1.f},
			{gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL},
			{},
		};
		gl_st->set_state(pl1);
		gfx::rt::set_current_render_target(shared_ptr<gfx_rt>());
	}

	void free_memory()
	{
		if (av_frame)
		{
			av_free(av_frame);
			avcodec_close(codec_ctx);
         avformat_close_input(&format_ctx);

			av_frame = nullptr;
			codec_ctx = nullptr;
			format_ctx = nullptr;
		}
	}

	vdec_state state;
	shared_ptr<gfx_tex> y_tex;
	shared_ptr<gfx_tex> u_tex;
	shared_ptr<gfx_tex> v_tex;
	int frame_is_complete;
	AVCodecContext* codec_ctx;
	AVFormatContext* format_ctx;
	AVStream* av_stream;
	int video_stream;
	AVCodec* av_codec;
	AVFrame* av_frame; 
	AVPacket av_packet;
	std::vector<char> yt;
	std::vector<char> ut;
	std::vector<char> vt;
	shared_ptr<gfx_quad_2d> q2d;
	shared_ptr<gfx_shader> conv_yuv420sp_2_rgb;
	shared_ptr<gfx_rt> rt;
	shared_ptr<gfx_tex> rt_tex;
	int current_frame_idx;
	std::string file_path;
	int tex_idx;
	float frame_limit;
	uint32 last_frame_time;
   std::shared_ptr<vdec_ffmpeg_listener> listener;
};


int media_info::get_width()
{
	return impl.lock()->codec_ctx->width;
}

int media_info::get_height()
{
	return impl.lock()->codec_ctx->height;
}

int media_info::get_current_frame_index()
{
	return impl.lock()->current_frame_idx;
}

int64 media_info::get_frame_count()
{
	return impl.lock()->av_stream->nb_frames;
}

double media_info::get_frame_rate()
{
	shared_ptr<vdec_ffmpeg_impl> i = impl.lock();

	return i->codec_ctx->time_base.den / double(i->codec_ctx->time_base.num * i->codec_ctx->ticks_per_frame);
}

unsigned long long media_info::get_duration_us()
{
	return impl.lock()->format_ctx->duration;
}

shared_ptr<gfx_tex> media_info::get_current_frame()
{
	return impl.lock()->rt_tex;
}

int media_info::get_total_width()
{
	return impl.lock()->av_frame->linesize[0];
}

vdec_state media_info::get_dec_state()
{
	return impl.lock()->get_state();
}


vdec_ffmpeg::vdec_ffmpeg()
{
	impl = shared_ptr<vdec_ffmpeg_impl>(new vdec_ffmpeg_impl());
	impl->mi = shared_ptr<media_info>(new media_info(impl));
}

vdec_ffmpeg::~vdec_ffmpeg()
{
}

shared_ptr<media_info> vdec_ffmpeg::get_media_info()
{
	return impl->get_media_info();
}

int vdec_ffmpeg::start_decoding(const char* ivideo_path)
{
	return impl->start_decoding(ivideo_path);
}

void vdec_ffmpeg::stop()
{
	impl->stop_decoding();
}

vdec_state vdec_ffmpeg::get_state()
{
	return impl->get_state();
}

void vdec_ffmpeg::update(std::shared_ptr<gfx_camera> i_mws_cam)
{
	switch(get_state())
	{
	case st_stopped:
		break;

	case st_playing:
		{
			impl->update(i_mws_cam);
			break;
		}

	case st_paused:
		break;
	}
}

void vdec_ffmpeg::play()
{
	impl->set_state(st_playing);
}

void vdec_ffmpeg::replay()
{
	impl->replay();
}

void vdec_ffmpeg::pause()
{
	impl->set_state(st_paused);
}

void vdec_ffmpeg::play_pause()
{
	switch(get_state())
	{
	case st_stopped:
		impl->set_state(st_playing);
		break;

	case st_playing:
		impl->set_state(st_paused);
		break;

	case st_paused:
		impl->set_state(st_playing);
		break;
	}
}

void vdec_ffmpeg::goto_frame(int iframe_idx)
{
	impl->goto_frame(iframe_idx);
}

void vdec_ffmpeg::next_frame()
{
	impl->next_frame();
}

void vdec_ffmpeg::prev_frame()
{
	impl->prev_frame();
}

void vdec_ffmpeg::set_frame_limit(float iframe_limit)
{
	impl->set_frame_limit(iframe_limit);
}

void vdec_ffmpeg::set_listener(std::shared_ptr<vdec_ffmpeg_listener> listener)
{
   impl->set_listener(listener);
}

#endif
