#include "stdafx.h"

#include "unit-test-grid.hpp"

#ifdef UNIT_TEST_GRID

#include "com/unit/input-ctrl.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include <tiny-obj-loader/tiny_obj_loader.hpp>
#include <glm/inc.hpp>


class unit_test_grid_impl
{
public:
	unit_test_grid_impl()
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

	shared_ptr<gfx_camera> ortho_cam;
	shared_ptr<gfx_camera> persp_cam;
	glm::vec3 u_v3_light_dir;
	float t;
	glm::vec3 look_at_dir;
	glm::vec3 up_dir;
	float speed;

	static const int GRID_SIDE = 5;
	static const float GRID_SIZE;
	glm::vec3 grid_3d[GRID_SIDE][GRID_SIDE][GRID_SIDE];
};
const float unit_test_grid_impl::GRID_SIZE = 500.f;


unit_test_grid::unit_test_grid()
{
	set_name("test-grid");
}

shared_ptr<unit_test_grid> unit_test_grid::nwi()
{
	return shared_ptr<unit_test_grid>(new unit_test_grid());
}

void unit_test_grid::init()
{
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl->add_receiver(get_smtp_instance());
}

void unit_test_grid::load()
{
	p = shared_ptr<unit_test_grid_impl>(new unit_test_grid_impl());

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

bool unit_test_grid::update()
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

	return unit::update();
}

void unit_test_grid::receive(shared_ptr<iadp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);

			if(ts->get_type() == touch_sym_evt::TS_PRESS_AND_DRAG)
			{
				float dx = ts->points[0].x - ts->prev_state.te->points[0].x;
				float dy = ts->points[0].y - ts->prev_state.te->points[0].y;
				float dx_rad = glm::radians(dx / 2);
				float dy_rad = glm::radians(dy / 2);

				glm::vec3 right_dir = glm::cross(p->look_at_dir, p->up_dir);
				glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
				p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_right_dir);
				p->up_dir = glm::normalize(glm::cross(right_dir, p->look_at_dir));

				glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, p->up_dir);
				p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_up_dir);
				ts->process();
			}
			else if(ts->get_type() == touch_sym_evt::TS_MOUSE_WHEEL)
			{
				shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

				p->persp_cam->position += p->look_at_dir * 150.f * float(mw->wheel_delta);
			}
		}
		else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if(ke->get_type() != key_evt::KE_RELEASED)
			{
				bool do_action = true;

				switch(ke->get_key())
				{
				case KEY_Q:
					{
						p->persp_cam->position -= p->look_at_dir * 5.5f;
						break;
					}

				case KEY_E:
					{
						p->persp_cam->position += p->look_at_dir * 5.5f;
						break;
					}

				case KEY_A:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), p->look_at_dir);
						p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
						break;
					}

				case KEY_D:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), p->look_at_dir);
						p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
						break;
					}

				case KEY_Z:
					{
						p->speed -= 0.05f;
						break;
					}

				case KEY_C:
					{
						p->speed += 0.05f;
						break;
					}

				default:
					do_action = false;
				}

				if(!do_action && ke->get_type() != key_evt::KE_REPEATED)
				{
					do_action = true;

					switch(ke->get_key())
					{
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
