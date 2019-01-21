#pragma once

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "pfm.hpp"
#include "gfx.hpp"
#include "gfx-vxo.hpp"
#include "gfx-camera.hpp"
#include "ext/gfx-surface.hpp"
#include "mws-vkb-diagram.hpp"
#include <rng/rng.hpp>
#include <glm/inc.hpp>


class vkb_voronoi_geometry : public gfx_node
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

   static mws_sp<vkb_voronoi_geometry> nwi(mws_sp<vkb_voronoi_data> idiag_data, mws_sp<gfx_camera> i_cam)
   {
      mws_sp<vkb_voronoi_geometry> inst(new vkb_voronoi_geometry(idiag_data));

      inst->init(i_cam);

      return inst;
   }

   void set_line_geometry(std::vector<vkb_nexus_pair>& iline_points, mws_sp<gfx_vxo> imesh)
   {
      int list_size = iline_points.size();
      imesh->set_size(list_size * 4, list_size * 6);
      std::vector<gfx_indices_type>& ks_indices_data = imesh->get_ix_buffer();
      std::vector<vx_fmt_3f_4f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4f_4b_2f_1i>*)&imesh->get_vx_buffer());

      glm::vec3 pos;
      vx_seg_data_4f nrm;
      vx_color_coord_4b color0;
      vx_color_coord_4b color1;
      gfx_uint id = 0;

      struct line_setup
      {
         static void setup(mws_sp<vkb_voronoi_data> diag_data, vkb_nexus_pair& inp, vkb_diagram_point& ip, float& isize, vx_color_coord_4b& ic)
         {
            vkb_diagram_point* p0 = diag_data->get_nexus_point_by_id(inp.nexus0_id);
            vkb_diagram_point* p1 = diag_data->get_nexus_point_by_id(inp.nexus1_id);
            ic.r = 0; ic.g = 0; ic.b = 255; ic.a = 255;
            isize = 1.f;
         }
      };

      glm::vec2 tex0 = { -0.5f, -0.5f };
      glm::vec2 tex1 = { +0.5f, -0.5f };
      glm::vec2 tex2 = { -0.5f, +0.5f };
      glm::vec2 tex3 = { +0.5f, +0.5f };

      for (int k = 0; k < list_size; k++)
      {
         vkb_nexus_pair& p = iline_points[k];
         vkb_diagram_point* np0 = diag_data->get_nexus_point_by_id(p.nexus0_id);
         vkb_diagram_point* np1 = diag_data->get_nexus_point_by_id(p.nexus1_id);
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

      int ind_size = ks_vertices_data.size() / 4;

      for (int k = 0; k < ind_size; k++)
      {
         int i_off = 4 * k;
         ks_indices_data[6 * k + 0] = i_off + 1;
         ks_indices_data[6 * k + 1] = i_off + 2;
         ks_indices_data[6 * k + 2] = i_off + 0;
         ks_indices_data[6 * k + 3] = i_off + 2;
         ks_indices_data[6 * k + 4] = i_off + 1;
         ks_indices_data[6 * k + 5] = i_off + 3;
      }

      imesh->update_data();
   }

   void set_points_geometry(vkb_position_vector& ipoint_list, mws_sp<gfx_vxo> imesh)
   {
      struct vx_fmt_p3f_t2f_1i
      {
         glm::vec3 pos;
         glm::vec2 tex;
         gfx_uint id;
      };

      glm::vec3 pos;
      glm::vec2 tex;
      int list_size = ipoint_list.size();
      imesh->set_size(list_size * 4, list_size * 6);
      std::vector<gfx_indices_type>& ks_indices_data = imesh->get_ix_buffer();
      std::vector<vx_fmt_p3f_t2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_p3f_t2f_1i>*)&imesh->get_vx_buffer());

      for (int k = 0; k < list_size; k++)
      {
         const glm::vec3& p = ipoint_list.get_position_at(k);
         gfx_uint id = ipoint_list.get_id_at(k);

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

      int ind_size = ks_vertices_data.size() / 4;

      for (int k = 0; k < ind_size; k++)
      {
         int i_off = 4 * k;
         ks_indices_data[6 * k + 0] = i_off + 1;
         ks_indices_data[6 * k + 1] = i_off + 2;
         ks_indices_data[6 * k + 2] = i_off + 0;
         ks_indices_data[6 * k + 3] = i_off + 2;
         ks_indices_data[6 * k + 4] = i_off + 1;
         ks_indices_data[6 * k + 5] = i_off + 3;
      }

      imesh->update_data();
   }

   void set_triangle_geometry(vkb_cell_point_id_vector& ipoint_list, const std::vector<int>& ipoint_count_list, mws_sp<gfx_vxo> imesh)
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
      int list_size = ipoint_count_list.size();
      int vx_count = 0;
      int ix_count = 0;

      for (int k = 0; k < list_size; k++)
      {
         int l2_size = ipoint_count_list[k];
         int ind_size = l2_size - 1;

         vx_count += l2_size;
         ix_count += ind_size * 3;
      }

      imesh->set_size(vx_count, ix_count);
      std::vector<gfx_indices_type>& ks_indices_data = imesh->get_ix_buffer();
      //std::vector<vx_fmt_3f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4b_2f_1i>*)&imesh->get_vx_buffer());
      vx_fmt_3f_4b_2f_1i* ks_vertices_data = (vx_fmt_3f_4b_2f_1i*)begin_ptr(imesh->get_vx_buffer());
      int idx = 0;
      int vx_idx = 0;
      int ix_idx = 0;

      for (int k = 0; k < list_size; k++)
      {
         gfx_uint id = 0;
         int l2_size = ipoint_count_list[k];
         int i_idx = idx;

         clr.r = 0;
         clr.g = 0;
         clr.b = 0;
         clr.a = 255;

         for (int l = 0; l < l2_size; l++)
         {
            const glm::vec3& p = ipoint_list.get_position_at(idx);

            pos = p;
            tex = glm::vec2(-0.5f, -0.5f);
            vx_fmt_3f_4b_2f_1i vx0 = { pos, clr, tex, id };
            ks_vertices_data[vx_idx++] = vx0;

            idx++;
         }

         int ind_size = l2_size - 1;

         for (int i = 0; i < ind_size; i++)
         {
            int i1 = i_idx + i + 1;
            int i2 = i + 2;

            if (i2 % l2_size == 0)
            {
               i2 = 1;
            }

            i2 += i_idx;
            ks_indices_data[ix_idx++] = i_idx;
            ks_indices_data[ix_idx++] = i1;
            ks_indices_data[ix_idx++] = i2;
         }
      }

      imesh->update_data();
   }

   mws_sp<vkb_voronoi_data> diag_data;
   mws_sp<gfx_vxo> nexus_pairs_mesh;
   mws_sp<gfx_vxo> voronoi_kernels_mesh;
   mws_sp<gfx_vxo> voronoi_nexus_mesh;
   mws_sp<gfx_vxo> voronoi_cell_faces_mesh;

private:
   vkb_voronoi_geometry(mws_sp<vkb_voronoi_data> idiag_data) : gfx_node(gfx::i())
   {
      diag_data = idiag_data;
   }

   void init(mws_sp<gfx_camera> i_cam)
   {
      float alpha_val = 0.8f;
      float line_thickness = 6.f;

      diag_data->info.nexus_points_visible = false;
      diag_data->info.nexus_pairs_visible = true;
      diag_data->info.kernel_points_visible = false;
      diag_data->info.cell_triangles_visible = true;

      // cache the shaders, to prevent recompiling
      shaders.push_back(gfx::i()->shader.new_program("sp_line"));
      shaders.push_back(gfx::i()->shader.new_program("sp_point"));
      shaders.push_back(gfx::i()->shader.new_program("sp_triangle"));

      nexus_pairs_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      nexus_pairs_mesh->name = "nexus_pairs_mesh";
      nexus_pairs_mesh->visible = diag_data->info.nexus_pairs_visible;
      (*nexus_pairs_mesh)[MP_SHADER_NAME] = "sp_line";
      //(*nexus_pairs_mesh)[MP_BLENDING] = MV_ALPHA;
      (*nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness;
      (*nexus_pairs_mesh)["u_v1_depth_offset"] = 0.001f;
      (*nexus_pairs_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 1.f, alpha_val);
      (*nexus_pairs_mesh)[MP_CULL_FRONT] = false;
      (*nexus_pairs_mesh)[MP_CULL_BACK] = false;
      (*nexus_pairs_mesh)[MP_BLENDING] = MV_ALPHA;
      //nexus_pairs_mesh->render_method = GLPT_POINTS;
      nexus_pairs_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_kernels_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_kernels_mesh->name = "voronoi_kernels_mesh";
      voronoi_kernels_mesh->visible = diag_data->info.kernel_points_visible;
      (*voronoi_kernels_mesh)[MP_SHADER_NAME] = "sp_point";
      (*voronoi_kernels_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_kernels_mesh)[MP_CULL_BACK] = false;
      (*voronoi_kernels_mesh)["u_v1_point_size"] = 2.f;
      (*voronoi_kernels_mesh)["u_v4_color"] = glm::vec4(0.f, 1.f, 1.f, alpha_val);
      (*voronoi_kernels_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_kernels_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_nexus_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_nexus_mesh->name = "voronoi_nexus_mesh";
      voronoi_nexus_mesh->visible = diag_data->info.nexus_points_visible;
      (*voronoi_nexus_mesh)[MP_SHADER_NAME] = "sp_point";
      (*voronoi_nexus_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_nexus_mesh)[MP_CULL_BACK] = false;
      (*voronoi_nexus_mesh)["u_v1_point_size"] = 2.5f;
      (*voronoi_nexus_mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, alpha_val);
      (*voronoi_nexus_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_nexus_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_cell_faces_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      voronoi_cell_faces_mesh->name = "voronoi_cell_faces_mesh";
      voronoi_cell_faces_mesh->visible = diag_data->info.cell_triangles_visible;
      (*voronoi_cell_faces_mesh)[MP_SHADER_NAME] = "sp_triangle";
      (*voronoi_cell_faces_mesh)["u_v1_point_size"] = 6.f;
      (*voronoi_cell_faces_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 0.f, alpha_val);
      (*voronoi_cell_faces_mesh)[MP_CULL_FRONT] = false;
      (*voronoi_cell_faces_mesh)[MP_CULL_BACK] = false;
      (*voronoi_cell_faces_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_cell_faces_mesh->camera_id_list = { i_cam->camera_id() };

      auto node = get_shared_ptr();
      node->attach(nexus_pairs_mesh);
      node->attach(voronoi_kernels_mesh);
      node->attach(voronoi_nexus_mesh);
      node->attach(voronoi_cell_faces_mesh);
   }

   std::vector<mws_sp<gfx_shader> > shaders;
};


class vkb_voronoi_main
{
public:
   static mws_sp<vkb_voronoi_main> nwi(int i_diag_width, int i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      mws_sp<vkb_voronoi_main> inst(new vkb_voronoi_main());

      inst->init(i_diag_width, i_diag_height, i_cam);

      return inst;
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

      if (diag_data->info.nexus_pairs_visible)
      {
         vgeom->set_line_geometry(diag_data->geom.nexus_pairs, vgeom->nexus_pairs_mesh);
      }

      if (diag_data->info.cell_triangles_visible)
      {
         vgeom->set_triangle_geometry(diag_data->geom.cell_points_ids, diag_data->geom.cell_point_count, vgeom->voronoi_cell_faces_mesh);
      }
   }

   void resize(int i_diag_width, int i_diag_height) {}

   const mws_sp<vkb_voronoi_data> get_diag_data() const { return diag_data; }
   struct idx_dist
   {
      int idx;
      float dist;
   };
   idx_dist get_kernel_idx_at(float i_x, float i_y) const
   {
      float min_dist = (float)0xffffffff;
      idx_dist ret = { -1, min_dist };
      glm::vec2 ref_pt(i_x, i_y);
      int size = diag_data->geom.kernel_points.size();

      for (int k = 0; k < size; k++)
      {
         auto& kp = diag_data->geom.kernel_points[k];
         glm::vec2 kp_pt(kp.position.x, kp.position.y);
         float dist = glm::distance2(ref_pt, kp_pt);

         if (dist < min_dist)
         {
            min_dist = dist;
            ret.idx = k;
         }
      }

      if (ret.idx >= 0)
      {
         ret.dist = glm::sqrt(min_dist);
      }

      return ret;
   }

   mws_sp<vkb_voronoi_data> diag_data;
   mws_sp<vkb_voronoi_geometry> vgeom;

private:
   vkb_voronoi_main() {}

   void init(int i_diag_width, int i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      diag_data = mws_sp<vkb_voronoi_data>(new vkb_voronoi_data());
      diag_data->geom.cell_points_ids.vdata = diag_data;
      diag_data->info.diag_width = i_diag_width;
      diag_data->info.diag_height = i_diag_height;
      vgeom = vkb_voronoi_geometry::nwi(diag_data, i_cam);
   }
};

#endif
