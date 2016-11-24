#include "stdafx.h"

#include "unit-test-ffmpeg.hpp"

#ifdef UNIT_TEST_FFMPEG

#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/unit/update-ctrl.hpp"
#include "ffmpeg/vdec-ffmpeg.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-state.hpp"
#include "ext/gfx-surface.hpp"
#include "com/ux/ux-camera.hpp"

using std::string;


namespace test_ffmpeg
{
	shared_ptr<vdec_ffmpeg> vdec;
	shared_ptr<gfx_quad_2d> q2d_tex;
	shared_ptr<gfx_quad_2d> q2d_rt_tex;
	shared_ptr<gfx_shader> texture_display;
}

using namespace test_ffmpeg;

unit_test_ffmpeg::unit_test_ffmpeg()
{
	set_name("test-ffmpeg");
}

shared_ptr<unit_test_ffmpeg> unit_test_ffmpeg::new_instance()
{
	return shared_ptr<unit_test_ffmpeg>(new unit_test_ffmpeg());
}

void unit_test_ffmpeg::init()
{
	trx("test-ffmpeg...");
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl->add_receiver(get_smtp_instance());
}

void unit_test_ffmpeg::load()
{
	ux_cam->clear_color = true;
	ux_cam->clear_color_value = gfx_color::colors::blue;
	q2d_tex = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());
	q2d_rt_tex = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

	gfx_quad_2d& q_tex = *q2d_tex;
	gfx_quad_2d& q_rt_tex = *q2d_rt_tex;

	texture_display = gfx::shader::get_program_by_name("texture_display");

	if (!texture_display)
	{
		texture_display = gfx::shader::new_program("texture_display", "basic_tex.vsh", "basic_tex.fsh");
		gfx_util::check_gfx_error();
	}

	gfx::shader::set_current_program(texture_display);

	q_tex[MP_CULL_BACK] = false;
	q_tex[MP_CULL_FRONT] = false;
	q_tex[MP_DEPTH_TEST] = false;
	q_tex.set_dimensions(2, 2);
	q_tex[MP_SHADER_NAME] = "basic_tex";

	q_rt_tex[MP_CULL_BACK] = false;
	q_rt_tex[MP_CULL_FRONT] = false;
	q_rt_tex[MP_DEPTH_TEST] = false;
	q_rt_tex.set_dimensions(2, 2);
	q_rt_tex[MP_SHADER_NAME] = "basic_tex";

	shared_ptr<pfm_file> raf = pfm_file::get_inst("giphy.mp4");

	std::string fpath = raf->get_full_path();
	vdec = shared_ptr<vdec_ffmpeg>(new vdec_ffmpeg());
	vdec->start_decoding(fpath.c_str());
	vdec->update(ux_cam);

	int fw = vdec->get_media_info()->get_width();
	int tw = vdec->get_media_info()->get_total_width();
	float hratio = float(fw) / tw;
	q_tex.set_tex_coord(glm::vec2(0.f), glm::vec2(hratio, 0.f), glm::vec2(hratio, 1.f), glm::vec2(0.f, 1.f));
	q_rt_tex.set_tex_coord(glm::vec2(0.f), glm::vec2(hratio, 0.f), glm::vec2(hratio, 1.f), glm::vec2(0.f, 1.f));

	if (!raf->exists())
	{
		vprint("\nfile %s not found\n", raf->get_file_name().c_str());
	}
}

bool unit_test_ffmpeg::update()
{
	touch_ctrl->update();
	key_ctrl->update();

	frame_count++;
	uint32 now = pfm::time::get_time_millis();
	uint32 dt = now - last_frame_time;

	if(dt >= 1000)
	{
		fps = frame_count * 1000.f / dt;
		last_frame_time = now;
		frame_count = 0;
	}

	shared_ptr<media_info> mi = vdec->get_media_info();
	shared_ptr<gfx_tex> tex = mi->get_current_frame();
	float p = float(mi->get_current_frame_index()) / mi->get_frame_count();
	float tw = mi->get_width();

	(*q2d_tex)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
	q2d_tex->position = glm::vec3(50.f + tw * 0.5, 50.f + tex->get_height() * 0.5, 0.f);
	q2d_tex->scaling = glm::vec3(tw * 0.5, tex->get_height() * 0.5, 1.f);

	(*q2d_rt_tex)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
	q2d_rt_tex->position = glm::vec3(50.f + p * tw * 0.5, 20 * 0.5 + tex->get_height() + 100, 0.f);
	q2d_rt_tex->scaling = glm::vec3(p * tw * 0.5, 20, 1.f);

	ux_cam->draw_mesh(q2d_tex);
	ux_cam->draw_mesh(q2d_rt_tex);

	vdec->update(ux_cam);

	if (vdec->get_state() == st_stopped)
	{
		vdec->replay();
		vdec->update(ux_cam);
	}

	gfx_util::check_gfx_error();

	return unit::update();
}

void unit_test_ffmpeg::receive(shared_ptr<iadp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

			//if(ts->get_type() == touch_sym_evt::TS_BACKWARD_SWIPE)
			//{
			//	back();
			//	ts->process();
			//}
		}
		else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if(ke->get_type() != key_evt::KE_RELEASED)
			{
				bool isAction = false;

				if(ke->get_type() != key_evt::KE_REPEATED)
				{
					isAction = true;

					switch(ke->get_key())
					{
					case KEY_SPACE:
					case KEY_F1:
						vdec->play_pause();
						break;

					case KEY_BACK:
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
						isAction = false;
					}
				}

				if(isAction)
				{
					ke->process();
				}
			}
		}
	}
}

#endif
