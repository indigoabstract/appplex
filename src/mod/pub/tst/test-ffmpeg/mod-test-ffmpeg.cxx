#include "stdafx.hxx"

#include "mod-test-ffmpeg.hxx"
#include "mws-mod-ctrl.hxx"
#include "input/input-ctrl.hxx"
#include "input/update-ctrl.hxx"
#include "ffmpeg/vdec-ffmpeg.hxx"
#include "ffmpeg/venc-ffmpeg.hxx"
#include "gfx.hxx"
#include "gfx-rt.hxx"
#include "gfx-shader.hxx"
#include "gfx-vxo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-state.hxx"
#include "gfx-vxo-ext.hxx"
#include "mws/mws-camera.hxx"

using std::string;


namespace test_ffmpeg
{
   mws_sp<vdec_ffmpeg> vdec;
   mws_sp<venc_ffmpeg> venc;
   mws_sp<mws_vdec_listener> vdec_listener;
   mws_sp<gfx_quad_2d> q2d_tex;
	mws_sp<gfx_quad_2d> q2d_rt_tex;
}

using namespace test_ffmpeg;

mod_test_ffmpeg::mod_test_ffmpeg() : mws_mod(mws_stringify(MOD_TEST_FFMPEG)) {}

mws_sp<mod_test_ffmpeg> mod_test_ffmpeg::nwi()
{
	return mws_sp<mod_test_ffmpeg>(new mod_test_ffmpeg());
}

void mod_test_ffmpeg::init()
{
	trx("test-ffmpeg...");
	//touch_ctrl_inst->add_receiver(get_smtp_instance());
	//key_ctrl_inst->add_receiver(get_smtp_instance());
}

void mod_test_ffmpeg::load()
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
	q_tex[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
   q2d_tex->set_translation(50.f, 50.f);

	q_rt_tex[MP_CULL_BACK] = false;
	q_rt_tex[MP_CULL_FRONT] = false;
	q_rt_tex[MP_DEPTH_TEST] = false;
	q_rt_tex.set_dimensions(1, 1);
	q_rt_tex[MP_SHADER_NAME] = gfx::basic_tex_sh_id;

   {
      class vdec_ffmpeg_listener_impl : public mws_vdec_listener
      {
      public:
         virtual void on_start(mws_sp<mws_video_params> i_params) override
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
      mws_sp<mws_file> raf = mws_file::get_inst("video.mp4");

      std::string fpath = raf->string_path();
      vdec = mws_sp<vdec_ffmpeg>(new vdec_ffmpeg());
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
         mws_print("\nfile %s not found\n", raf->filename().c_str());
      }
   }
}

void mod_test_ffmpeg::update_view(int update_count)
{
	mws_sp<mws_media_info> mi = vdec->get_media_info();
	mws_sp<gfx_tex> tex = mi->get_current_frame();
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

void mod_test_ffmpeg::receive(mws_sp<mws_dp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(mws_ptr_evt::ptr_evt_type))
		{
			mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);
		}
		else if(idp->is_type(mws_key_evt::key_evt_type))
		{
			mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

			if(ke->get_type() != mws_key_evt::ke_released)
			{
				bool do_action = false;

				if(ke->get_type() != mws_key_evt::ke_repeated)
				{
					do_action = true;

					switch(ke->get_key())
					{
					case mws_key_space:
					case mws_key_f1:
						vdec->play_pause();
						break;

					case mws_key_backspace:
					case mws_key_f2:
						vdec->stop();
						break;

					case mws_key_f6:
						mws_mod_ctrl::inst()->set_app_exit_on_next_run(true);
						break;

					case mws_key_f11:
						mws::screen::set_full_screen_mode(!mws::screen::is_full_screen_mode());
						break;

					default:
						do_action = false;
					}
				}

				if(do_action)
				{
					process(ke);
				}
			}
		}
	}
}
