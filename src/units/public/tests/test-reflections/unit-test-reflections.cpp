#include "stdafx.h"

#include "unit-test-reflections.hpp"

#ifdef UNIT_TEST_REFLECTIONS

#include "cpp-property.hpp"
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
#include "ext/gfx-surface.hpp"
#include <utils/free-camera.hpp>
#include <glm/glm.hpp>


class unit_test_reflections_impl
{
public:
	template <class host> class Number_Accessor2 : public number_accessor<host, glm::vec3>
	{
	public:
		virtual void set(host* ihost, const glm::vec3& ivalue)
		{
			this->val = ivalue;
		}

		virtual const glm::vec3& get(host* ihost) const 
		{
			return this->val;
		}
		friend host;
	};

	unit_test_reflections_impl() : Number(this)
	{
		Number = glm::vec3(22);
		//Number = Number() + glm::vec3(23);
		Number += glm::vec3(23);
		Number -= glm::vec3(21);
		const glm::vec3& xxx = Number;
		float x = Number().x;
		t = 0;
		free_cam = shared_ptr<free_camera>(new free_camera());
	}

	shared_ptr<gfx_box> skybox;
	shared_ptr<gfx_vpc_ring_sphere> vpc_rs_mesh;
	shared_ptr<gfx_vpc_ring_sphere> s_mesh;
	shared_ptr<gfx_camera> persp_cam;
	glm::vec3 u_v3_light_dir;
	float t;
	shared_ptr<free_camera> free_cam;
	def_number_prop(unit_test_reflections_impl, Number_Accessor2) Number;
};


unit_test_reflections::unit_test_reflections()
{
	set_name("test-reflections");
}

shared_ptr<unit_test_reflections> unit_test_reflections::new_instance()
{
	return shared_ptr<unit_test_reflections>(new unit_test_reflections());
}

void unit_test_reflections::init()
{
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl->add_receiver(get_smtp_instance());
}

void unit_test_reflections::load()
{
	p = shared_ptr<unit_test_reflections_impl>(new unit_test_reflections_impl());

	p->persp_cam = gfx_camera::new_inst();
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 0.1f;
	p->persp_cam->far_clip_distance = 50000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;
	p->persp_cam->position = glm::vec3(0.f, 0.f, 250.f);
	p->free_cam->persp_cam = p->persp_cam;

	p->u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);

	p->skybox = shared_ptr<gfx_box>(new gfx_box());
	gfx_box& r_cube_mesh = *p->skybox;
	float s = p->persp_cam->far_clip_distance * 0.5;
	r_cube_mesh.set_dimensions(s, s, s);
	r_cube_mesh[MP_SHADER_NAME] = "skybox";
	r_cube_mesh["u_scm_skybox"] = "skybx";
	r_cube_mesh[MP_CULL_BACK] = false;
	r_cube_mesh[MP_CULL_FRONT] = true;

	p->vpc_rs_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
	gfx_vpc_ring_sphere& r_vpc_rs_mesh = *p->vpc_rs_mesh;
	//r_vpc_rs_mesh.render_method = GLPT_LINES;
	r_vpc_rs_mesh.set_dimensions(100, 75);
	r_vpc_rs_mesh.position = glm::vec3(0.f, 0.f, 0.f);
	r_vpc_rs_mesh[MP_SHADER_NAME] = "sp_l_d_o_globe3d";
	r_vpc_rs_mesh["u_s2d_day"] = "earth_flat_map.png";
	r_vpc_rs_mesh["u_s2d_night"] = "earth_night_flat_map.png";
	r_vpc_rs_mesh["u_s2d_clouds"] = "clouds_26Aug2011_8192.png";
	r_vpc_rs_mesh["u_s2d_normal_specular_map"] = "earth_normal_spec_map.png";
	r_vpc_rs_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	p->s_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
	gfx_vpc_ring_sphere& rs_mesh = *p->s_mesh;
	rs_mesh.set_dimensions(100, 75);
	rs_mesh.position = glm::vec3(0.f, 0.f, 0.f);
	rs_mesh[MP_SHADER_NAME] = "l_d_o_test";
	//rs_mesh["u_s2d_day"] = "earth_flat_map.png";
	//rs_mesh["u_s2d_night"] = "earth_night_flat_map.png";
	//rs_mesh["u_s2d_clouds"] = "clouds_26Aug2011_8192.png";
	rs_mesh["u_scm_skybox"] = "ocean";
	rs_mesh["u_v3_light_dir"] = p->u_v3_light_dir;
	//rs_mesh[MP_WIREFRAME_MODE] = true;

	gfx_scene_inst->attach(p->persp_cam);
	//gfx_scene_inst->attach(p->s_mesh);
	gfx_scene_inst->attach(p->vpc_rs_mesh);
	gfx_scene_inst->attach(p->skybox);

	p->persp_cam->position = glm::vec3(0.f, 0.f, 200.f);
	p->free_cam->look_at_dir = glm::vec3(0.f, 0.25f, -1.f);
	p->free_cam->up_dir = glm::vec3(0.0f, 1.0f, 0.0f);

	gfx_util::check_gfx_error();
}

bool unit_test_reflections::update()
{
	float r = 85;
	float t = p->t;

	p->skybox->position = p->persp_cam->position;
	//p->u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);
	p->vpc_rs_mesh->orientation = glm::quat(glm::vec3(0, -t, 0));
	//(*p->s_mesh)["u_v3_light_dir"] = p->u_v3_light_dir;
	p->t += 0.001f;

	p->free_cam->update();

	gfx_util::check_gfx_error();

	return unit::update();
}

void unit_test_reflections::receive(shared_ptr<iadp> idp)
{
	p->free_cam->update_input(idp);
}

#endif
