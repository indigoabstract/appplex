#pragma once

#include "gfx-vxo.hpp"


class gfx_debug_vxo : public gfx_vxo
{
public:
	gfx_debug_vxo(vx_info ivxi, bool iis_submesh = false);

	virtual void render_mesh(shared_ptr<gfx_camera> icamera);
};


class gfx_obj_vxo : public gfx_vxo
{
public:
	gfx_obj_vxo();
	void operator=(const std::string& imesh_name);
	//virtual void render_mesh(shared_ptr<gl_camera> icamera);

	//std::vector<shared_ptr<gl_mesh> > mesh_list;
	bool is_loaded;
};


class gfx_plane : public gfx_vxo
{
public:
   gfx_plane();
   virtual void set_dimensions(float idx, float idy);
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

class gfx_icosahedron : public gfx_vxo
{
public:
	gfx_icosahedron();
	void set_dimensions(float iradius);
};

// variable polygon count
class gfx_vpc_box : public gfx_vxo
{
public:
	gfx_vpc_box();
	void set_dimensions(float iradius, int isegments);
};

class gfx_vpc_kubic_sphere : public gfx_vxo
{
public:
	gfx_vpc_kubic_sphere();
	void set_dimensions(float iradius, int isegments);
};

class gfx_vpc_ring_sphere : public gfx_vxo
{
public:
	gfx_vpc_ring_sphere();
	void set_dimensions(float iradius, int igrid_point_count);
};
