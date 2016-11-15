#include "stdafx.h"

#include "free-camera.hpp"
#include "pfm.hpp"
#include "com/unit/input-ctrl.hpp"
#include "gfx-camera.hpp"


free_camera::free_camera()
{
	look_at_dir = glm::vec3(0.f, 0.f, -1.f);
	up_dir = glm::vec3(0.f, 1.f, 0.f);
	speed = 0.f;
	mw_speed_factor = 25.f;
}

void free_camera::update_input(shared_ptr<iadp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

			//vprint("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
			if(ts->get_type() == touch_sym_evt::TS_PRESS_AND_DRAG)
			{
				float dx = ts->crt_state.pos.x - ts->prev_state.pos.x;
				float dy = ts->crt_state.pos.y - ts->prev_state.pos.y;
				float dx_rad = glm::radians(dx / 2);
				float dy_rad = glm::radians(dy / 2);

				glm::vec3 right_dir = glm::cross(look_at_dir, up_dir);
				glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
				look_at_dir = glm::normalize(look_at_dir * rot_around_right_dir);
				up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));

				glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, up_dir);
				look_at_dir = glm::normalize(look_at_dir * rot_around_up_dir);
				ts->process();
			}
			else if(ts->get_type() == touch_sym_evt::TS_MOUSE_WHEEL)
			{
				shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

				persp_cam->position += look_at_dir * mw_speed_factor * float(mw->wheel_delta);
				ts->process();
			}
		}
		else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if(ke->get_type() != key_evt::KE_RELEASED)
			{
				bool isAction = true;

				switch(ke->get_key())
				{
				case KEY_Q:
					{
						persp_cam->position -= look_at_dir * 0.05f;
						break;
					}

				case KEY_E:
					{
						persp_cam->position += look_at_dir * 0.05f;
						break;
					}

				case KEY_A:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), look_at_dir);
						up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
						break;
					}

				case KEY_D:
					{
						glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), look_at_dir);
						up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
						break;
					}

				case KEY_Z:
					{
						speed -= 0.05f;
						break;
					}

				case KEY_C:
					{
						speed += 0.05f;
						break;
					}

				default:
					isAction = false;
				}

				if(!isAction && ke->get_type() != key_evt::KE_REPEATED)
				{
					isAction = true;

					switch(ke->get_key())
					{
					case KEY_SPACE:
					case KEY_F1:
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

void free_camera::update()
{
	persp_cam->position += look_at_dir * speed;
	persp_cam->look_at(look_at_dir, up_dir);
}
