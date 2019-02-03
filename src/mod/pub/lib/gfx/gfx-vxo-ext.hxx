#pragma once

#include "gfx-vxo.hxx"


class gfx_debug_vxo : public gfx_vxo
{
public:
   gfx_debug_vxo(vx_info ivxi, bool iis_submesh = false);

   virtual void draw_in_sync(mws_sp<gfx_camera> icamera);
};


class gfx_obj_vxo : public gfx_vxo
{
public:
   gfx_obj_vxo();
   void operator=(const std::string& imesh_name);
   //virtual void draw_in_sync(mws_sp<gl_camera> icamera);

   //std::vector<mws_sp<gl_mesh> > mesh_list;
   bool is_loaded;
};


class gfx_plane : public gfx_vxo
{
public:
   gfx_plane(mws_sp<gfx> i_gi = nullptr);
   virtual void set_dimensions(float idx, float idy, float i_z_val = 0.f);
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
   void set_dimensions(float idx, float idy, float idz);
};


class gfx_vpc_ring_sphere : public gfx_vxo
{
public:
   gfx_vpc_ring_sphere();
   void set_dimensions(float iradius, int igrid_point_count);
};
