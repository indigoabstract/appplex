#pragma once

#include "gfx-vxo.hxx"


class gfx_debug_vxo : public gfx_vxo
{
public:
   gfx_debug_vxo(vx_info i_vxi, bool i_is_submesh = false);

   virtual void draw_in_sync(mws_sp<gfx_camera> i_camera);
};


class gfx_obj_vxo : public gfx_vxo
{
public:
   gfx_obj_vxo();
   void operator=(const std::string& i_mesh_name);
   //virtual void draw_in_sync(mws_sp<gl_camera> i_camera);

   //std::vector<mws_sp<gl_mesh> > mesh_list;
   bool is_loaded;
};


class gfx_plane : public gfx_vxo
{
public:
   gfx_plane(mws_sp<gfx> i_gi = nullptr);
   virtual void set_dimensions(float i_dx, float i_dy, float i_z_val = 0.f);
};


class gfx_billboard : public gfx_plane
{
public:
   gfx_billboard();
};


class gfx_grid : public gfx_vxo
{
public:
   gfx_grid();
   virtual void set_dimensions(int i_h_point_count, int i_v_point_count);
};


class gfx_box : public gfx_vxo
{
public:
   gfx_box();
   void set_dimensions(float i_dx, float i_dy, float i_dz);
};


class gfx_vpc_ring_sphere : public gfx_vxo
{
public:
   gfx_vpc_ring_sphere();
   void set_dimensions(float i_radius, int i_grid_point_count);
};


// a geometric prism with its vertical faces perpendicular to the top/bottom faces
class gfx_right_prism : public gfx_vxo
{
public:
   gfx_right_prism();
   // i_base_vertices are vertices of the bottom face(located in xOy) in clockwise order and defined as offsets relative to the bottom face's center
   // i_base_vertices needs to have at least 3 vertices to make a triangle, the simplest face type
   // i_height is the height along the positive y axis starting from the bottom face
   void set_dimensions(const std::vector<glm::vec2>& i_base_vertices, float i_height);
};


// a geometric pyramid with its vertical faces perpendicular to the top/bottom faces
class gfx_right_pyramid : public gfx_vxo
{
public:
   gfx_right_pyramid();
   // i_base_vertices are vertices of the bottom face(located in xOy) in clockwise order and defined as offsets relative to the bottom face's center
   // i_base_vertices needs to have at least 3 vertices to make a triangle, the simplest face type
   // i_height is the height along the positive y axis starting from the bottom face
   void set_dimensions(const std::vector<glm::vec2>& i_base_vertices, float i_height);
};
