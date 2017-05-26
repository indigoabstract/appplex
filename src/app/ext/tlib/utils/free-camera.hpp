#pragma once

#include "min.hpp"
#include <glm/glm.hpp>

class gfx_camera;


class free_camera
{
public:
	free_camera();

	void update_input(shared_ptr<iadp> idp);
	void update();

	shared_ptr<gfx_camera> persp_cam;
	glm::vec3 look_at_dir;
	glm::vec3 up_dir;
	float speed;
	float mw_speed_factor;
};
