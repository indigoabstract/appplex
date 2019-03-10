#pragma once

#include "pfm.hxx"
#include "gfx.hxx"
#include "gfx-vxo.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo-ext.hxx"
#include "jcv/vrn-diag-data.hxx"
#include "jcv/vrn-diag.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>


static const std::string vkb_line_sh = "mws-vkb-line";
static const std::string vkb_point_sh = "mws-vkb-point";
static const std::string vkb_triangle_sh = "mws-vkb-triangle";


struct mws_vrn_obj_types
{
   static const uint32 kernel_points = (1 << 0);
   static const uint32 nexus_points = (1 << 1);
   static const uint32 nexus_pairs = (1 << 2);
   static const uint32 cells = (1 << 3);
   static const uint32 delaunay_diag = (1 << 4);
   static const uint32 convex_hull = (1 << 5);
   static const uint32 obj_count = 6;
};


enum class mws_vrn_diag_data_change_type
{
   diag_no_change,
   diag_rebuild_geometry,
   diag_rebuild_all,
};


class mws_vkb_gen
{
public:
   static mws_sp<mws_vkb_gen> nwi(mws_sp<mws_vrn_data> i_diag_data)
   {
      mws_sp<mws_vkb_gen> inst(new mws_vkb_gen());

      inst->init(i_diag_data);

      return inst;
   }

   void update_diag();

   void clear_points()
   {
      vx.clear();
      vy.clear();
      diag_data_state = mws_vrn_diag_data_change_type::diag_no_change;
   }

   void random_points()
   {
      RNG rng;
      //rng.setSeed(12345);
      int max_extra_groups = 2;
      int max_extra_group_points = 3;
      int max_random_points = 5;
      int group_count = 2 + rng.nextInt(max_extra_groups);
      int random_point_count = 5 + rng.nextInt(max_random_points);
      int hw = diag_data->info.diag_width;
      int hh = diag_data->info.diag_height;
      int x_inf = 50;
      int x_sup = hw - 50;
      int y_inf = 50;
      int y_sup = hh - 50;

      clear_points();

      for (int k = 0; k < group_count; k++)
      {
         int group_point_count = 3 + rng.nextInt(max_extra_group_points);

         for (int l = 0; l < group_point_count; l++)
         {
            int x = rng.range(x_inf, x_sup);
            int y = rng.range(y_inf, y_sup);

            vx.push_back((float)x);
            vy.push_back((float)y);
         }
      }

      for (int k = 0; k < random_point_count; k++)
      {
         int x = rng.range(x_inf, x_sup);
         int y = rng.range(y_inf, y_sup);

         vx.push_back((float)x);
         vy.push_back((float)y);
      }

      voronoi_diag_impl->init_data(diag_data, vx, vy);
   }

   mws_sp<mws_vrn_diag> voronoi_diag_impl;
   mws_sp<mws_vrn_data> diag_data;
   mws_vrn_diag_data_change_type diag_data_state;
   std::vector<float> vx, vy;

private:
   mws_vkb_gen()
   {
   }

   void init(mws_sp<mws_vrn_data> i_diag_data)
   {
      diag_data = i_diag_data;
      voronoi_diag_impl = mws_vrn_diag::nwi();
      diag_data_state = mws_vrn_diag_data_change_type::diag_no_change;

      //random_points();
   }
};


class mws_vkb_geom : public gfx_node
{
public:
   struct vx_seg_data_4f
   {
      gfx_float x, y, z, w;

      void set(const glm::vec4& iv)
      {
         x = iv.x; y = iv.y; z = iv.z; w = iv.w;
      }
   };

   struct vx_fmt_3f_4f_4b_2f_1i
   {
      glm::vec3 pos;
      vx_seg_data_4f nrm;
      vx_color_coord_4b clr;
      glm::vec2 tex;
      gfx_uint id;
   };

   static mws_sp<mws_vkb_geom> nwi(mws_sp<mws_vrn_data> i_diag_data, mws_sp<gfx_camera> i_cam)
   {
      mws_sp<mws_vkb_geom> inst(new mws_vkb_geom(i_diag_data));

      inst->init(i_cam);

      return inst;
   }

   void set_line_geometry(mws_vrn_nexus_pair_vect& i_line_points, mws_sp<gfx_vxo> i_mesh)
   {
      uint32 list_size = i_line_points.size();
      i_mesh->set_size(list_size * 4, list_size * 6);
      std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
      std::vector<vx_fmt_3f_4f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4f_4b_2f_1i>*) & i_mesh->get_vx_buffer());

      glm::vec3 pos;
      vx_seg_data_4f nrm;
      vx_color_coord_4b color0;
      vx_color_coord_4b color1;
      gfx_uint id = 0;

      struct line_setup
      {
         static void setup(mws_sp<mws_vrn_data> diag_data, mws_vrn_nexus_pair& inp, mws_vrn_nexus_pt& ip, float& isize, vx_color_coord_4b& ic)
         {
            mws_vrn_nexus_pt* p0 = diag_data->get_nexus_point_by_id(inp.nexus0_id);
            mws_vrn_nexus_pt* p1 = diag_data->get_nexus_point_by_id(inp.nexus1_id);
            ic.r = 0; ic.g = 0; ic.b = 255; ic.a = 255;
            isize = 1.f;
         }
      };

      glm::vec2 tex0 = { -0.5f, -0.5f };
      glm::vec2 tex1 = { +0.5f, -0.5f };
      glm::vec2 tex2 = { -0.5f, +0.5f };
      glm::vec2 tex3 = { +0.5f, +0.5f };

      for (uint32 k = 0; k < list_size; k++)
      {
         mws_vrn_nexus_pair& p = i_line_points.vect[k];
         mws_vrn_nexus_pt* np0 = diag_data->get_nexus_point_by_id(p.nexus0_id);
         mws_vrn_nexus_pt* np1 = diag_data->get_nexus_point_by_id(p.nexus1_id);
         glm::vec3& p0 = np0->position;
         glm::vec3& p1 = np1->position;
         glm::vec3 seg_dir = glm::normalize(p1 - p0);
         float line_thickness = 1.f;
         float line_thickness2 = 1.f;

         line_setup::setup(diag_data, p, *np0, line_thickness, color0);
         line_setup::setup(diag_data, p, *np1, line_thickness2, color1);

         pos = p0;
         nrm.set(glm::vec4(seg_dir, -0.5f * line_thickness));
         vx_fmt_3f_4f_4b_2f_1i vx0 = { pos, nrm, color0, tex0, id };
         ks_vertices_data[4 * k] = vx0;

         pos = p0;
         nrm.set(glm::vec4(seg_dir, +0.5f * line_thickness));
         vx_fmt_3f_4f_4b_2f_1i vx1 = { pos, nrm, color0, tex1, id };
         ks_vertices_data[4 * k + 1] = vx1;

         pos = p1;
         nrm.set(glm::vec4(seg_dir, -0.5f * line_thickness2));
         vx_fmt_3f_4f_4b_2f_1i vx2 = { pos, nrm, color1, tex2, id };
         ks_vertices_data[4 * k + 2] = vx2;

         pos = p1;
         nrm.set(glm::vec4(seg_dir, +0.5f * line_thickness2));
         vx_fmt_3f_4f_4b_2f_1i vx3 = { pos, nrm, color1, tex3, id };
         ks_vertices_data[4 * k + 3] = vx3;
      }

      uint32 ind_size = ks_vertices_data.size() / 4;

      for (uint32 k = 0; k < ind_size; k++)
      {
         uint32 i_off = 4 * k;
         ks_indices_data[6 * k + 0] = i_off + 1;
         ks_indices_data[6 * k + 1] = i_off + 2;
         ks_indices_data[6 * k + 2] = i_off + 0;
         ks_indices_data[6 * k + 3] = i_off + 2;
         ks_indices_data[6 * k + 4] = i_off + 1;
         ks_indices_data[6 * k + 5] = i_off + 3;
      }

      i_mesh->update_data();
   }

   void set_points_geometry(mws_vrn_pos_vect & i_point_list, mws_sp<gfx_vxo> i_mesh)
   {
      struct vx_fmt_p3f_t2f_1i
      {
         glm::vec3 pos;
         glm::vec2 tex;
         gfx_uint id;
      };

      glm::vec3 pos;
      glm::vec2 tex;
      uint32 list_size = i_point_list.size();
      i_mesh->set_size(list_size * 4, list_size * 6);
      std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
      std::vector<vx_fmt_p3f_t2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_p3f_t2f_1i>*) & i_mesh->get_vx_buffer());

      for (uint32 k = 0; k < list_size; k++)
      {
         const glm::vec3& p = i_point_list.get_position_at(k);
         gfx_uint id = i_point_list.get_id_at(k);

         pos = p;
         tex = glm::vec2(-0.5f, -0.5f);
         vx_fmt_p3f_t2f_1i vx0 = { pos, tex, id };
         ks_vertices_data[4 * k + 0] = vx0;

         pos = p;
         tex = glm::vec2(-0.5f, +0.5f);
         vx_fmt_p3f_t2f_1i vx1 = { pos, tex, id };
         ks_vertices_data[4 * k + 1] = vx1;

         pos = p;
         tex = glm::vec2(+0.5f, -0.5f);
         vx_fmt_p3f_t2f_1i vx2 = { pos, tex, id };
         ks_vertices_data[4 * k + 2] = vx2;

         pos = p;
         tex = glm::vec2(+0.5f, +0.5f);
         vx_fmt_p3f_t2f_1i vx3 = { pos, tex, id };
         ks_vertices_data[4 * k + 3] = vx3;
      }

      uint32 ind_size = ks_vertices_data.size() / 4;

      for (uint32 k = 0; k < ind_size; k++)
      {
         uint32 i_off = 4 * k;
         ks_indices_data[6 * k + 0] = i_off + 1;
         ks_indices_data[6 * k + 1] = i_off + 2;
         ks_indices_data[6 * k + 2] = i_off + 0;
         ks_indices_data[6 * k + 3] = i_off + 2;
         ks_indices_data[6 * k + 4] = i_off + 1;
         ks_indices_data[6 * k + 5] = i_off + 3;
      }

      i_mesh->update_data();
   }

   void set_triangle_geometry(mws_vrn_pos_vect & i_point_list, const std::vector<uint32> & i_point_count_list, mws_sp<gfx_vxo> i_mesh)
   {
      struct vx_fmt_3f_4b_2f_1i
      {
         glm::vec3 pos;
         vx_color_coord_4b clr;
         glm::vec2 tex;
         gfx_uint id;
      };

      glm::vec3 pos;
      vx_color_coord_4b clr;
      glm::vec2 tex;
      uint32 list_size = i_point_count_list.size();
      uint32 vx_count = 0;
      uint32 ix_count = 0;

      for (uint32 k = 0; k < list_size; k++)
      {
         uint32 l2_size = i_point_count_list[k];
         uint32 ind_size = l2_size - 1;

         vx_count += l2_size;
         ix_count += ind_size * 3;
      }

      i_mesh->set_size(vx_count, ix_count);
      std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
      //std::vector<vx_fmt_3f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4b_2f_1i>*)&i_mesh->get_vx_buffer());
      vx_fmt_3f_4b_2f_1i* ks_vertices_data = (vx_fmt_3f_4b_2f_1i*)begin_ptr(i_mesh->get_vx_buffer());
      int idx = 0;
      int vx_idx = 0;
      int ix_idx = 0;

      for (uint32 k = 0; k < list_size; k++)
      {
         gfx_uint id = i_point_list.get_id_at(k);
         uint32 l2_size = i_point_count_list[k];
         int i_idx = idx;

         clr.r = 0;
         clr.g = 0;
         clr.b = 0;
         clr.a = 255;

         for (uint32 l = 0; l < l2_size; l++)
         {
            const glm::vec3& p = i_point_list.get_position_at(idx);

            pos = p;
            tex = glm::vec2(-0.5f, -0.5f);
            vx_fmt_3f_4b_2f_1i vx0 = { pos, clr, tex, id };
            ks_vertices_data[vx_idx++] = vx0;

            idx++;
         }

         uint32 ind_size = l2_size - 1;

         for (uint32 i = 0; i < ind_size; i++)
         {
            int i1 = i_idx + i + 1;
            int i2 = i + 2;

            if (i2% l2_size == 0)
            {
               i2 = 1;
            }

            i2 += i_idx;
            ks_indices_data[ix_idx++] = i_idx;
            ks_indices_data[ix_idx++] = i1;
            ks_indices_data[ix_idx++] = i2;
         }
      }

      i_mesh->update_data();
   }

   mws_sp<mws_vrn_data> diag_data;
   mws_sp<gfx_plane> quad_mesh;
   mws_sp<gfx_vxo> voronoi_kernels_mesh;
   mws_sp<gfx_vxo> voronoi_nexus_mesh;
   mws_sp<gfx_vxo> nexus_pairs_mesh;
   mws_sp<gfx_vxo> voronoi_cell_faces_mesh;
   mws_sp<gfx_vxo> delaunay_diag_mesh;
   mws_sp<gfx_vxo> convex_hull_mesh;

private:
   mws_vkb_geom(mws_sp<mws_vrn_data> i_diag_data) : gfx_node(gfx::i())
   {
      diag_data = i_diag_data;
   }

   void init(mws_sp<gfx_camera> i_cam)
   {
      float alpha_val = 0.8f;
      float line_thickness = 6.f;

      // cache the shaders, to prevent recompiling
      init_shaders();

      // kernel points
      {
         voronoi_kernels_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
         voronoi_kernels_mesh->name = "voronoi_kernels_mesh";
         voronoi_kernels_mesh->visible = diag_data->info.kernel_points_visible;
         (*voronoi_kernels_mesh)[MP_SHADER_NAME] = vkb_point_sh;
         (*voronoi_kernels_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
         (*voronoi_kernels_mesh)[MP_CULL_BACK] = false;
         (*voronoi_kernels_mesh)["u_v1_point_size"] = 2.f;
         (*voronoi_kernels_mesh)["u_v4_color"] = glm::vec4(0.f, 1.f, 1.f, alpha_val);
         (*voronoi_kernels_mesh)[MP_BLENDING] = MV_ALPHA;
         voronoi_kernels_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // nexus points
      {
         voronoi_nexus_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
         voronoi_nexus_mesh->name = "voronoi_nexus_mesh";
         voronoi_nexus_mesh->visible = diag_data->info.nexus_points_visible;
         (*voronoi_nexus_mesh)[MP_SHADER_NAME] = vkb_point_sh;
         (*voronoi_nexus_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
         (*voronoi_nexus_mesh)[MP_CULL_BACK] = false;
         (*voronoi_nexus_mesh)["u_v1_point_size"] = 2.5f;
         (*voronoi_nexus_mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, alpha_val);
         (*voronoi_nexus_mesh)[MP_BLENDING] = MV_ALPHA;
         voronoi_nexus_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // nexus pairs
      {
         nexus_pairs_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
         nexus_pairs_mesh->name = "nexus_pairs_mesh";
         nexus_pairs_mesh->visible = diag_data->info.nexus_pairs_visible;
         (*nexus_pairs_mesh)[MP_SHADER_NAME] = vkb_line_sh;
         (*nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness;
         (*nexus_pairs_mesh)["u_v1_depth_offset"] = 0.001f;
         (*nexus_pairs_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 1.f, alpha_val);
         (*nexus_pairs_mesh)[MP_CULL_FRONT] = false;
         (*nexus_pairs_mesh)[MP_CULL_BACK] = false;
         (*nexus_pairs_mesh)[MP_BLENDING] = MV_ALPHA;
         //nexus_pairs_mesh->render_method = GLPT_POINTS;
         nexus_pairs_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // cell faces
      {
         voronoi_cell_faces_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
         voronoi_cell_faces_mesh->name = "voronoi_cell_faces_mesh";
         voronoi_cell_faces_mesh->visible = diag_data->info.cell_triangles_visible;
         (*voronoi_cell_faces_mesh)[MP_SHADER_NAME] = vkb_triangle_sh;
         (*voronoi_cell_faces_mesh)["u_v1_point_size"] = 6.f;
         (*voronoi_cell_faces_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 0.f, alpha_val);
         (*voronoi_cell_faces_mesh)[MP_CULL_FRONT] = false;
         (*voronoi_cell_faces_mesh)[MP_CULL_BACK] = false;
         (*voronoi_cell_faces_mesh)[MP_BLENDING] = MV_ALPHA;
         voronoi_cell_faces_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // delaunay
      {
         delaunay_diag_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
         delaunay_diag_mesh->name = "delaunay_diag_mesh";
         delaunay_diag_mesh->visible = diag_data->info.delaunay_diag_visible;
         (*delaunay_diag_mesh)[MP_SHADER_NAME] = vkb_line_sh;
         (*delaunay_diag_mesh)["u_v1_line_thickness"] = line_thickness;
         delaunay_diag_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // convex hull
      {
         convex_hull_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
         convex_hull_mesh->name = "convex_hull_mesh";
         convex_hull_mesh->visible = diag_data->info.convex_hull_visible;
         (*convex_hull_mesh)[MP_SHADER_NAME] = vkb_line_sh;
         (*convex_hull_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
         (*convex_hull_mesh)["u_v1_line_thickness"] = line_thickness;
         convex_hull_mesh->camera_id_list = { i_cam->camera_id() };
      }
      // attach them into the scene
      {
         auto node = get_mws_sp();
         node->name = "vrn-geom";
         node->attach(delaunay_diag_mesh);
         node->attach(nexus_pairs_mesh);
         node->attach(convex_hull_mesh);
         node->attach(voronoi_kernels_mesh);
         node->attach(voronoi_nexus_mesh);
         node->attach(voronoi_cell_faces_mesh);
      }
      {
         mws_sp<gfx_plane> quad_mesh(new gfx_plane());
         quad_mesh->name = "quad_mesh";
         gfx_plane& r_quad_mesh = *quad_mesh;
         //r_quad_mesh.camera_id_list.clear();
         //r_quad_mesh.camera_id_list.push_back(persp_cam->camera_id());
         r_quad_mesh[MP_SHADER_NAME] = "c-o-shader";
         r_quad_mesh["u_v4_color"] = glm::vec4(0, 0.05f, 0, 1);
         r_quad_mesh[MP_CULL_BACK] = false;
         r_quad_mesh.set_dimensions((float)diag_data->info.diag_width, (float)diag_data->info.diag_height);
         r_quad_mesh.position = glm::vec3(0, 0, -0.5f);
         quad_mesh->camera_id_list = { i_cam->camera_id() };
         //node->attach(quad_mesh);
      }
   }

   void init_shaders()
   {
      // line shader
      vkb_line_shader = gfx::i()->shader.get_program_by_name(vkb_line_sh);
      if (!vkb_line_shader)
      {
         auto vsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core


               layout(location = 0) in vec3 a_v3_position;
               layout(location = 1) in vec4 a_v4_seg_data;
               layout(location = 2) in vec4 a_iv4_color;
               layout(location = 3) in vec2 a_v2_tex_coord;

               uniform mat4 u_m4_model_view;
               uniform mat4 u_m4_model_view_proj;
               uniform mat4 u_m4_projection;

               uniform float u_v1_line_thickness;
               uniform float u_v1_depth_offset;

               flat out vec4 v_v4_color;
               out vec2 v_v2_tex_coord;

               void main()
               {
	               vec3 v3_cam_dir = normalize(u_m4_model_view[3].xyz);
	               vec3 v3_position = (u_m4_model_view * vec4(a_v3_position, 1.0)).xyz;
	               vec3 v3_direction = (u_m4_model_view * vec4(a_v4_seg_data.xyz, 0.0)).xyz;
	               vec3 vect = cross(v3_cam_dir, v3_direction);
	               float vsize = a_v4_seg_data.w * u_v1_line_thickness;
	
	               v3_position = v3_position + vect * vsize;
	               v_v4_color = a_iv4_color;
	               v_v2_tex_coord = a_v2_tex_coord;
	
	               vec4 v4_position = u_m4_projection * vec4(v3_position, 1.0);
	
	               //v4_position.z -= u_v1_depth_offset;
	               gl_Position = v4_position;
               }
               )"
         ));

         auto fsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core

               #ifdef GL_ES
	               precision highp float;
               #endif

               layout(location = 0) out vec4 v4_frag_color;

               uniform vec4 u_v4_color;

               flat in vec4 v_v4_color;
               in vec2 v_v2_tex_coord;

               void main()
               {
	               v4_frag_color = u_v4_color;
               }
               )"
         ));

         vkb_line_shader = gfx::i()->shader.new_program_from_src(vkb_line_sh, vsh, fsh);
      }

      // point shader
      vkb_point_shader = gfx::i()->shader.get_program_by_name(vkb_point_sh);
      if (!vkb_point_shader)
      {
         auto vsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core


               layout(location = 0) in vec3 a_v3_position;
               layout(location = 1) in vec2 a_v2_tex_coord;

               uniform mat4 u_m4_model_view;
               uniform mat4 u_m4_model_view_proj;
               uniform mat4 u_m4_projection;

               uniform float u_v1_point_size;

               out vec2 v_v2_tex_coord;

               void main()
               {
	               float vsize = u_v1_point_size;
	               vec3 v3_position = (u_m4_model_view * vec4(a_v3_position, 1.0)).xyz;
	
	               v_v2_tex_coord = a_v2_tex_coord;
	               v3_position = v3_position + vec3(1., 0., 0.) * vsize * a_v2_tex_coord.x;
	               v3_position = v3_position + vec3(0., 1., 0.) * vsize * a_v2_tex_coord.y;
	
	               gl_Position = u_m4_projection * vec4(v3_position, 1.0);
               }
               )"
         ));

         auto fsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core

               #ifdef GL_ES
	               precision highp float;	
               #endif

               layout(location = 0) out vec4 v4_frag_color;

               uniform vec4 u_v4_color;

               in vec2 v_v2_tex_coord;

               void main()
               {
	               v4_frag_color = u_v4_color;
               }
               )"
         ));

         vkb_point_shader = gfx::i()->shader.new_program_from_src(vkb_point_sh, vsh, fsh);
      }

      // triangle shader
      vkb_triangle_shader = gfx::i()->shader.get_program_by_name(vkb_triangle_sh);
      if (!vkb_triangle_shader)
      {
         auto vsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core

               layout (location = 0) in vec3 a_v3_position;
               layout (location = 1) in vec4 a_iv4_color;
               layout (location = 2) in vec2 a_v2_tex_coord;

               uniform mat4 u_m4_model;
               uniform mat4 u_m4_model_view_proj;
               uniform mat4 u_m4_projection;
               uniform mat4 u_m4_view_inv;

               out vec2 v_v2_tex_coord;
               out vec4 v_v4_color;
               out vec3 v_v3_pos_ms;
               out vec3 v_v3_pos_ws;
               out vec3 v_v3_cam_dir_ws;

               void main()
               {
	               v_v2_tex_coord = a_v2_tex_coord;
	               v_v4_color = a_iv4_color;
	               v_v3_pos_ms = a_v3_position;
	               v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	               v_v3_cam_dir_ws = normalize(v_v3_pos_ws - u_m4_view_inv[3].xyz);
	
	               gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
               }
               )"
         ));

         auto fsh = mws_sp<std::string>(new std::string(
            R"(
               //@es #version 300 es
               //@dt #version 330 core

               #ifdef GL_ES
                  precision highp float;
               #endif

               layout(location = 0) out vec4 v4_frag_color;

               uniform vec4 u_v4_color;

               void main()
               {
	               v4_frag_color = u_v4_color;
               }
               )"
         ));

         vkb_triangle_shader = gfx::i()->shader.new_program_from_src(vkb_triangle_sh, vsh, fsh);
      }
   }

   mws_sp<gfx_shader> vkb_line_shader;
   mws_sp<gfx_shader> vkb_point_shader;
   mws_sp<gfx_shader> vkb_triangle_shader;
};


class mws_vkb_main
{
public:
   static mws_sp<mws_vkb_main> nwi(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      mws_sp<mws_vkb_main> inst(new mws_vkb_main());
      inst->setup(i_diag_width, i_diag_height, i_cam);
      return inst;
   }

   void init()
   {
      mws_assert(!vgen);
      vgen = mws_vkb_gen::nwi(diag_data);
      vgeom = mws_vkb_geom::nwi(diag_data, cam.lock());
   }

   void toggle_voronoi_object(uint32 i_obj_type_mask)
   {
      bool visibility_changed = false;
      mws_vrn_data::settings& s = diag_data->info;

      for (uint32 k = 0; k < (uint32)mws_vrn_obj_types::obj_count; k++)
      {
         uint32 idx = (1 << k);
         uint32 obj_type = uint32(idx & i_obj_type_mask);
         mws_sp<gfx_vxo> vxo;
         bool is_visible = false;
         visibility_changed = true;

         switch (obj_type)
         {
         case mws_vrn_obj_types::kernel_points:
            is_visible = s.kernel_points_visible = !s.kernel_points_visible;
            vxo = (vgeom) ? vgeom->voronoi_kernels_mesh : nullptr;
            break;

         case mws_vrn_obj_types::nexus_points:
            is_visible = s.nexus_points_visible = !s.nexus_points_visible;
            vxo = (vgeom) ? vgeom->voronoi_nexus_mesh : nullptr;
            break;

         case mws_vrn_obj_types::nexus_pairs:
            is_visible = s.nexus_pairs_visible = !s.nexus_pairs_visible;
            vxo = (vgeom) ? vgeom->nexus_pairs_mesh : nullptr;
            break;

         case mws_vrn_obj_types::cells:
            is_visible = s.cell_triangles_visible = !s.cell_triangles_visible;
            vxo = (vgeom) ? vgeom->voronoi_cell_faces_mesh : nullptr;
            break;

         case mws_vrn_obj_types::convex_hull:
            is_visible = s.convex_hull_visible = !s.convex_hull_visible;
            vxo = (vgeom) ? vgeom->convex_hull_mesh : nullptr;
            break;

         case mws_vrn_obj_types::delaunay_diag:
            is_visible = s.delaunay_diag_visible = !s.delaunay_diag_visible;
            vxo = (vgeom) ? vgeom->delaunay_diag_mesh : nullptr;
            break;
         }

         if (vxo)
         {
            vxo->visible = is_visible;
         }
      }

      if (visibility_changed)
      {
         if (vgen)
         {
            vgen->diag_data_state = mws_vrn_diag_data_change_type::diag_rebuild_geometry;
         }
      }
   }

   void update_diag()
   {
      vgen->update_diag();
      update_geometry();
   }

   void update_geometry()
   {
      if (diag_data->info.kernel_points_visible)
      {
         vgeom->set_points_geometry(diag_data->geom.kernel_points, vgeom->voronoi_kernels_mesh);
      }

      if (diag_data->info.nexus_points_visible)
      {
         vgeom->set_points_geometry(diag_data->geom.nexus_points, vgeom->voronoi_nexus_mesh);
      }


      if (diag_data->info.delaunay_diag_visible)
      {
         //vgeom->set_line_geometry(diag_data->geom.delaunay_diag_points, vgeom->delaunay_diag_mesh);
      }

      if (diag_data->info.nexus_pairs_visible)
      {
         vgeom->set_line_geometry(diag_data->geom.nexus_pairs, vgeom->nexus_pairs_mesh);
      }

      if (diag_data->info.convex_hull_visible)
      {
         //vgeom->set_line_geometry(diag_data->geom.convex_hull_points, vgeom->convex_hull_mesh);
      }

      if (diag_data->info.cell_triangles_visible)
      {
         vgeom->set_triangle_geometry(diag_data->geom.cell_points_ids, diag_data->geom.cell_point_count, vgeom->voronoi_cell_faces_mesh);
      }
   }

   void resize(uint32 i_diag_width, uint32 i_diag_height)
   {
      diag_data->info.diag_width = i_diag_width;
      diag_data->info.diag_height = i_diag_height;
   }

   const mws_sp<mws_vrn_data> get_diag_data() const { return diag_data; }

   void update_nexus_pairs_geometry()
   {
      if (diag_data->info.nexus_pairs_visible)
      {
         //vgen->set_altitude(diag_data->geom.nexus_pairs);
         vgeom->set_line_geometry(diag_data->geom.nexus_pairs, vgeom->nexus_pairs_mesh);
      }
   }

   void set_kernel_points(std::vector<glm::vec2> i_kernel_points);
   mws_vrn_diag::idx_dist get_kernel_idx_at(float i_x, float i_y) const;
   void move_kernel_to(uint32 i_idx, float i_x, float i_y);
   void insert_kernel_at(float i_x, float i_y);
   void remove_kernel(uint32 i_idx);

   mws_sp<mws_vrn_data> diag_data;
   mws_sp<mws_vkb_gen> vgen;
   mws_sp<mws_vkb_geom> vgeom;

private:
   mws_vkb_main() {}

   void setup(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      diag_data = mws_sp<mws_vrn_data>(new mws_vrn_data());
      diag_data->geom.cell_points_ids.vdata = diag_data;
      diag_data->info.diag_width = i_diag_width;
      diag_data->info.diag_height = i_diag_height;
      cam = i_cam;
   }

   mws_wp<gfx_camera> cam;
};
