#include "stdafx.h"

#include "gfx-quad-2d.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfmgl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using gfx_vxo_util::set_mesh_data;

void gfx_quad_2d::set_anchor(e_anchor_types ianchor_type)
{
	anchor_type = ianchor_type;
	set_translation(tx, ty);
}

void gfx_quad_2d::set_translation(float ix, float iy)
{
	tx = ix;
	ty = iy;

	switch (anchor_type)
	{
	case e_top_left:
		position = glm::vec3(tx + sx / 2, ty + sy / 2, 0.f);
		break;

	case e_center:
		position = glm::vec3(tx, ty, 0.f);
		break;

	case e_btm_center:
		position = glm::vec3(tx, ty - sy / 2, 0.f);
		break;
	}
}

void gfx_quad_2d::set_rotation(float ia)
{
	a = ia;
}

void gfx_quad_2d::set_scale(float ix, float iy)
{
	sx = ix;
	sy = iy;
	scaling = glm::vec3(sx, sy, 1.f);
	set_translation(tx, ty);
}

void gfx_quad_2d::set_v_flip(bool iv_flip)
{
	if (iv_flip)
	{
		vertices_buffer[4] = 1;
		vertices_buffer[9] = 0;
		vertices_buffer[14] = 0;
		vertices_buffer[19] = 1;
	}
	else
	{
		vertices_buffer[4] = 0;
		vertices_buffer[9] = 1;
		vertices_buffer[14] = 1;
		vertices_buffer[19] = 0;
	}
}

void gfx_quad_2d::set_dimensions(float idx, float idy)
{
	dx = idx;
	dy = idy;
	gfx_plane::set_dimensions(dx, dy);
}

void gfx_quad_2d::set_tex_coord(glm::vec2 lt, glm::vec2 rt, glm::vec2 rb, glm::vec2 lb)
{
	float p = 0.5;
	const vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// xyz, uv
	{
		{ { -p * dx,  p * dy, 0 },{ 0, 0, -1 },{ lb.s, lb.t } },
		{ { -p * dx, -p * dy, 0 },{ 0, 0, -1 },{ lt.s, lt.t } },
		{ { p * dx, -p * dy, 0 },{ 0, 0, -1 },{ rt.s, rt.t } },
		{ { p * dx,  p * dy, 0 },{ 0, 0, -1 },{ rb.s, rb.t } },
	};

	const gfx_indices_type tindices_data[] =
	{
		//0, 1, 2, 0, 2, 3
		//2, 0, 1, 0, 2, 3,
		1, 0, 2, 3, 2, 0,
	};

	set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}

gfx_quad_2d::gfx_quad_2d()
{
	anchor_type = e_top_left;
	a = 0;
	sx = sy = 1;
}
