#include "stdafx.h"

#include "unit-test-shadow-map.hpp"

#ifdef UNIT_TEST_SHADOW_MAP

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
#include <tiny-obj-loader/tiny_obj_loader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>


class unit_test_shadow_map_impl
{
public:
	unit_test_shadow_map_impl()
	{
		t = 0;
	}

	shared_ptr<gfx_rt> rt;
	shared_ptr<gfx_tex> rt_tex;
	shared_ptr<gfx_tex> rt_tex_globe;
	shared_ptr<gfx_plane> quad_mesh;
	shared_ptr<gfx_box> cube_mesh;
	shared_ptr<gfx_vpc_box> vpc_b_mesh;
	shared_ptr<gfx_vpc_kubic_sphere> vpc_ks_mesh;
	shared_ptr<gfx_vpc_ring_sphere> vpc_rs_mesh;
	shared_ptr<gfx_obj_vxo> obj_mesh[6];
	shared_ptr<gfx_camera> ortho_cam;
	shared_ptr<gfx_camera> persp_cam;
	glm::vec3 u_v3_light_dir;
	float t;
};


unit_test_shadow_map::unit_test_shadow_map()
{
	set_name("test-shadow-map");
}

shared_ptr<unit_test_shadow_map> unit_test_shadow_map::new_instance()
{
	return shared_ptr<unit_test_shadow_map>(new unit_test_shadow_map());
}

void unit_test_shadow_map::init()
{
	trx("test-shadow-map...");
}

void unit_test_shadow_map::load()
{
	p = shared_ptr<unit_test_shadow_map_impl>(new unit_test_shadow_map_impl());
	p->ortho_cam = gfx_camera::new_inst();
	p->ortho_cam->camera_id = "ortho_camera";
	p->ortho_cam->rendering_priority = 1;
	p->ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
	p->ortho_cam->near_clip_distance = -1;
	p->ortho_cam->far_clip_distance = 1;
	//sm_inst->ortho_cam->clear_color = true;
	//sm_inst->ortho_cam->clear_color_value = glm::vec4(0.f, 0.f, 1.f, 1.f);

	p->persp_cam = gfx_camera::new_inst();
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 0.1f;
	p->persp_cam->far_clip_distance = 1000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;
	p->persp_cam->position = glm::vec3(0.f, 0.f, 250.f);

	//std::string material = "[shader-name[fx_rays], u_s2d_diff_tex[texture-name[wr1_stuff01], addr-u[wrap], addr-v[wrap]], blending[none], transparent-sorting[true], depth-write[true], depth-function[less]]";
	//gl_material mat;
	//mat[MP_SHADER_NAME] = "fx_rays";
	//mat[MP_SHADER_NAME][MP_VSH_NAME] = "fx";
	////mat[MP_SHADER_NAME][MP_FSH_NAME] = "fx_rays";
	//mat["u_s2d_diff_tex"] = "wr1_stuff01";
	//mat["u_s2d_diff_tex"]["addr-u"] = "clamp";
	////mat["u_s2d_diff_tex"]["addr-v"] = "wrap";
	//mat["blending"] = "none";
	//mat["transparent-sorting"] = false;
	//mat["depth-write"] = true;
	//mat["depth-function"] = "greater";
	//mat["u_v3_light_dir"] = glm::vec3(1.f, 2.f, 3.f);
	////mat["material"] = material;
	//mws_print("\n\nxxx\n");
	//mat.debug_print();

	p->rt_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), 256, 256);
	p->rt = gfx::rt::new_rt();
	p->rt->set_color_attachment(p->rt_tex);

	shared_ptr<gfx_state> gl_st = gfx::get_gfx_state();

	gfx::rt::set_current_render_target(p->rt);
	decl_scgfxpl(pl1)
	{
		{gl::COLOR_CLEAR_VALUE, 0.35f, 0.35f, 0.65f, 0.09f},
		{gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL},
		{},
	};
	gl_st->set_state(pl1);

	gfx::rt::set_current_render_target(shared_ptr<gfx_rt>());

	p->u_v3_light_dir = -glm::vec3(-1.f, 1.f, 1.f);
	//std::string shader_name = "basic_tex";
	std::string shader_name = "l_d_o";

	p->quad_mesh = shared_ptr<gfx_plane>(new gfx_plane());
	gfx_plane& r_quad_mesh = *p->quad_mesh;
	//r_quad_mesh.camera_id_list.clear();
	r_quad_mesh.camera_id_list.push_back(p->ortho_cam->camera_id());
	r_quad_mesh.set_dimensions(200, 200);
	r_quad_mesh.position = glm::vec3(0.f, -50.f, 0.f);
	r_quad_mesh[MP_SHADER_NAME] = shader_name;
	r_quad_mesh["u_s2d_tex"] = p->rt_tex->get_name();
	r_quad_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	r_quad_mesh.scaling = glm::vec3(1, 10, 1);
	//r_quad_mesh.orientation = glm::quat(glm::vec3(3.1415f / 3, 0.f, 0.f));

	p->cube_mesh = shared_ptr<gfx_box>(new gfx_box());
	gfx_box& r_cube_mesh = *p->cube_mesh;
	r_cube_mesh.set_dimensions(50, 50, 50);
	r_cube_mesh.position = glm::vec3(0.f, 50.f, 0.f);
	r_cube_mesh[MP_SHADER_NAME] = shader_name;
	r_cube_mesh["u_s2d_tex"] = p->rt_tex->get_name();
	r_cube_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	p->vpc_b_mesh = shared_ptr<gfx_vpc_box>(new gfx_vpc_box());
	gfx_vpc_box& r_vpc_b_mesh = *p->vpc_b_mesh;
	r_vpc_b_mesh.render_method = GLPT_LINES;
	r_vpc_b_mesh.set_dimensions(50, 15);
	r_vpc_b_mesh.position = glm::vec3(0.f, 50.f, 0.f);
	r_vpc_b_mesh[MP_SHADER_NAME] = shader_name;
	r_vpc_b_mesh["u_s2d_tex"] = p->rt_tex->get_name();
	r_vpc_b_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	p->vpc_ks_mesh = shared_ptr<gfx_vpc_kubic_sphere>(new gfx_vpc_kubic_sphere());
	gfx_vpc_kubic_sphere& r_vpc_ks_mesh = *p->vpc_ks_mesh;
	//r_vpc_ks_mesh.render_method = GLPT_LINES;
	r_vpc_ks_mesh.set_dimensions(110, 15);
	r_vpc_ks_mesh.position = glm::vec3(0.f, 50.f, 0.f);
	r_vpc_ks_mesh[MP_SHADER_NAME] = "dbg_l_d_o";
	r_vpc_ks_mesh["u_s2d_tex"] = p->rt_tex->get_name();
	r_vpc_ks_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	p->vpc_rs_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
	gfx_vpc_ring_sphere& r_vpc_rs_mesh = *p->vpc_rs_mesh;
	//r_vpc_rs_mesh.render_method = GLPT_LINES;
	r_vpc_rs_mesh.set_dimensions(100, 75);
	r_vpc_rs_mesh.position = glm::vec3(0.f, 50.f, 0.f);
	r_vpc_rs_mesh[MP_SHADER_NAME] = "sp_l_d_o_globe3d";
	r_vpc_rs_mesh["u_s2d_day"] = "earth_flat_map.png";
	r_vpc_rs_mesh["u_s2d_night"] = "earth_night_flat_map.png";
	r_vpc_rs_mesh["u_s2d_clouds"] = "clouds_26Aug2011_8192.png";
	r_vpc_rs_mesh["u_v3_light_dir"] = p->u_v3_light_dir;

	for (int k = 0; k < 6; k++)
	{
		p->obj_mesh[k] = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
	}

	glm::vec3 pos_data[6] =
	{
		glm::vec3(-23.559,-1.37,-39.732), glm::vec3(-17.107,47.601,-25.071), glm::vec3(27.988,10.933,-23.373),
		glm::vec3(-23.968,-7.517,-19.171), glm::vec3(-23.968,-7.517,-19.171), glm::vec3(33.104,15.318,-21.16),
	};
	const char* mesh_names[6] =
	{
		"J251_WF_Grid", "J251_WF_fence", "J251_WF_adssigns", "J251_WF_terrain", "J251_WF_relief", "J251_WF_trees",
		//"winter-scene", 
	};
	const char* shader_names[6] =
	{
		"l_d_o", "l_d_o", "l_d_o", "l_d_o", "l_d_o", "l_d_o",
	};
	const char* tex_names[6] =
	{
		"grid_alpha", "WR_1_Fence_alpha", "wr1_stuff01", "terrain_WR_1", "terrain_WR_1", "WR_1_tree",
	};
	bool visible[6] =
	{
		true, true, true, true, true, true,
	};
	float s = 15.f;

	for (int k = 0; k < 6; k++)
	{
		p->obj_mesh[k]->position = glm::vec3(pos_data[k].x, pos_data[k].y, pos_data[k].z);
		(*p->obj_mesh[k]) = mesh_names[k] + std::string(".obj");
		(*p->obj_mesh[k])[MP_SHADER_NAME] = shader_names[k];
		(*p->obj_mesh[k])["u_s2d_tex"] = tex_names[k] + std::string(".png");
		(*p->obj_mesh[k])["u_v3_light_dir"] = p->u_v3_light_dir;
		p->obj_mesh[k]->visible = visible[k];
	}

	gfx_scene_inst->attach(p->persp_cam);
	gfx_scene_inst->attach(p->ortho_cam);
	//gfx_scene_inst->attach(impl->quad_mesh);
	//gfx_scene_inst->attach(impl->cube_mesh);
	//gfx_scene_inst->attach(impl->vpc_b_mesh);
	//gfx_scene_inst->attach(impl->vpc_ks_mesh);
	//gfx_scene_inst->attach(impl->vpc_rs_mesh);

	for (int k = 0; k < 6; k++)
	{
		gfx_scene_inst->attach(p->obj_mesh[k]);
	}

	mws_report_gfx_errs();
}

bool unit_test_shadow_map::update()
{
	float r = 85;
	float t = p->t;
	glm::vec3 pos = p->obj_mesh[0]->position;
	p->persp_cam->position = pos + glm::vec3(r * glm::cos(t), r * glm::sin(t), 50.f);
	p->persp_cam->look_at(pos - p->persp_cam->position(), glm::vec3(0.0f, 0.0f, 1.0f));

	p->u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);
	//persp_cam->position = glm::vec3(r * glm::sin(t), 125.f * glm::sin(t), r * glm::cos(t));
	//persp_cam->orientation = glm::quat(glm::vec3(0.f, t, 0.f));
	p->quad_mesh->orientation = glm::quat(glm::vec3(3 * glm::pi<float>() / 2, 0, 0));
	//cube_mesh->orientation = glm::quat(glm::vec3(t, t, t));
	//vpc_b_mesh->orientation = glm::quat(glm::vec3(t, t, t));
	p->vpc_ks_mesh->orientation = glm::quat(glm::vec3(0, t, 0));
	(*p->vpc_rs_mesh)["u_v3_light_dir"] = p->u_v3_light_dir;
	p->t += 0.01f;
	//shared_ptr<gl_state> gl_st = gl_ctrl::get_gl_state();
	//decl_scglpl(pl1)
	//{
	//	{gl::COLOR_CLEAR_VALUE, 1.f, 0.f, 1.f, 1.f},
	//	{gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL},
	//	{gl::BLEND, gl::TRUE_GL}, {gl::BLEND_SRC_DST, gl::SRC_ALPHA_GL, gl::ONE_MINUS_SRC_ALPHA_GL},
	//	{},
	//};
	//gl_st->set_state(pl1);

	mws_report_gfx_errs();

	return unit::update();
}

#endif
