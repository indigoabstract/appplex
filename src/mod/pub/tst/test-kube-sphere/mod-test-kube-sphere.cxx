#include "stdafx.hxx"

#include "mod-test-kube-sphere.hxx"
#include "icosphere.hxx"
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
#include "gfx-vxo-ext.hxx"
#include "gfx-ext/gfx-surface.hxx"
#include "tiny-obj-loader/tiny_obj_loader.hxx"
#include <glm/inc.hpp>


class mod_test_kube_sphere_impl
{
public:
	mod_test_kube_sphere_impl()
	{
		t = 0;
		t2 = 0;
		phi = 0;
		theta = 0;
		look_at_dir = glm::vec3(0.f, 0.f, -1.f);
		up_dir = glm::vec3(0.f, 1.f, 0.f);
		speed = 0;
		sphere_radius = 1500;
	}

	mws_sp<gfx_rt> rt;
	mws_sp<gfx_tex> rt_tex;
	mws_sp<gfx_box> skybox;
	mws_sp<gfx_vpc_kubic_sphere> vpc_ks_mesh;
	mws_sp<icosphere> ico_sphere;
	mws_sp<gfx_camera> ortho_cam;
	mws_sp<gfx_camera> persp_cam;
	glm::vec3 u_v3_light_dir;
	float t;
	float t2;
	float phi;
	float theta;
	glm::vec3 look_at_dir;
	glm::vec3 up_dir;
	float speed;
	float sphere_radius;
};


mod_test_kube_sphere::mod_test_kube_sphere() : mws_mod(mws_stringify(MOD_TEST_KUBE_SPHERE)) {}

mws_sp<mod_test_kube_sphere> mod_test_kube_sphere::nwi()
{
	return mws_sp<mod_test_kube_sphere>(new mod_test_kube_sphere());
}

void mod_test_kube_sphere::init()
{
	//touch_ctrl_inst->add_receiver(get_smtp_instance());
	//key_ctrl_inst->add_receiver(get_smtp_instance());
}

void mod_test_kube_sphere::load()
{
	p = mws_sp<mod_test_kube_sphere_impl>(new mod_test_kube_sphere_impl());
	p->ortho_cam = gfx_camera::nwi();
	p->ortho_cam->camera_id = "ortho_camera";
	p->ortho_cam->rendering_priority = 1;
	p->ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
	p->ortho_cam->near_clip_distance = -5000;
	p->ortho_cam->far_clip_distance = 5000;
	p->ortho_cam->clear_depth = false;

	p->persp_cam = gfx_camera::nwi();
	p->persp_cam->name = "defcam";
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 0.01f;
	p->persp_cam->far_clip_distance = 50000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;

   gfx_tex_params prm;

   prm.set_format_id("RGBA8");
   prm.set_rt_params();
   p->rt_tex = gfx::i()->tex.nwi(gfx_tex::gen_id(), 256, 256, &prm);
	p->rt = gfx::i()->rt.new_rt();
	p->rt->set_color_attachment(p->rt_tex);

	mws_sp<gfx_state> gl_st = gfx::i()->get_gfx_state();

	gfx::i()->rt.set_current_render_target(p->rt);
	decl_scgfxpl(pl1)
	{
		{gl::COLOR_CLEAR_VALUE, 0.35f, 0.35f, 0.65f, 0.09f},
		{gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL},
		{},
	};
	gl_st->set_state(pl1);

	gfx::i()->rt.set_current_render_target(mws_sp<gfx_rt>());

	p->u_v3_light_dir = -glm::vec3(-1.f, 1.f, 1.f);

	p->skybox = mws_sp<gfx_box>(new gfx_box());
	gfx_box& r_cube_mesh = *p->skybox;
	float s = p->persp_cam->far_clip_distance * 0.5f;
	r_cube_mesh.set_dimensions(s, s, s);
	r_cube_mesh[MP_SHADER_NAME] = "skybox";
	r_cube_mesh["u_scm_skybox"] = "skybx";
	r_cube_mesh[MP_CULL_BACK] = false;
	r_cube_mesh[MP_CULL_FRONT] = true;

	p->vpc_ks_mesh = mws_sp<gfx_vpc_kubic_sphere>(new gfx_vpc_kubic_sphere());
	gfx_vpc_kubic_sphere& r_vpc_ks_mesh = *p->vpc_ks_mesh;
	r_vpc_ks_mesh.set_dimensions(p->sphere_radius, 50);
	r_vpc_ks_mesh.position = glm::vec3(-p->sphere_radius * 3.f, 0.f, 0.f);
	r_vpc_ks_mesh[MP_SHADER_NAME] = "dbg_l_d_o";
	r_vpc_ks_mesh["u_s2d_tex"] = p->rt_tex->get_name();
	r_vpc_ks_mesh["u_v3_light_dir"] = p->u_v3_light_dir;
	//r_vpc_ks_mesh[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;

	p->ico_sphere = mws_sp<icosphere>(new icosphere());
	icosphere& r_ico_sphere = *p->ico_sphere;
	r_ico_sphere.set_dimensions(p->sphere_radius / 2, 150);
	r_ico_sphere.position = glm::vec3(0.f, 0.f, 0.f);
	r_ico_sphere[MP_SHADER_NAME] = "dbg_l_d_o";//"l_c_o";
	r_ico_sphere["u_s2d_tex"] = p->rt_tex->get_name();
	r_ico_sphere["u_v4_color"] = glm::vec4(1, 1, 1.f, 1.f);
	r_ico_sphere["u_v3_light_dir"] = p->u_v3_light_dir;
	//r_ico_sphere[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
	//r_ico_sphere[MP_CULL_BACK] = false;
	//r_ico_sphere[MP_CULL_FRONT] = true;

	gfx_scene_inst->attach(p->persp_cam);
	gfx_scene_inst->attach(p->ortho_cam);
	gfx_scene_inst->attach(p->vpc_ks_mesh);
	gfx_scene_inst->attach(p->ico_sphere);
	gfx_scene_inst->attach(p->skybox);

	p->persp_cam->position = glm::vec3(0.f, 0.f, p->sphere_radius + 15.f);

	mws_report_gfx_errs();
}

bool mod_test_kube_sphere::update()
{
	float t = p->t;
	float t2 = p->t2;
	p->skybox->position = p->persp_cam->position;
	p->u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);
	p->vpc_ks_mesh->orientation = glm::quat(glm::vec3(0, t, 0));
	p->ico_sphere->orientation = glm::quat(glm::vec3(t, t2, t));
	//impl->t += 0.00005f;
	//impl->t2 += 0.005f;

	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(500,1500.f,50), glm::vec4(1, 0, 0.f, 1.f), 1.f);

	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(5000, 0, 0), glm::vec4(1, 0, 0.f, 1.f), 1.f);
	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 5000, 0), glm::vec4(0, 1, 0.f, 1.f), 1.f);
	p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 0, 5000), glm::vec4(0, 0, 1.f, 1.f), 1.f);

	//impl->ortho_cam->draw_line(glm::vec3(100, 100, 0), glm::vec3(230, 540, 0), glm::vec4(0, 0, 1.f, 1.f), 10.f);
	//std::vector<ia_color>& face_colors = impl->ico_sphere->face_colors;
	//float y = 20;

	//for (std::vector<ia_color>::iterator it = face_colors.begin(); it != face_colors.end(); it++)
	//{
	//	impl->ortho_cam->draw_line(glm::vec3(50, y, 1), glm::vec3(150, y, 1), it->to_vec4(), 10);
	//	y += 30;
	//}

	p->persp_cam->position += p->look_at_dir * p->speed;
	p->persp_cam->look_at(p->look_at_dir, p->up_dir);

	int fc = p->ico_sphere->get_face_count();
	glm::vec3 forward_dir = p->persp_cam->get_forward_dir();

	for(int k = 0; k < fc; k++)
	{
		mws_sp<icosphere_face> face = p->ico_sphere->get_face_at(k);
		bool visible = false;

		for (int k = 0; k < 3; k++)
		{
			float nf_dot = glm::dot(face->corner_normals[k], -forward_dir);

			if(nf_dot >= 0.f)
			{
				visible = true;
			}
		}

		face->visible = visible;

		if(visible)
		{
			float d = glm::distance(p->persp_cam->position(), face->middle_point);

			if(d < 150)
			{
				face->gen_geometry(7);
			}
			else if(d < 350)
			{
				face->gen_geometry(6);
			}
			else if(d < 500)
			{
				face->gen_geometry(5);
			}
			else if(d < 1000)
			{
				face->gen_geometry(4);
			}
			else if(d < 2000)
			{
				face->gen_geometry(3);
			}
			else if(d < 3500)
			{
				face->gen_geometry(2);
			}
			else if(d < 5500)
			{
				face->gen_geometry(1);
			}
			else
			{
				face->gen_geometry(0);
			}
			//mws_print("dist %f\n", d);
		}
	}

	mws_report_gfx_errs();

	return mws_mod::update();
}

void mod_test_kube_sphere::receive(mws_sp<mws_dp> idp)
{
	if(!idp->is_processed())
	{
		if(idp->is_type(mws_ptr_evt::ptr_evt_type))
		{
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);

         mws_print("tn %s\n", ts->get_evt_type().c_str());
         if (ts->type == mws_ptr_evt::mouse_wheel)
         {
            p->persp_cam->position += p->look_at_dir * 150.f * float(ts->mouse_wheel_delta);
         }

			//if(ts->get_type() == touch_sym_evt::TS_PRESS_AND_DRAG)
			//{
			//	float dx = ts->points[0].x - ts->prev_state.te->points[0].x;
			//	float dy = ts->points[0].y - ts->prev_state.te->points[0].y;
			//	float dx_rad = glm::radians(dx / 2);
			//	float dy_rad = glm::radians(dy / 2);

			//	//impl->theta += glm::radians(dx / 10);
			//	//impl->phi += glm::radians(-dy / 10);
			//	//impl->theta = glm::mod(impl->theta, 2 * glm::pi<float>());
			//	//impl->phi = glm::mod(impl->phi, 2 * glm::pi<float>());
			//	//glm::vec3 axis(glm::sin(impl->theta) * glm::sin(impl->phi), glm::cos(impl->theta) * glm::sin(impl->phi), -glm::cos(impl->phi));
			//	//axis = glm::normalize(axis);
			//	//mws_print("x %f, y %f\n", impl->theta, impl->phi);
			//	//mws_print("x %f, y %f, z %f - th %f, ph %f\n", axis.x, axis.y, axis.z, impl->theta, impl->phi);
			//	glm::vec3 right_dir = glm::cross(p->look_at_dir, p->up_dir);
			//	glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
			//	p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_right_dir);
			//	p->up_dir = glm::normalize(glm::cross(right_dir, p->look_at_dir));

			//	glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, p->up_dir);
			//	p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_up_dir);
			//	ts->process();
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
						p->persp_cam->position -= p->look_at_dir * 0.05f;
						break;
					}

				case mws_key_e:
					{
						p->persp_cam->position += p->look_at_dir * 0.05f;
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
					case mws_key_space:
					case mws_key_f1:
						//vdec->play_pause();
						break;

					case mws_key_backspace:
					case mws_key_f2:
						//vdec->stop();
						break;

					case mws_key_f6:
						//mws_mod_ctrl::inst()->set_app_exit_on_next_run(true);
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
