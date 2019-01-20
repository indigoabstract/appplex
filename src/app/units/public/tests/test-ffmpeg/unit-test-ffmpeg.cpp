#include "stdafx.h"

#include "unit-test-ffmpeg.hpp"

#ifdef UNIT_TEST_FFMPEG

#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/unit/update-ctrl.hpp"
#include "ffmpeg/vdec-ffmpeg.hpp"
#include "ffmpeg/venc-ffmpeg.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-state.hpp"
#include "ext/gfx-surface.hpp"
#include "com/mws/mws-camera.hpp"

using std::string;


namespace test_ffmpeg
{
   shared_ptr<vdec_ffmpeg> vdec;
   shared_ptr<venc_ffmpeg> venc;
   shared_ptr<mws_vdec_listener> vdec_listener;
   shared_ptr<gfx_quad_2d> q2d_tex;
	shared_ptr<gfx_quad_2d> q2d_rt_tex;
}

using namespace test_ffmpeg;

unit_test_ffmpeg::unit_test_ffmpeg() : unit(mws_stringify(UNIT_TEST_FFMPEG)) {}

shared_ptr<unit_test_ffmpeg> unit_test_ffmpeg::nwi()
{
	return shared_ptr<unit_test_ffmpeg>(new unit_test_ffmpeg());
}

void unit_test_ffmpeg::init()
{
	trx("test-ffmpeg...");
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl_inst->add_receiver(get_smtp_instance());
}

void unit_test_ffmpeg::load()
{
	mws_cam->clear_color = true;
	mws_cam->clear_color_value = gfx_color::colors::blue;
	q2d_tex = gfx_quad_2d::nwi();
	q2d_rt_tex = gfx_quad_2d::nwi();

	gfx_quad_2d& q_tex = *q2d_tex;
	gfx_quad_2d& q_rt_tex = *q2d_rt_tex;

	q_tex[MP_CULL_BACK] = false;
	q_tex[MP_CULL_FRONT] = false;
	q_tex[MP_DEPTH_TEST] = false;
	q_tex.set_dimensions(1, 1);
	q_tex[MP_SHADER_NAME] = "basic-tex-shader";
   q2d_tex->set_translation(50.f, 50.f);

	q_rt_tex[MP_CULL_BACK] = false;
	q_rt_tex[MP_CULL_FRONT] = false;
	q_rt_tex[MP_DEPTH_TEST] = false;
	q_rt_tex.set_dimensions(1, 1);
	q_rt_tex[MP_SHADER_NAME] = "basic-tex-shader";

   {
      class vdec_ffmpeg_listener_impl : public mws_vdec_listener
      {
      public:
         virtual void on_start(std::shared_ptr<mws_video_params> i_params) override
         {
            venc->set_video_path("test-vid.mp4");
            venc->start_encoding(*i_params, mws_vid_enc_method::e_enc_m0);
         }

         virtual void on_frame_decoded(void* i_frame) override
         {
            venc->encode_frame_impl((AVFrame*)i_frame);
         }

         virtual void on_finish() override
         {
            venc->stop_encoding();
            vdec->set_listener(nullptr);
         }
      };

      venc = std::make_shared<venc_ffmpeg>();
      vdec_listener = std::make_shared<mws_vdec_listener>();
   }
   {
      shared_ptr<pfm_file> raf = pfm_file::get_inst("video.mp4");

      std::string fpath = raf->get_full_path();
      vdec = shared_ptr<vdec_ffmpeg>(new vdec_ffmpeg());
      // get notified when a frame is ready and then encode it
      //vdec->set_listener(vdec_listener);
      vdec->set_video_path(fpath);
      vdec->start_decoding();
      vdec->update();

      int fw = vdec->get_media_info()->get_width();
      int tw = vdec->get_media_info()->get_total_width();
      float hratio = float(fw) / tw;
      q_tex.set_tex_coord(glm::vec2(0.f), glm::vec2(hratio, 0.f), glm::vec2(hratio, 1.f), glm::vec2(0.f, 1.f));
      q_rt_tex.set_tex_coord(glm::vec2(0.f), glm::vec2(hratio, 0.f), glm::vec2(hratio, 1.f), glm::vec2(0.f, 1.f));

      if (!raf->exists())
      {
         mws_print("\nfile %s not found\n", raf->get_file_name().c_str());
      }
   }
}

void unit_test_ffmpeg::update_view(int update_count)
{
	shared_ptr<mws_media_info> mi = vdec->get_media_info();
	shared_ptr<gfx_tex> tex = mi->get_current_frame();
	float p = float(mi->get_current_frame_index()) / mi->get_frame_count();
	float tw = (float)mi->get_width();
   float th = (float)tex->get_height();
   auto fps_d = mi->get_frame_rate();

	(*q2d_tex)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
   q2d_tex->set_scale(tw, th);

	(*q2d_rt_tex)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
   q2d_rt_tex->set_translation(50.f, 20 + th + 100);
   q2d_rt_tex->set_scale(p * tw, 20);

   q2d_tex->draw_out_of_sync(mws_cam);
   q2d_rt_tex->draw_out_of_sync(mws_cam);

	vdec->update(mws_cam);

	if (vdec->get_state() == mws_vdec_state::st_stopped)
	{
		vdec->replay();
		vdec->update();
	}

	mws_report_gfx_errs();
}

void unit_test_ffmpeg::receive(shared_ptr<iadp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);
		}
		else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if(ke->get_type() != key_evt::KE_RELEASED)
			{
				bool do_action = false;

				if(ke->get_type() != key_evt::KE_REPEATED)
				{
					do_action = true;

					switch(ke->get_key())
					{
					case KEY_SPACE:
					case KEY_F1:
						vdec->play_pause();
						break;

					case KEY_BACKSPACE:
					case KEY_F2:
						vdec->stop();
						break;

					case KEY_F6:
						unit_ctrl::inst()->set_app_exit_on_next_run(true);
						break;

					case KEY_F11:
						pfm::screen::set_full_screen_mode(!pfm::screen::is_full_screen_mode());
						break;

					default:
						do_action = false;
					}
				}

				if(do_action)
				{
					ke->process();
				}
			}
		}
	}
}

#endif
