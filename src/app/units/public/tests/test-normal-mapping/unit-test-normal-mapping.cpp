#include "stdafx.h"

#include "unit-test-normal-mapping.hpp"

#ifdef UNIT_TEST_NORMAL_MAPPING

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
#include "gfx-color.hpp"
#include "ext/gfx-surface.hpp"
#include "media/res-ld/res-ld.hpp"
#include <rng/rng.hpp>
#include <utils/free-camera.hpp>
#include <glm/glm.hpp>
#include <map>


namespace ns_unit_test_normal_mapping
{
	class ux_select_button : public gfx_plane
	{
	public:
		ux_select_button(shared_ptr<unit_test_normal_mapping> iunit, int ibutton_id, std::string itex_name)
		{
			unit = iunit;
			button_id = ibutton_id;
			tex_name = itex_name;
			z_pos = -1;
			is_init = false;
		}

		shared_ptr<unit_test_normal_mapping> get_unit()
		{
			return unit.lock();
		}

		bool is_hit(float ix, float iy)
		{
			float x_size = x_percent_size * get_unit()->get_width();
			float y_size = y_percent_size * get_unit()->get_height();
			float boxx = x_percent_pos * get_unit()->get_width();// + x_size / 2;
			float boxy = y_percent_pos * get_unit()->get_height();// + y_size / 2;
			float boxWidth = x_size;
			float boxHeight = y_size;

			return is_inside_box(ix, iy, boxx, boxy, boxWidth, boxHeight);
		}

		// percent of screen res
		void set_dim(float ix_percent_pos, float iy_percent_pos, float ix_percent_size, float iy_percent_size)
		{
			init();

			x_percent_size = ix_percent_size;
			y_percent_size = iy_percent_size;

			float x_size = x_percent_size * get_unit()->get_width();
			float y_size = y_percent_size * get_unit()->get_height();

			set_dimensions(x_size, y_size);

			x_percent_pos = ix_percent_pos;
			y_percent_pos = iy_percent_pos;

			float x_pos = x_percent_pos * get_unit()->get_width() + x_size / 2;
			float y_pos = y_percent_pos * get_unit()->get_height() + y_size / 2;

			glm::vec3 pos(x_pos, y_pos, z_pos);

			position = pos;
		}

		void init()
		{
			if(!is_init)
			{
				is_init = true;
				ux_select_button& inst = *static_pointer_cast<ux_select_button>(get_shared_ptr());

				inst[MP_SHADER_NAME] = "basic_tex";
				inst[MP_BLENDING] = MV_ALPHA;
				//inst["u_v4_color"] = ia_color::colors::dodger_blue.to_vec4();
				inst["u_s2d_tex"] = tex_name;
			}
		}

		int button_id;
		std::string tex_name;
		float z_pos;
		float x_percent_pos, y_percent_pos;
		float x_percent_size, y_percent_size;
		bool is_init;
		weak_ptr<unit_test_normal_mapping> unit;
	};


	struct obj_tf
	{
		obj_tf(std::vector<shared_ptr<gfx_vxo> > imesh_list, glm::quat iinit_orientation)
		{
			mesh_list = imesh_list;
			init_orientation = iinit_orientation;
		}

		std::vector<shared_ptr<gfx_vxo> > mesh_list;
		glm::quat init_orientation;
	};


	class impl
	{
	public:
		enum e_material_types
		{
			mt_metallic_paint,
			mt_metallic_paint3,
			mt_car_wheel_rim,
			mt_car_tyre,
			mt_car_carbon_fiber,
			mt_car_glass,
			mt_car_headlights,
			mt_car_headlights_glass,
			mt_car_chassis,
			mt_car_interior,
		};

		impl(shared_ptr<unit_test_normal_mapping> iunit)
		{
			unit = iunit;
			t = 0;
			skybox = shared_ptr<gfx_box>(new gfx_box());
			s_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
			plane_mesh = shared_ptr<gfx_plane>(new gfx_plane());
			persp_cam = gfx_camera::new_inst();
			free_cam = shared_ptr<free_camera>(new free_camera());
			is_paused = false;
			current_car_idx = 0;
			draw_axis = false;

			mat_car_windows = shared_ptr<gfx_material>(new gfx_material());
			mat_car_tyres = shared_ptr<gfx_material>(new gfx_material());
			mat_car_carbon_fiber = shared_ptr<gfx_material>(new gfx_material());
			mat_car_wheel_rims = shared_ptr<gfx_material>(new gfx_material());
			mat_car_body = shared_ptr<gfx_material>(new gfx_material());
			mat_car_body_3 = shared_ptr<gfx_material>(new gfx_material());
			mat_car_chassis = shared_ptr<gfx_material>(new gfx_material());
			mat_car_interior = shared_ptr<gfx_material>(new gfx_material());
			mat_car_headlights = shared_ptr<gfx_material>(new gfx_material());
			mat_car_headlights_glass = shared_ptr<gfx_material>(new gfx_material());

			shared_ptr<ux_select_button> b;
			float y = 0.01;
			float off = 0.14;

			b = shared_ptr<ux_select_button>(new ux_select_button(iunit, 0, "button-left"));
			b->set_dim(0.01, y, 0.2, 0.1);
			button_list.push_back(b);

			b = shared_ptr<ux_select_button>(new ux_select_button(iunit, 1, "button-right"));
			y += off;
			b->set_dim(0.01, y, 0.2, 0.1);
			button_list.push_back(b);
		}

		shared_ptr<unit_test_normal_mapping> get_unit()
		{
			return unit.lock();
		}

		void set_car_idx(int icar_idx, shared_ptr<gfx_scene> gfx_scene_inst)
		{
			std::vector<shared_ptr<gfx_vxo> > mesh_tab;

			if(obj_tf_inst)
			{
				std::vector<shared_ptr<gfx_vxo> >& mesh_list = obj_tf_inst->mesh_list;

				for(int k = 0; k < mesh_list.size(); k++)
				{
					mesh_list[k]->detach();
				}
			}

			t = 0;
			current_car_idx = icar_idx;
			std::string scene_name = scene_idx_map[icar_idx];
			obj_tf_inst = obj_map[scene_name];

			std::vector<shared_ptr<gfx_vxo> >& mesh_list = obj_tf_inst->mesh_list;

			for(int k = 0; k < mesh_list.size(); k++)
			{
				//mesh_list[k]->get_material()->clear_entries();
				mesh_tab.push_back(mesh_list[k]);
			}

			if(s_mesh->get_parent())
			{
				s_mesh->detach();
			}

			switch(current_car_idx)
			{
			case 0:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.0275, 0.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.f, 0.0083f, 0.1579, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.2706, 0.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 1:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.4f, 0.f, 0.349999f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.6f, 0.f, 0.f, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.f, 0.35f, -0.35f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.501961f, 0.5f, 0.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 2:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint2";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.50275, 0.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.f, 0.0083f, 0.579, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.2706, 0.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 3:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint2";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.750275, 0.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.5f, 0.0083f, 0.079, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.92706, 0.f, 0.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 4:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint2";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.0, 0.f, 1.f, 0.0750275f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.025f, 0.0083f, 0.079, 0.05f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.0, 0.f, 0.1f, 0.0f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 5:
				{
					mesh_tab.push_back(s_mesh);
					shared_ptr<gfx_material> new_mat(new gfx_material());
					gfx_material& mat = *new_mat;

					mat[MP_SHADER_NAME] = "metallic-paint2";
					mat["u_s2d_base_normal_map"] = "car-normal-map";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					glm::vec4 chrome(0.75, 0.75f, 0.75f, 0.0f);

					mat["u_v4_paint_front_color"] = chrome;
					mat["u_v4_paint_middle_color"] = chrome;
					mat["u_v4_paint_back_color"] = chrome;
					mat["u_v4_flake_color"] = glm::vec4(0.90164, 0.9268, 1.f, 0.f);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(new_mat);
					}
					break;
				}

			case 6:
				// wheel rim
				{
					mesh_tab.push_back(s_mesh);
					load_material(mt_car_wheel_rim);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(mat_car_wheel_rims);
					}
					break;
				}

			case 7:
				// car tyre
				{
					mesh_tab.push_back(s_mesh);
					load_material(mt_car_tyre);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(mat_car_tyres);
					}
					break;
				}

			case 8:
				// car glass
				{
					mesh_tab.push_back(s_mesh);
					load_material(mt_car_glass);

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->set_material(mat_car_windows);
					}
					break;
				}

			case 9:
			case 10:
				// watchdogs cars
				{
					mesh_tab[0]->look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
					obj_tf_inst->init_orientation = mesh_tab[0]->orientation;

					for(int k = 0; k <= mt_car_interior; k++)
					{
						load_material((e_material_types)k);
					}

					for(int k = 0; k < mesh_tab.size(); k++)
					{
						mesh_tab[k]->orientation = obj_tf_inst->init_orientation;
					}
					break;
				}
			}

			for(int k = 0; k < mesh_tab.size(); k++)
			{
				gfx_scene_inst->attach(mesh_tab[k]);
			}
		}

		void load_material(e_material_types imat_type)
		{
			switch(imat_type)
			{
			case mt_metallic_paint:
				{
					gfx_material& mat = *mat_car_body;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_metallic_paint_2";//"sp_metallic_paint";
					mat["u_s2d_noise_map"] = "tex11";
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.f, 1.f, 0.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.f, 0.0083f, 0.1579, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.2706, 0.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_metallic_paint3:
				{
					gfx_material& mat = *mat_car_body_3;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_metallic_paint_3";//"sp_metallic_paint";
					mat["u_s2d_noise_map"] = "tex11";
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(0.f, 1.f, 0.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(0.f, 0.0083f, 0.1579, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(0.2706, 0.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_wheel_rim:
				{
					gfx_material& mat = *mat_car_wheel_rims;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_wheel_rim";
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					float f = 0.975f;
					glm::vec4 chrome(f, f, f, 0.0f);

					mat["u_v4_paint_front_color"] = chrome;
					mat["u_v4_paint_middle_color"] = chrome;
					mat["u_v4_paint_back_color"] = chrome;
					mat["u_v4_flake_color"] = glm::vec4(0.90164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_tyre:
				{
					gfx_material& mat = *mat_car_tyres;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_tyre";
					mat["u_s2d_diffuse_map"] = "GLOBAL_Tires_D";
					mat["u_s2d_base_normal_map"] = "GLOBAL_Tires_N";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 0.1f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_carbon_fiber:
				{
					gfx_material& mat = *mat_car_carbon_fiber;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_carbon_fiber";
					mat["u_s2d_diffuse_map"] = "Carbon_Fiber";
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 0.1f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_glass:
				{
					gfx_material& mat = *mat_car_windows;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_glass";
					mat[MP_BLENDING] = MV_ALPHA;
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_headlights:
				{
					gfx_material& mat = *mat_car_headlights;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_tyre";
					mat["u_s2d_diffuse_map"] = "GLOBAL_Lights_D";
					mat["u_s2d_base_normal_map"] = "GLOBAL_Lights_N";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 0.1f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_headlights_glass:
				{
					gfx_material& mat = *mat_car_headlights_glass;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_glass";
					mat[MP_BLENDING] = MV_ALPHA;
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_chassis:
				{
					gfx_material& mat = *mat_car_chassis;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_wheel_rim";
					mat["u_s2d_base_normal_map"] = "Generic_Normals";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 8.f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					float f = 0.5f;
					glm::vec4 chrome(f, f, f, 0.0f);

					mat["u_v4_paint_front_color"] = chrome;
					mat["u_v4_paint_middle_color"] = chrome;
					mat["u_v4_paint_back_color"] = chrome;
					mat["u_v4_flake_color"] = glm::vec4(0.90164, 0.9268, 1.f, 0.f);
					break;
				}

			case mt_car_interior:
				{
					gfx_material& mat = *mat_car_interior;
					mat.clear_entries();

					mat[MP_SHADER_NAME] = "sp_car_tyre";
					mat["u_s2d_diffuse_map"] = "GLOBAL_Interior_D";
					mat["u_s2d_base_normal_map"] = "GLOBAL_Interior_N";
					mat["u_s2d_micro_flackes_normal_map"] = "sparkle-normal-map";
					mat["u_scm_skybox"] = "showroom";
					mat["u_v1_flake_nrm_perturbation2"] = 1.f;
					mat["u_v1_fresnel_exponent"] = 16.f;
					mat["u_v1_brightness_factor"] = 0.1f;
					mat["u_v1_gloss_level"] = 0.f;
					mat["u_v1_nrm_perturbation"] = 1.f;
					mat["u_v1_flake_nrm_perturbation1"] = 0.1f;

					mat["u_v4_paint_front_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_paint_middle_color"] = glm::vec4(1.f, 1.0f, 1.0, 0.f);
					mat["u_v4_paint_back_color"] = glm::vec4(1.0, 1.f, 1.f, 0.f);
					mat["u_v4_flake_color"] = glm::vec4(0.0164, 0.9268, 1.f, 0.f);
					break;
				}
			}
		}

		void init(shared_ptr<gfx_scene> gfx_scene_inst)
		{
			persp_cam->camera_id = "default";
			persp_cam->rendering_priority = 0;
			persp_cam->near_clip_distance = 0.1f;
			persp_cam->far_clip_distance = 50000.f;
			persp_cam->fov_y_deg = 60.f;
			persp_cam->clear_color = true;
			persp_cam->clear_color_value = gfx_color::colors::black;
			persp_cam->clear_depth = true;
			persp_cam->position = glm::vec3(0.f, 0.f, 250.f);
			free_cam->persp_cam = persp_cam;
			u_v3_light_dir = glm::normalize(-glm::vec3(1.f, 0.5f, 0.5f));

			ortho_cam = gfx_camera::new_inst();
			ortho_cam->camera_id = "ortho_cam";
			ortho_cam->rendering_priority = 1;
			ortho_cam->projection_type = "orthographic";
			ortho_cam->near_clip_distance = -100;
			ortho_cam->far_clip_distance = 100;
			ortho_cam->clear_color = false;
			//ortho_cam->clear_color_value = ia_color::colors::black.to_vec4();
			ortho_cam->clear_depth = true;

			// cubemap
			{
				gfx_box& r_cube_mesh = *skybox;
				float s = persp_cam->far_clip_distance * 0.5;
				r_cube_mesh.set_dimensions(s, s, s);
				r_cube_mesh[MP_SHADER_NAME] = "skybox";
				r_cube_mesh["u_scm_skybox"] = "showroom";
				r_cube_mesh[MP_CULL_BACK] = false;
				r_cube_mesh[MP_CULL_FRONT] = true;
			}


			// plane for testing normal mapping
			{
				gfx_plane& r_plane_mesh = *plane_mesh;
				r_plane_mesh.set_dimensions(500, 500);
				r_plane_mesh.position = glm::vec3(0.f, -150.f, 0.f);
				r_plane_mesh.look_at(glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
				r_plane_mesh[MP_SHADER_NAME] = "sp_dns_o";
				r_plane_mesh["u_s2d_diffuse_map"] = "stone-floor";
				r_plane_mesh["u_s2d_normal_map"] = "stone-floor-normal";
				r_plane_mesh["u_s2d_specular_map"] = "stone-floor-specular";
				r_plane_mesh["u_v3_light_dir"] = u_v3_light_dir;
			}



			// sphere for testing materials
			{
				gfx_vpc_ring_sphere& rs_mesh = *s_mesh;
				rs_mesh.set_dimensions(100, 75);
				rs_mesh.position = glm::vec3(0.f, 0.f, -500.f);

				std::vector<shared_ptr<gfx_vxo> > mesh_list;
				mesh_list.push_back(s_mesh);
				obj_map["test_sphere"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, s_mesh->orientation));
			}


			shared_ptr<gfx_obj_vxo> obj_mesh;
			// ferrari model
			{
				obj_mesh = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
				gfx_obj_vxo& r_obj_mesh = *obj_mesh;
				r_obj_mesh.scaling = glm::vec3(150.f);
				r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
				r_obj_mesh = "car.obj";

				std::vector<shared_ptr<gfx_vxo> > mesh_list;
				mesh_list.push_back(obj_mesh);
				obj_map["ferrari_car"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}

			// car wheel rim
			{
				obj_mesh = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
				gfx_obj_vxo& r_obj_mesh = *obj_mesh;
				r_obj_mesh.scaling = glm::vec3(0.7f);
				r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
				r_obj_mesh.orientation *= glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
				//r_obj_mesh = "w1.obj";
				r_obj_mesh = "Agile_01_Rims_HD.obj";

				std::vector<shared_ptr<gfx_vxo> > mesh_list;
				mesh_list.push_back(obj_mesh);
				obj_map["car_wheel_rim"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}

			// car wheel tyre
			{
				obj_mesh = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
				gfx_obj_vxo& r_obj_mesh = *obj_mesh;
				r_obj_mesh.scaling = glm::vec3(0.7f);
				r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
				r_obj_mesh.orientation *= glm::angleAxis(glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
				//r_obj_mesh = "w1.obj";
				r_obj_mesh = "Agile_01_Tyres_HD.obj";

				std::vector<shared_ptr<gfx_vxo> > mesh_list;
				mesh_list.push_back(obj_mesh);
				obj_map["car_wheel_tyre"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}

			// car glass
			{
				obj_mesh = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
				gfx_obj_vxo& r_obj_mesh = *obj_mesh;
				r_obj_mesh.scaling = glm::vec3(0.5f);
				//r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
				r_obj_mesh.look_at(glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));
				r_obj_mesh = "mustang_garage.obj";

				std::vector<shared_ptr<gfx_vxo> > mesh_list;
				mesh_list.push_back(obj_mesh);
				obj_map["car_glass"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}


			// watchdogs car1
			{
				const int DIM = 8;
				const shared_ptr<gfx_material> mesh_materials[DIM] =
				{
					mat_car_body, mat_car_chassis, mat_car_headlights,
					mat_car_headlights_glass, mat_car_interior, mat_car_wheel_rims,
					mat_car_tyres, mat_car_windows,
				};
				const char* mesh_names[DIM] =
				{
					"Agile_01_Body_HD.obj", "Agile_01_Chassis_HD.obj", "Agile_01_Headlights_HD.obj",
					"Agile_01_HeadlightsGlass_HD.obj", "Agile_01_Interior_HD.obj", "Agile_01_Rims_HD.obj",
					"Agile_01_Tyres_HD.obj", "Agile_01_Windows_HD.obj",
				};
				shared_ptr<gfx_obj_vxo> obj_mesh_tab[DIM];
				std::vector<shared_ptr<gfx_vxo> > mesh_list;

				for (int k = 0; k < DIM; k++)
				{
					obj_mesh_tab[k] = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
					gfx_obj_vxo& r_obj_mesh = *obj_mesh_tab[k];

					r_obj_mesh.scaling = glm::vec3(0.7f);
					r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
					r_obj_mesh.set_material(mesh_materials[k]);
					r_obj_mesh = mesh_names[k];
					mesh_list.push_back(obj_mesh_tab[k]);
				}

				obj_map["watchdogs_car1"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}

			// mustang
			{
				const int DIM = 1;
				const shared_ptr<gfx_material> mesh_materials[DIM] =
				{
					mat_car_body_3,/* mat_car_chassis, mat_car_headlights_glass,
					mat_car_headlights, mat_car_headlights, mat_car_interior,
					mat_car_wheel_rims, mat_car_carbon_fiber, mat_car_tyres,
					mat_car_windows,*/
				};
				const char* mesh_names[DIM] =
				{
					"Mustang_Body_HD001.obj", /*"Mustang_Chassis_HD001.obj", "Mustang_Headlights_Glass_HD001.obj",
					"Mustang_Headlights_HD.obj", "Mustang_Headlights_HD001.obj", "Mustang_Interior_HD001.obj",
					"Mustang_Metal_HD001.obj", "Mustang_Plastic_HD001.obj", "Mustang_Tyres_HD001.obj",
					"Mustang_Windows_HD001.obj",*/
				};
				shared_ptr<gfx_obj_vxo> obj_mesh_tab[DIM];
				std::vector<shared_ptr<gfx_vxo> > mesh_list;

				for (int k = 0; k < DIM; k++)
				{
					obj_mesh_tab[k] = shared_ptr<gfx_obj_vxo>(new gfx_obj_vxo());
					gfx_obj_vxo& r_obj_mesh = *obj_mesh_tab[k];

					r_obj_mesh.scaling = glm::vec3(0.5f);
					r_obj_mesh.look_at(glm::vec3(0, -1, 0), glm::vec3(0, 0, 1));
					r_obj_mesh.set_material(mesh_materials[k]);
					r_obj_mesh = mesh_names[k];
					mesh_list.push_back(obj_mesh_tab[k]);
				}

				obj_map["mustang"] = shared_ptr<obj_tf>(new obj_tf(mesh_list, obj_mesh->orientation));
			}

			scene_idx_map[0] = "ferrari_car";
			scene_idx_map[1] = "ferrari_car";
			scene_idx_map[2] = "ferrari_car";
			scene_idx_map[3] = "ferrari_car";
			scene_idx_map[4] = "ferrari_car";
			scene_idx_map[5] = "ferrari_car";
			scene_idx_map[6] = "car_wheel_rim";
			scene_idx_map[7] = "car_wheel_tyre";
			scene_idx_map[8] = "car_glass";
			scene_idx_map[9] = "watchdogs_car1";
			scene_idx_map[10] = "mustang";

			car_count = scene_idx_map.size();
			set_car_idx(6, gfx_scene_inst);

			gfx_scene_inst->attach(persp_cam);
			//gfx_scene_inst->attach(s_mesh);
			//gfx_scene_inst->attach(plane_mesh);
			gfx_scene_inst->attach(skybox);

			persp_cam->position = glm::vec3(150.f, 150, 150);
			glm::vec3 look_at_dir = glm::normalize(-persp_cam->position());
			free_cam->look_at_dir = look_at_dir;
			free_cam->up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
			gfx_scene_inst->attach(ortho_cam);

			for(int k = 0; k < button_list.size(); k++)
			{
				button_list[k]->camera_id_list.clear();
				button_list[k]->camera_id_list.push_back(ortho_cam->camera_id());
				gfx_scene_inst->attach(button_list[k]);
			}
		}

		bool process_input(pfm_touch_event::touch_point& ipoint)
		{
			int width = get_unit()->get_width();
			int height = get_unit()->get_height();

			for(int k = 0; k < button_list.size(); k++)
			{
				shared_ptr<ux_select_button> btn = button_list[k];

				if(btn->is_hit(ipoint.x, ipoint.y))
				{
					switch(btn->button_id)
					{
					case 0:
						{
							current_car_idx =  (current_car_idx - 1 + car_count) % car_count;
							set_car_idx(current_car_idx, get_unit()->gfx_scene_inst);
							break;
						}

					case 1:
						{
							current_car_idx =  (current_car_idx + 1) % car_count;
							set_car_idx(current_car_idx, get_unit()->gfx_scene_inst);
							break;
						}
					}

					break;
				}
			}

			return true;
		}

		shared_ptr<gfx_box> skybox;
		shared_ptr<gfx_plane> plane_mesh;
		shared_ptr<gfx_vpc_ring_sphere> s_mesh;
		shared_ptr<obj_tf> obj_tf_inst;
		shared_ptr<gfx_camera> persp_cam;
		shared_ptr<gfx_camera> ortho_cam;
		glm::vec3 u_v3_light_dir;
		bool draw_axis;
		float t;
		shared_ptr<free_camera> free_cam;
		bool is_paused;
		int current_car_idx;
		int car_count;
		std::vector<shared_ptr<ux_select_button> > button_list;

		shared_ptr<gfx_material> mat_car_windows;
		shared_ptr<gfx_material> mat_car_tyres;
		shared_ptr<gfx_material> mat_car_carbon_fiber;
		shared_ptr<gfx_material> mat_car_wheel_rims;
		shared_ptr<gfx_material> mat_car_body;
		shared_ptr<gfx_material> mat_car_body_3;
		shared_ptr<gfx_material> mat_car_chassis;
		shared_ptr<gfx_material> mat_car_interior;
		shared_ptr<gfx_material> mat_car_headlights;
		shared_ptr<gfx_material> mat_car_headlights_glass;

		std::map<uint32, std::string> scene_idx_map;
		std::map<std::string, shared_ptr<obj_tf> > obj_map;
		weak_ptr<unit_test_normal_mapping> unit;
	};
}


using namespace ns_unit_test_normal_mapping;


unit_test_normal_mapping::unit_test_normal_mapping()
{
	set_name("test-normal-mapping");
}

shared_ptr<unit_test_normal_mapping> unit_test_normal_mapping::new_instance()
{
	return shared_ptr<unit_test_normal_mapping>(new unit_test_normal_mapping());
}

void unit_test_normal_mapping::init()
{
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl->add_receiver(get_smtp_instance());
}

bool merge_diffuse_specular(shared_ptr<pfm_file> idiff_file, shared_ptr<pfm_file> ispec_file)
{
	union color32
	{
		uint32 rgba;
		struct
		{
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
	};

	shared_ptr<RawImageData> diffmap = res_ld::inst()->load_image(idiff_file);
	color32* diffmap_ptr = (color32*)diffmap->data;
	shared_ptr<RawImageData> specularmap = res_ld::inst()->load_image(ispec_file);
	color32* specularmap_ptr = (color32*)specularmap->data;

	int width = diffmap->width;
	int height = diffmap->height;
	int size = width * height;

	if (specularmap->width != width || specularmap->height != height)
	{
		vprint("specularmap size != diffmap size [%s, %s]", ispec_file->get_file_name().c_str(), idiff_file->get_file_name().c_str());

		return false;
	}

	std::vector<uint32> rgba(size);

	for (int k = 0; k < size; k++)
	{
		uint32 specular = specularmap_ptr[k].r;
		uint32 normal = (diffmap_ptr[k].r << 16) | (diffmap_ptr[k].g << 8) | diffmap_ptr[k].b;

		rgba[k] = normal | ((specular << 24) & 0xff000000);
	}

	std::string new_filename = "new_diff_" + idiff_file->get_file_name();
	shared_ptr<pfm_file> f = pfm_file::get_inst(new_filename);
	res_ld::inst()->save_image(f, width, height, (uint8*)begin_ptr(rgba));

	return true;
}

bool merge_diffuse_specular(std::string idiff_fname, std::string ispec_fname)
{
	shared_ptr<pfm_file> idiff_file = pfm_file::get_inst(idiff_fname);
	shared_ptr<pfm_file> ispec_file = pfm_file::get_inst(ispec_fname);

	return merge_diffuse_specular(idiff_file, ispec_file);
}

void extract_alpha_channel()
{
	union color32
	{
		uint32 rgba;
		struct
		{
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
	};

	shared_ptr<RawImageData> img = res_ld::inst()->load_image("trail.png");
	color32* img_ptr = (color32*)img->data;
	int size = img->width * img->height;

	for (int k = 0; k < size; k++)
	{
		img_ptr[k].a = img_ptr[k].r;
		img_ptr[k].r = img_ptr[k].g = img_ptr[k].b = 255;
	}

	shared_ptr<pfm_file> f = pfm_file::get_inst("trail-2.png");
	res_ld::inst()->save_image(f, img->width, img->height, img->data);
}

void combine_lightmaps()
{
	union color32
	{
		uint32 rgba;
		struct
		{
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
	};

	shared_ptr<RawImageData> img_day = res_ld::inst()->load_image("Track25_Lightmap_Day2.png");
	shared_ptr<RawImageData> img_night = res_ld::inst()->load_image("Track25_Lightmap_Night2.png");
	shared_ptr<RawImageData> img_overcast = res_ld::inst()->load_image("Track25_Lightmap_Overcast2.png");
	color32* img_day_ptr = (color32*)img_day->data;
	color32* img_night_ptr = (color32*)img_night->data;
	color32* img_overcast_ptr = (color32*)img_overcast->data;
	int size = img_day->width * img_day->height;
	std::vector<uint32> rgba(size);
	color32* img_ptr = (color32*)&rgba[0];

	for (int k = 0; k < size; k++)
	{
		img_ptr[k].a = 0;
		img_ptr[k].r = img_overcast_ptr[k].a;
		img_ptr[k].g = img_night_ptr[k].a;
		img_ptr[k].b = img_day_ptr[k].a;
	}

	shared_ptr<pfm_file> f = pfm_file::get_inst("Track25_Lightmap2.png");
	res_ld::inst()->save_image(f, img_day->width, img_day->height, (uint8*)img_ptr);
}

void combine_trail()
{
	union color32
	{
		uint32 rgba;
		struct
		{
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
	};

	shared_ptr<RawImageData> img_alpha = res_ld::inst()->load_image("trail-alpha.png");
	shared_ptr<RawImageData> img_outline = res_ld::inst()->load_image("trail-outline.png");
	color32* img_alpha_ptr = (color32*)img_alpha->data;
	color32* img_outline_ptr = (color32*)img_outline->data;
	int size = img_alpha->width * img_alpha->height;
	std::vector<uint32> rgba(size);
	color32* img_ptr = (color32*)&rgba[0];

	for (int k = 0; k < size; k++)
	{
		img_ptr[k].a = img_alpha_ptr[k].a;
		img_ptr[k].b = img_outline_ptr[k].a;
		img_ptr[k].g = 0;
		img_ptr[k].r = 0;
	}

	shared_ptr<pfm_file> f = pfm_file::get_inst("trail-new.png");
	res_ld::inst()->save_image(f, img_alpha->width, img_alpha->height, (uint8*)img_ptr);
}

void unit_test_normal_mapping::load()
{
	//merge_diffuse_specular("Track_11_Flag_Night_C.png", "Track_11_Flag_S.png");
	//merge_diffuse_specular("Track_12_Building_03_Day_C.png", "Track_12_Building_03_S.png");
	//merge_diffuse_specular("Track_12_Building_03_Night_C.png", "Track_12_Building_03_S.png");
	//merge_diffuse_specular("Track_12_Building_03_Overcast_C.png", "Track_12_Building_03_S.png");
	//merge_diffuse_specular("Track_26_AD_day_d.png", "Track_26_AD_s.png");
	//merge_diffuse_specular("Track_26_AD_night_d.png", "Track_26_AD_s.png");
	//merge_diffuse_specular("Track_26_AD_overcast_d.png", "Track_26_AD_s.png");
	//merge_diffuse_specular("Track_26_Airship_day_d.png", "Track_26_Airship_s.png");
	//merge_diffuse_specular("Track_26_Airship_night_d.png", "Track_26_Airship_s.png");
	//merge_diffuse_specular("Track_26_Airship_overcast_d.png", "Track_26_Airship_s.png");
	//merge_diffuse_specular("Track_26_bgbuildings_day_d.png", "Track_26_bgbuildings_s.png");
	//merge_diffuse_specular("Track_26_bgbuildings_night_d.png", "Track_26_bgbuildings_s.png");
	//merge_diffuse_specular("Track_26_bgbuildings_overcast_d.png", "Track_26_bgbuildings_s.png");
	//merge_diffuse_specular("Track_26_bigstuff_day_d.png", "Track_26_bigstuff_s.png");
	//merge_diffuse_specular("Track_26_bigstuff_night_d.png", "Track_26_bigstuff_s.png");
	//merge_diffuse_specular("Track_26_bigstuff_overcast_d.png", "Track_26_bigstuff_s.png");
	//merge_diffuse_specular("Track_26_boat_day_d.png", "Track_26_boat_s.png");
	//merge_diffuse_specular("Track_26_boat_night_d.png", "Track_26_boat_s.png");
	//merge_diffuse_specular("Track_26_boat_overcast_d.png", "Track_26_boat_s.png");
	//merge_diffuse_specular("Track_26_boite_sev_c.png", "Track_26_boite_sev_s.png");
	//merge_diffuse_specular("Track_26_boite_sev_night_c.png", "Track_26_boite_sev_s.png");
	//merge_diffuse_specular("Track_26_crane_day_d.png", "Track_26_crane_s.png");
	//merge_diffuse_specular("Track_26_crane_night_d.png", "Track_26_crane_s.png");
	//merge_diffuse_specular("Track_26_crane_overcast_d.png", "Track_26_crane_s.png");
	//merge_diffuse_specular("Track_26_house_day_d.png", "Track_26_house_s.png");
	//merge_diffuse_specular("Track_26_house_night_d.png", "Track_26_house_s.png");
	//merge_diffuse_specular("Track_26_house_overcast_d.png", "Track_26_house_s.png");
	//merge_diffuse_specular("Track_26_smallstuffa_day_d.png", "Track_26_smallstuffa_s.png");
	//merge_diffuse_specular("Track_26_smallstuffa_night_d.png", "Track_26_smallstuffa_s.png");
	//merge_diffuse_specular("Track_26_smallstuffa_overcast_d.png", "Track_26_smallstuffa_s.png");
	//merge_diffuse_specular("Track_26_terrain_day_d.png", "Track_26_terrain_overcast_s.png");
	//merge_diffuse_specular("Track_26_terrain_night_d.png", "Track_26_terrain_overcast_s.png");
	//merge_diffuse_specular("Track_26_terrain_overcast_d.png", "Track_26_terrain_s.png");
	//merge_diffuse_specular("Track_06_Light_01_Day_C.png", "Track_06_Light_01_S.png");
	//merge_diffuse_specular("Track_06_Light_01_Overcast_C.png", "Track_06_Light_01_S.png");
	//extract_alpha_channel();
	//combine_lightmaps();
	//combine_trail();

	if (false)
	{
		shared_ptr<pfm_path> path = pfm_path::get_inst("", "png");
		shared_ptr<std::vector<shared_ptr<pfm_file> > > file_list = path->list_directory(true);
		std::vector<std::string> spec_file_list;
		std::vector<std::string> diff_file_list;
		auto it = file_list->begin();

		for (; it != file_list->end(); it++)
		{
			shared_ptr<pfm_file> file = *it;
			std::string fname = file->get_file_name();

			if (ends_with(fname, "_S.png"))
			{
				spec_file_list.push_back(fname);
			}
		}

		struct pred
		{
			bool operator()(const std::string& a, const std::string& b) const
			{
				return a.length() > b.length();
			}
		};

		std::sort(spec_file_list.begin(), spec_file_list.end(), pred());

		auto its = spec_file_list.begin();

		for (; its != spec_file_list.end(); its++)
		{
			std::string fname = *its;
			auto it2 = file_list->begin();
			std::string root = fname.substr(0, fname.length() - 5);
			trx(fname);

			for (; it2 != file_list->end(); it2++)
			{
				shared_ptr<pfm_file> diff_file = *it2;
				std::string diff_fname = diff_file->get_file_name();

				if (starts_with(diff_fname, root) && !ends_with(diff_fname, "_S.png") && (std::find(diff_file_list.begin(), diff_file_list.end(), diff_fname) == diff_file_list.end()))
				{
					shared_ptr<pfm_file> specular_file = pfm_file::get_inst(fname);
					//int idx = fname.length() - 6;
					//std::string diff_fname_last = diff_fname.substr(idx, diff_fname.length() - idx);
					diff_file_list.push_back(diff_fname);
					//merge_diffuse_specular(diff_file, specular_file);
					trx("\t" + diff_fname);
				}
			}
		}
	}

	//int width = 2048;
	//int height = 1024;
	//int size = width * height;
	//std::vector<uint32> rgba(size);
	//RNG rng;

	//for (int k = 0; k < size; k++)
	//{
	//	int a = 255;
	//	int r = rng.nextInt(256);
	//	int g = rng.nextInt(256);
	//	int b = 128 + rng.nextInt(128);

	//	rgba[k] = ((a << 24) & 0xff000000) | ((r << 16) & 0x00ff0000) | ((g << 8) & 0x0000ff00) | b;
	//	//rgba[k] = 0x707abcde;
	//}

	//shared_ptr<pfm_file> f = pfm_file::get_inst("sparkle-normal-map.png");
	//res_ld::inst()->save_image(f, width, height, (uint8*)begin_ptr(rgba));
	
	//union color32
	//{
	//	uint32 rgba;
	//	struct
	//	{
	//		uint8 r;
	//		uint8 g;
	//		uint8 b;
	//		uint8 a;
	//	};
	//};

	//shared_ptr<RawImageData> diffmap = res_ld::inst()->load_image("earth_diffmap.png");
	//color32* diffmap_ptr = (color32*)diffmap->data;
	//shared_ptr<RawImageData> specularmap = res_ld::inst()->load_image("earth_flat_map_spec.png");
	//color32* specularmap_ptr = (color32*)specularmap->data;

	//for (int k = 0; k < size; k++)
	//{
	//	uint32 specular = specularmap_ptr[k].r;
	//	uint32 normal = (diffmap_ptr[k].r << 16) | (diffmap_ptr[k].g << 8) | diffmap_ptr[k].b;

	//	rgba[k] = normal | ((specular << 24) & 0xff000000);
	//}

	//shared_ptr<pfm_file> f = pfm_file::get_inst("earth_normal_spec_map.png");
	//res_ld::inst()->save_image(f, width, height, (uint8*)begin_ptr(rgba));

	p = shared_ptr<impl>(new impl(static_pointer_cast<unit_test_normal_mapping>(get_smtp_instance())));
	p->init(gfx_scene_inst);

	gfx_util::check_gfx_error();
}

bool unit_test_normal_mapping::update()
{
	float& t = p->t;

	p->skybox->position = p->persp_cam->position;
	p->s_mesh->orientation = glm::quat(glm::vec3(0, t, 0));

	std::vector<shared_ptr<gfx_vxo> >& mesh_list = p->obj_tf_inst->mesh_list;

	for(int k = 0; k < mesh_list.size(); k++)
	{
		mesh_list[k]->orientation = glm::quat(glm::vec3(0, -t, 0)) * p->obj_tf_inst->init_orientation;
	}


	gfx_material& mat = *p->mat_car_wheel_rims;
	float gloss = glm::mod(t*4, 9.0f);
	mat["u_v1_gloss_level"] = gloss;

	if (!p->is_paused)
	{
		t += 0.01f;
	}

	p->free_cam->update();

	if(p->draw_axis)
	{
		p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(5000, 0, 0), glm::vec4(1, 0, 0.f, 1.f), 1.f);
		p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 5000, 0), glm::vec4(0, 1, 0.f, 1.f), 1.f);
		p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 0, 5000), glm::vec4(0, 0, 1.f, 1.f), 1.f);
	}

	gfx_util::check_gfx_error();

	return unit::update();
}

void unit_test_normal_mapping::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		switch(ts->get_type())
		{
		case touch_sym_evt::TS_RELEASED:
			{
				p->process_input(ts->crt_state.te->points[0]);
				ts->process();

				break;
			}
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
			case KEY_SPACE:
				{
					p->is_paused = !p->is_paused;
					break;
				}

			case KEY_LEFT:
				{
					p->current_car_idx =  (p->current_car_idx - 1 + p->car_count) % p->car_count;
					p->set_car_idx(p->current_car_idx, gfx_scene_inst);
					break;
				}

			case KEY_RIGHT:
				{
					p->current_car_idx =  (p->current_car_idx + 1) % p->car_count;
					p->set_car_idx(p->current_car_idx, gfx_scene_inst);
					break;
				}

			case KEY_R:
				{
					p->set_car_idx(p->current_car_idx, gfx_scene_inst);
					break;
				}

			case KEY_Y:
				{
					p->draw_axis = !p->draw_axis;
					break;
				}

			case KEY_W:
				{
					std::vector<shared_ptr<gfx_vxo> >& mesh_list = p->obj_tf_inst->mesh_list;

					for(int k = 0; k < mesh_list.size(); k++)
					{
						gfx_vxo& mesh = *mesh_list[k];
						mesh[MP_WIREFRAME_MODE] = !mesh[MP_WIREFRAME_MODE].get_bool_value();
					}
					break;
				}

			default:
				isAction = false;
			}

			if(isAction)
			{
				ke->process();
			}
		}
	}

	p->free_cam->update_input(idp);
}

#endif
