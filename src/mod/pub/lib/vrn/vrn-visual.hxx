#pragma once

#include "pfm.hxx"
#include "gfx.hxx"
#include "gfx-vxo.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo-ext.hxx"
#include "vrn/vrn-diag-data.hxx"
#include "vrn/vrn-diag.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>


static const std::string vkb_cell_borders_sh = "mws-vkb-cell-borders";
static const std::string vkb_line_sh = "mws-vkb-line";
static const std::string vkb_point_sh = "mws-vkb-point";
static const std::string vkb_triangle_sh = "mws-vkb-triangle";


struct mws_vrn_obj_types
{
   static const uint32 none = 0;
   static const uint32 kernel_points = (1 << 0);
   static const uint32 nexus_points = (1 << 1);
   static const uint32 nexus_pairs = (1 << 2);
   static const uint32 cells = (1 << 3);
   static const uint32 delaunay_diag = (1 << 4);
   static const uint32 convex_hull = (1 << 5);
   static const uint32 cell_borders = (1 << 6);
   static const uint32 obj_count = 7;
};


enum class mws_vrn_diag_data_change_type
{
   diag_no_change,
   diag_rebuild_geometry,
   diag_rebuild_all,
};


class mws_vrn_gen
{
public:
   static mws_sp<mws_vrn_gen> nwi(mws_sp<mws_vrn_data> i_diag_data);
   void update_diag();
   void clear_points();
   void random_points();

   mws_sp<mws_vrn_diag> voronoi_diag_impl;
   mws_sp<mws_vrn_data> diag_data;
   mws_vrn_diag_data_change_type diag_data_state;
   std::vector<float> vx, vy;

private:
   mws_vrn_gen();
   void init(mws_sp<mws_vrn_data> i_diag_data);
};


class mws_vrn_cell_borders : public gfx_node
{
public:
   static mws_sp<mws_vrn_cell_borders> nwi();
   uint32 get_cell_borders_mesh_size() const;
   mws_sp<gfx_vxo> get_cell_borders_mesh_at(uint32 i_idx) const;
   void set_cell_borders_tex(mws_sp<gfx_tex> i_tex);
   void set_geometry(mws_sp<mws_vrn_data> i_diag_data, mws_vrn_cell_pt_id_vect& i_point_list, const std::vector<uint32>& i_point_count_list);

protected:
   mws_vrn_cell_borders();

   std::vector<mws_sp<gfx_vxo>> cell_borders_mesh_vect;
   mws_sp<gfx_tex> tex;
};


class mws_vrn_geom : public gfx_node
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

   static mws_sp<mws_vrn_geom> nwi(mws_sp<mws_vrn_data> i_diag_data, mws_sp<gfx_camera> i_cam);
   void update_geometry(mws_sp<mws_vrn_data> i_diag_data);
   void set_line_geometry(mws_vrn_nexus_pair_vect& i_line_points, mws_sp<gfx_vxo> i_mesh);
   void set_points_geometry(mws_vrn_pos_vect& i_point_list, mws_sp<gfx_vxo> i_mesh);
   void set_triangle_geometry(mws_vrn_pos_vect& i_point_list, const std::vector<uint32>& i_point_count_list, mws_sp<gfx_vxo> i_mesh);
   void set_cell_borders_geom(mws_sp<mws_vrn_data> i_diag_data, mws_vrn_cell_pt_id_vect& i_point_list, const std::vector<uint32>& i_point_count_list, mws_sp<mws_vrn_cell_borders> i_mesh);
   mws_sp<mws_vrn_cell_borders> get_cell_borders();
   void set_cell_borders(mws_sp<mws_vrn_cell_borders> i_cell_borders);
   void make_obj_visible(uint32 i_obj_type);

   mws_sp<mws_vrn_data> diag_data;
   mws_sp<gfx_plane> quad_mesh;
   mws_sp<gfx_vxo> voronoi_kernels_mesh;
   mws_sp<gfx_vxo> voronoi_nexus_mesh;
   mws_sp<gfx_vxo> nexus_pairs_mesh;
   mws_sp<gfx_vxo> voronoi_cell_faces_mesh;
   mws_sp<gfx_vxo> delaunay_diag_mesh;
   mws_sp<gfx_vxo> convex_hull_mesh;
   mws_sp<mws_vrn_cell_borders> cell_borders;

private:
   mws_vrn_geom(mws_sp<mws_vrn_data> i_diag_data);
   void init(mws_sp<gfx_camera> i_cam);
   void init_shaders();

   mws_sp<gfx_shader> vkb_cell_borders_shader;
   mws_sp<gfx_shader> vkb_line_shader;
   mws_sp<gfx_shader> vkb_point_shader;
   mws_sp<gfx_shader> vkb_triangle_shader;
};


class mws_vrn_main
{
public:
   ~mws_vrn_main();
   static mws_sp<mws_vrn_main> nwi(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam);
   void init();
   void toggle_voronoi_object(uint32 i_obj_type_mask);
   void update_diag();
   void update_geometry();
   // resizes the diagram using the existing kernel point set
   void resize(uint32 i_diag_width, uint32 i_diag_height);
   const mws_sp<mws_vrn_data> get_diag_data() const;
   void update_nexus_pairs_geometry();
   // creates a new diagram using a new set of kernel points
   void set_kernel_points(std::vector<glm::vec2> i_kernel_points);
   mws_vrn_diag::idx_dist get_kernel_idx_at(float i_x, float i_y) const;
   void move_kernel_to(uint32 i_idx, float i_x, float i_y);
   void insert_kernel_at(float i_x, float i_y);
   void remove_kernel(uint32 i_idx);

   mws_sp<mws_vrn_data> diag_data;
   mws_sp<mws_vrn_gen> vgen;
   mws_sp<mws_vrn_geom> vgeom;

private:
   mws_vrn_main();
   void setup(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam);

   mws_wp<gfx_camera> cam;
};
