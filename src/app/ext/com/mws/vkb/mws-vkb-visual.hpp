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


enum diag_data_change_type
{
   e_diag_no_change,
   e_diag_rebuild_geometry,
   e_diag_rebuild_all,
};


class voronoi_gen
{
public:
   static shared_ptr<voronoi_gen> nwi(shared_ptr<voronoi_data> idiag_data)
   {
      shared_ptr<voronoi_gen> inst(new voronoi_gen());

      inst->init(idiag_data);

      return inst;
   }

   void update_diag();

   void clear_points()
   {
      vx.clear();
      vy.clear();
      diag_data_state = e_diag_no_change;
   }


   shared_ptr<voronoi_data> diag_data;
   diag_data_change_type diag_data_state;
   std::vector<float> vx, vy;

private:
   voronoi_gen()
   {
   }

   void init(shared_ptr<voronoi_data> idiag_data)
   {
      diag_data = idiag_data;
      diag_data_state = e_diag_no_change;
   }
};


class voronoi_geometry : public gfx_node
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

   static shared_ptr<voronoi_geometry> nwi(shared_ptr<voronoi_data> idiag_data, mws_sp<gfx_camera> i_cam)
   {
      shared_ptr<voronoi_geometry> inst(new voronoi_geometry(idiag_data));

      inst->init(i_cam);

      return inst;
   }

   void set_line_geometry(nexus_pair_vect& iline_points, shared_ptr<gfx_vxo> imesh)
   {
      int list_size = iline_points.get_size();
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
         static void setup(shared_ptr<voronoi_data> diag_data, nexus_pair& inp, nexus_point& ip, float& isize, vx_color_coord_4b& ic)
         {
            nexus_point* p0 = diag_data->get_nexus_point_by_id(inp.nexus0_id);
            nexus_point* p1 = diag_data->get_nexus_point_by_id(inp.nexus1_id);
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
         nexus_pair& p = iline_points.vect[k];
         nexus_point* np0 = diag_data->get_nexus_point_by_id(p.nexus0_id);
         nexus_point* np1 = diag_data->get_nexus_point_by_id(p.nexus1_id);
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

   void set_points_geometry(position_vector& ipoint_list, shared_ptr<gfx_vxo> imesh)
   {
      struct vx_fmt_p3f_t2f_1i
      {
         glm::vec3 pos;
         glm::vec2 tex;
         gfx_uint id;
      };

      glm::vec3 pos;
      glm::vec2 tex;
      int list_size = ipoint_list.get_size();
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

   void set_triangle_geometry(position_vector& ipoint_list, const std::vector<int>& ipoint_count_list, shared_ptr<gfx_vxo> imesh)
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
         gfx_uint id = ipoint_list.get_id_at(k);
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

   shared_ptr<voronoi_data> diag_data;
   shared_ptr<gfx_node> voronoi_landscape_mesh;
   shared_ptr<gfx_plane> quad_mesh;
   shared_ptr<gfx_vxo> delaunay_diag_mesh;
   shared_ptr<gfx_vxo> nexus_pairs_mesh;
   shared_ptr<gfx_vxo> convex_hull_mesh;
   shared_ptr<gfx_vxo> voronoi_kernels_mesh;
   shared_ptr<gfx_vxo> voronoi_nexus_mesh;
   shared_ptr<gfx_vxo> voronoi_cell_faces_mesh;

private:
   voronoi_geometry(shared_ptr<voronoi_data> idiag_data) : gfx_node(gfx::i())
   {
      diag_data = idiag_data;
   }

   void init(mws_sp<gfx_camera> i_cam)
   {
      float line_thickness = 6.f;

      diag_data->info.delaunay_diag_visible = false;
      diag_data->info.nexus_points_visible = true;
      diag_data->info.voronoi_diag_visible = true;
      diag_data->info.convex_hull_visible = false;
      diag_data->info.kernel_points_visible = true;
      diag_data->info.cell_triangles_visible = true;

      // cache the shaders, to prevent recompiling
      shaders.push_back(gfx::i()->shader.new_program("sp_line"));
      shaders.push_back(gfx::i()->shader.new_program("sp_point"));
      shaders.push_back(gfx::i()->shader.new_program("sp_triangle"));

      voronoi_landscape_mesh = shared_ptr<gfx_node>(new gfx_node(gfx::i()));
      voronoi_landscape_mesh->name = "voronoi_landscape_mesh";

      delaunay_diag_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      delaunay_diag_mesh->name = "delaunay_diag_mesh";
      delaunay_diag_mesh->visible = diag_data->info.delaunay_diag_visible;
      (*delaunay_diag_mesh)[MP_SHADER_NAME] = "sp_line";
      (*delaunay_diag_mesh)["u_v1_line_thickness"] = line_thickness;
      delaunay_diag_mesh->camera_id_list = { i_cam->camera_id() };

      nexus_pairs_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      nexus_pairs_mesh->name = "nexus_pairs_mesh";
      nexus_pairs_mesh->visible = diag_data->info.voronoi_diag_visible;
      (*nexus_pairs_mesh)[MP_SHADER_NAME] = "sp_line";
      //(*nexus_pairs_mesh)[MP_BLENDING] = MV_ALPHA;
      (*nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness;
      (*nexus_pairs_mesh)["u_v1_depth_offset"] = 0.001f;
      (*nexus_pairs_mesh)[MP_CULL_FRONT] = false;
      (*nexus_pairs_mesh)[MP_CULL_BACK] = false;
      //nexus_pairs_mesh->render_method = GLPT_POINTS;
      nexus_pairs_mesh->camera_id_list = { i_cam->camera_id() };

      convex_hull_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      convex_hull_mesh->name = "convex_hull_mesh";
      convex_hull_mesh->visible = diag_data->info.convex_hull_visible;
      (*convex_hull_mesh)[MP_SHADER_NAME] = "sp_line";
      (*convex_hull_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*convex_hull_mesh)["u_v1_line_thickness"] = line_thickness;
      convex_hull_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_kernels_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_kernels_mesh->name = "voronoi_kernels_mesh";
      voronoi_kernels_mesh->visible = diag_data->info.kernel_points_visible;
      (*voronoi_kernels_mesh)[MP_SHADER_NAME] = "sp_point";
      (*voronoi_kernels_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_kernels_mesh)[MP_CULL_BACK] = false;
      (*voronoi_kernels_mesh)["u_v1_point_size"] = 2.f;
      (*voronoi_kernels_mesh)["u_v4_color"] = glm::vec4(0, 1, 1, 1);
      voronoi_kernels_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_nexus_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_nexus_mesh->name = "voronoi_nexus_mesh";
      voronoi_nexus_mesh->visible = diag_data->info.nexus_points_visible;
      (*voronoi_nexus_mesh)[MP_SHADER_NAME] = "sp_point";
      (*voronoi_nexus_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_nexus_mesh)[MP_CULL_BACK] = false;
      (*voronoi_nexus_mesh)["u_v1_point_size"] = 2.5f;
      (*voronoi_nexus_mesh)["u_v4_color"] = glm::vec4(1.f, 0, 0, 1);
      voronoi_nexus_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_cell_faces_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      voronoi_cell_faces_mesh->name = "voronoi_cell_faces_mesh";
      voronoi_cell_faces_mesh->visible = diag_data->info.cell_triangles_visible;
      (*voronoi_cell_faces_mesh)[MP_SHADER_NAME] = "sp_triangle";
      (*voronoi_cell_faces_mesh)["u_v1_point_size"] = 6.f;
      (*voronoi_cell_faces_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 0.f, 1);
      (*voronoi_cell_faces_mesh)[MP_CULL_FRONT] = false;
      (*voronoi_cell_faces_mesh)[MP_CULL_BACK] = false;
      voronoi_cell_faces_mesh->camera_id_list = { i_cam->camera_id() };

      voronoi_landscape_mesh->attach(delaunay_diag_mesh);
      voronoi_landscape_mesh->attach(nexus_pairs_mesh);
      voronoi_landscape_mesh->attach(convex_hull_mesh);
      voronoi_landscape_mesh->attach(voronoi_kernels_mesh);
      voronoi_landscape_mesh->attach(voronoi_nexus_mesh);
      voronoi_landscape_mesh->attach(voronoi_cell_faces_mesh);
      voronoi_landscape_mesh->position = glm::vec3(0.f, 0.f, -1.f);

      shared_ptr<gfx_plane> quad_mesh(new gfx_plane());
      quad_mesh->name = "quad_mesh";
      gfx_plane& r_quad_mesh = *quad_mesh;
      //r_quad_mesh.camera_id_list.clear();
      //r_quad_mesh.camera_id_list.push_back(persp_cam->camera_id());
      r_quad_mesh[MP_SHADER_NAME] = "c-o-shader";
      r_quad_mesh["u_v4_color"] = glm::vec4(0, 0.05f, 0, 1);
      r_quad_mesh[MP_CULL_BACK] = false;
      r_quad_mesh.set_dimensions(diag_data->info.diag_width, diag_data->info.diag_height);
      r_quad_mesh.position = glm::vec3(0, 0, -0.5f);
      quad_mesh->camera_id_list = { i_cam->camera_id() };
      //voronoi_landscape_mesh->attach(quad_mesh);

      //voronoi_landscape_mesh->position = glm::vec3(diag_data->info.diag_width / 2.f, diag_data->info.diag_height / 2.f, 0.f);
      get_shared_ptr()->attach(voronoi_landscape_mesh);
   }

   std::vector<shared_ptr<gfx_shader> > shaders;
};


class voronoi_main
{
public:
   static shared_ptr<voronoi_main> nwi(int i_diag_width, int i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      shared_ptr<voronoi_main> inst(new voronoi_main());

      inst->init(i_diag_width, i_diag_height, i_cam);

      return inst;
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

      if (diag_data->info.voronoi_diag_visible)
      {
         vgeom->set_line_geometry(diag_data->geom.nexus_pair_points, vgeom->nexus_pairs_mesh);
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

   void update_nexus_pairs_geometry()
   {
      if (diag_data->info.voronoi_diag_visible)
      {
         //vgen->set_altitude(diag_data->geom.nexus_pair_points);
         vgeom->set_line_geometry(diag_data->geom.nexus_pair_points, vgeom->nexus_pairs_mesh);
      }
   }

   void resize(int i_diag_width, int i_diag_height) {}

   void set_kernel_points(std::vector<glm::vec2> i_kernel_points);
   const mws_sp<voronoi_data> get_diag_data() const { return diag_data; }
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
      int size = diag_data->geom.kernel_points.get_size();

      for (size_t k = 0; k < size; k++)
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

   shared_ptr<voronoi_data> diag_data;
   shared_ptr<voronoi_gen> vgen;
   shared_ptr<voronoi_geometry> vgeom;

private:
   voronoi_main()
   {
   }

   void init(int i_diag_width, int i_diag_height, mws_sp<gfx_camera> i_cam)
   {
      diag_data = shared_ptr<voronoi_data>(new voronoi_data());
      diag_data->geom.cell_points_ids.vdata = diag_data;
      diag_data->info.diag_width = i_diag_width;
      diag_data->info.diag_height = i_diag_height;

      vgen = voronoi_gen::nwi(diag_data);
      vgeom = voronoi_geometry::nwi(diag_data, i_cam);
   }
};

void voronoi_gen::update_diag()
{
}


void voronoi_main::set_kernel_points(std::vector<glm::vec2> i_kernel_points)
{
   std::vector<float> vx, vy;

   vx.resize(i_kernel_points.size());
   vy.resize(i_kernel_points.size());

   for (size_t k = 0; k < i_kernel_points.size(); k++)
   {
      vx[k] = i_kernel_points[k].x;
      vy[k] = i_kernel_points[k].y;
   }

   update_diag();
}

#endif
