#include "stdafx.hxx"

#include "mod-test-grid.hxx"
#include "input/input-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-rt.hxx"
#include "gfx-camera.hxx"
#include "gfx-shader.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-vxo.hxx"
#include "gfx-state.hxx"
#include "tiny-obj-loader/tiny_obj_loader.hxx"
#include <glm/inc.hpp>


class mod_test_grid_impl
{
public:
	mod_test_grid_impl()
	{
		t = 0;
		look_at_dir = glm::vec3(0.f, 0.f, -1.f);
		up_dir = glm::vec3(0.f, 1.f, 0.f);
		speed = 0;

		float start = -(GRID_SIDE / 2) * GRID_SIZE;
		float end = (GRID_SIDE / 2) * GRID_SIZE;

		for (int i = 0; i < GRID_SIDE; i++)
		{
			float x = lerp(start, end, i / float(GRID_SIDE - 1.f));

			for (int j = 0; j < GRID_SIDE; j++)
			{
				float y = lerp(start, end, j / float(GRID_SIDE - 1.f));

				for (int k = 0; k < GRID_SIDE; k++)
				{
					float z = lerp(start, end, k / float(GRID_SIDE - 1.f));

					grid_3d[i][j][k] = glm::vec3(x, y, z);
				}
			}
		}
	}

	mws_sp<gfx_camera> ortho_cam;
	mws_sp<gfx_camera> persp_cam;
	glm::vec3 u_v3_light_dir;
	float t;
	glm::vec3 look_at_dir;
	glm::vec3 up_dir;
	float speed;

	static const int GRID_SIDE = 5;
	static const float GRID_SIZE;
	glm::vec3 grid_3d[GRID_SIDE][GRID_SIDE][GRID_SIDE];
};
const float mod_test_grid_impl::GRID_SIZE = 500.f;


mod_test_grid::mod_test_grid() : mws_mod(mws_stringify(MOD_TEST_GRID)) {}

mws_sp<mod_test_grid> mod_test_grid::nwi()
{
	return mws_sp<mod_test_grid>(new mod_test_grid());
}

void mod_test_grid::init()
{
	//touch_ctrl_inst->add_receiver(get_smtp_instance());
	//key_ctrl_inst->add_receiver(get_smtp_instance());
}

void mod_test_grid::load()
{
	p = mws_sp<mod_test_grid_impl>(new mod_test_grid_impl());

	p->persp_cam = gfx_camera::nwi();
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 5.1f;
	p->persp_cam->far_clip_distance = 50000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;
	p->persp_cam->position = glm::vec3(0.f, 0.f, 1500.f);

	p->u_v3_light_dir = -glm::vec3(-1.f, 1.f, 1.f);

	gfx_scene_inst->attach(p->persp_cam);

	mws_report_gfx_errs();
}

bool mod_test_grid::update()
{
	float t = p->t;

	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(500,1500.f,50), glm::vec4(1, 0, 0.f, 1.f), 1.f);

	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(5000, 0, 0), glm::vec4(1, 0, 0.f, 1.f), 1.f);
	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 5000, 0), glm::vec4(0, 1, 0.f, 1.f), 1.f);
	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 0, 5000), glm::vec4(0, 0, 1.f, 1.f), 1.f);

	for (int i = 0; i < p->GRID_SIDE; i++)
	{
		for (int j = 0; j < p->GRID_SIDE; j++)
		{
			for (int k = 0; k < p->GRID_SIDE; k++)
			{
				p->persp_cam->draw_point(p->grid_3d[i][j][k], glm::vec4(0, 0, 1.f, 1.f), 5.f);
			}
		}
	}

	mws_report_gfx_errs();

	p->persp_cam->position += p->look_at_dir * p->speed;
	p->persp_cam->look_at(p->look_at_dir, p->up_dir);
	p->u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);
	p->t += 0.0005f;

	return mws_mod::update();
}

void mod_test_grid::receive(mws_sp<mws_dp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(mws_ptr_evt::ptr_evt_type))
		{
			mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);

			//if(ts->type == touch_sym_evt::TS_PRESS_AND_DRAG)
			//{
			//	float dx = ts->points[0].x - ts->prev_state.te->points[0].x;
			//	float dy = ts->points[0].y - ts->prev_state.te->points[0].y;
			//	float dx_rad = glm::radians(dx / 2);
			//	float dy_rad = glm::radians(dy / 2);

			//	glm::vec3 right_dir = glm::cross(p->look_at_dir, p->up_dir);
			//	glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
			//	p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_right_dir);
			//	p->up_dir = glm::normalize(glm::cross(right_dir, p->look_at_dir));

			//	glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, p->up_dir);
			//	p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_up_dir);
			//	process(ts);
			//}
			//else if(ts->get_type() == touch_sym_evt::TS_MOUSE_WHEEL)
			//{
			//	mws_sp<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

			//	p->persp_cam->position += p->look_at_dir * 150.f * float(mw->wheel_delta);
			//}
		}
		else if(idp->is_type(mws_key_evt::key_evt_type))
		{
			mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

			if(ke->get_type() != mws_key_evt::ke_released)
			{
				bool do_action = true;

				switch(ke->get_key())
				{
				case mws_key_q:
					{
						p->persp_cam->position -= p->look_at_dir * 5.5f;
						break;
					}

				case mws_key_e:
					{
						p->persp_cam->position += p->look_at_dir * 5.5f;
						break;
					}

				case mws_key_a:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), p->look_at_dir);
						p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
						break;
					}

				case mws_key_d:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), p->look_at_dir);
						p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
						break;
					}

				case mws_key_z:
					{
						p->speed -= 0.05f;
						break;
					}

				case mws_key_c:
					{
						p->speed += 0.05f;
						break;
					}

				default:
					do_action = false;
				}

				if(!do_action && ke->get_type() != mws_key_evt::ke_repeated)
				{
					do_action = true;

					switch(ke->get_key())
					{
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
