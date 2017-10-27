#include "stdafx.h"

#include "gfx-surface.hpp"
#include <rng/rng.hpp>


namespace gfx_vxo_util
{
	vx_pos_coord_3f lerp_pos(const vx_pos_coord_3f& start, const vx_pos_coord_3f& end, float t)
	{
		vx_pos_coord_3f r = {start.x * (1.f - t) + end.x * t, start.y * (1.f - t) + end.y * t, start.z * (1.f - t) + end.z * t};

		return r;
	}

	vx_norm_coord_3f lerp_nrm(const vx_norm_coord_3f& start, const vx_norm_coord_3f& end, float t)
	{
		vx_norm_coord_3f r = {start.nx * (1.f - t) + end.nx * t, start.ny * (1.f - t) + end.ny * t, start.nz * (1.f - t) + end.nz * t};

		return r;
	}

	vx_tex_coord_2f lerp_tex(const vx_tex_coord_2f& start, const vx_tex_coord_2f& end, float t)
	{
		vx_tex_coord_2f r = {start.u * (1.f - t) + end.u * t, start.v * (1.f - t) + end.v * t};

		return r;
	}
}

using gfx_vxo_util::set_mesh_data;
using gfx_vxo_util::lerp_pos;
using gfx_vxo_util::lerp_nrm;
using gfx_vxo_util::lerp_tex;


gfx_debug_vxo::gfx_debug_vxo(vx_info ivxi, bool iis_submesh) : gfx_vxo(ivxi, iis_submesh)
{
}

void gfx_debug_vxo::render_mesh(shared_ptr<gfx_camera> icamera)
{
	gfx_vxo::render_mesh(icamera);
}


gfx_obj_vxo::gfx_obj_vxo() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
	is_loaded = false;
}

void gfx_obj_vxo::operator=(const std::string& imesh_name)
{
	set_mesh_name(imesh_name);
}

//void gfx_obj_mesh::render_mesh(shared_ptr<gfx_camera> icamera)
//{
//	std::vector<shared_ptr<gfx_mesh> >::iterator it = mesh_list.begin();
//
//	for(; it != mesh_list.end(); it++)
//	{
//		(*it)->render_mesh(icamera);
//	}
//}


gfx_plane::gfx_plane() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_plane::set_dimensions(float idx, float idy)
{
	float p = 0.5;
	const vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// xyz, uv
	{
		{{-p * idx,  p * idy, 0}, {0, 0, -1}, {0, 1}},
		{{-p * idx, -p * idy, 0}, {0, 0, -1}, {0, 0}},
		{{ p * idx, -p * idy, 0}, {0, 0, -1}, {1, 0}},
		{{ p * idx,  p * idy, 0}, {0, 0, -1}, {1, 1}},
	};

	const gfx_indices_type tindices_data[] =
	{
		//0, 1, 2, 0, 2, 3
		//2, 0, 1, 0, 2, 3,
		1, 0, 2, 3, 2, 0,
	};

	set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_grid::gfx_grid() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_grid::set_dimensions(int i_h_point_count, int i_v_point_count)
{
   static float p = 1.f;
   static glm::vec3 vx[] =
   {
      glm::vec3(-p, p, 0), glm::vec3(-p, -p, 0), glm::vec3(p, -p, 0), glm::vec3(p, p, 0),
   };
   static glm::vec2 tx[] =
   {
      glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1),
   };
   std::vector<vx_fmt_p3f_n3f_t2f> tvertices_data(i_h_point_count * i_v_point_count);
   std::vector<gfx_indices_type> tindices_data;

   for (int i = 0; i < i_v_point_count; i++)
   {
      float vf = i / float(i_v_point_count - 1);
      glm::vec3 left_vx = glm::mix(vx[0], vx[1], vf);
      glm::vec2 left_tx = glm::mix(tx[0], tx[1], vf);
      glm::vec3 right_vx = glm::mix(vx[3], vx[2], vf);
      glm::vec2 right_tx = glm::mix(tx[3], tx[2], vf);

      for (int j = 0; j < i_h_point_count; j++)
      {
         float hf = j / float(i_h_point_count - 1);
         glm::vec3 it_vx = glm::mix(left_vx, right_vx, hf);
         glm::vec2 it_tx = glm::mix(left_tx, right_tx, hf);
         auto& r = tvertices_data[i_h_point_count * i + j];

         r.pos.set(it_vx);
         r.nrm.set(glm::vec3(0, 0, -1));
         r.tex.set(it_tx.x, it_tx.y);
      }
   }

   for (int i = 1; i < i_v_point_count; i++)
   {
      for (int j = 1; j < i_h_point_count; j++)
      {
         int v = i_h_point_count * i;
         int v_m1 = i_h_point_count * (i - 1);
         int h = j;
         int h_m1 = j - 1;

         // lower left triangle: 1, 0, 2 in quad drawing
         tindices_data.push_back(v + h_m1);
         tindices_data.push_back(v_m1 + h_m1);
         tindices_data.push_back(v + h);
         // upper right triangle: 3, 2, 0 in quad drawing
         tindices_data.push_back(v_m1 + h);
         tindices_data.push_back(v + h);
         tindices_data.push_back(v_m1 + h_m1);
      }
   }

   set_mesh_data((const uint8*)tvertices_data.data(), sizeof(vx_fmt_p3f_n3f_t2f) * tvertices_data.size(),
      tindices_data.data(), sizeof(gfx_indices_type) * tindices_data.size(), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_box::gfx_box() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_box::set_dimensions(float idx, float idy, float idz)
{
	vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// x,y,z,	nx,ny,nz,	u,v,
	{
		{{-1, -1, +1,}, {}, {0, 0,}},		{{-1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, +1,}, {}, {1, 1,}},		{{+1, -1, +1,}, {}, {1, 0,}}, // front
		{{+1, -1, -1,}, {}, {0, 0,}},		{{+1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, -1,}, {}, {1, 1,}},		{{-1, -1, -1,}, {}, {1, 0,}}, // back
		{{-1, +1, +1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, +1, +1,}, {}, {1, 0,}}, // top
		{{+1, -1, +1,}, {}, {0, 0,}},		{{+1, -1, -1,}, {}, {0, 1,}},		{{-1, -1, -1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // bottom
		{{-1, -1, -1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, +1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // left
		{{+1, -1, +1,}, {}, {0, 0,}},		{{+1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // right
	};
	const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);

	const gfx_indices_type tindices_data[] =
	{
		1, 0, 2, 2, 0, 3,
		5, 4, 6, 6, 4, 7,
		9, 8, 10, 10, 8, 11,
		13, 12, 14, 14, 12, 15,
		17, 16, 18, 18, 16, 19,
		21, 20, 22, 22, 20, 23,
	};

	for (int k = 0; k < 6; k++)
	{
		int i = k * 4;
		vx_pos_coord_3f& pos0 = tvertices_data[i + 0].pos;
		vx_pos_coord_3f& pos1 = tvertices_data[i + 1].pos;
		vx_pos_coord_3f& pos2 = tvertices_data[i + 2].pos;
		glm::vec3 v0(pos0.x, pos0.y, pos0.z);
		glm::vec3 v1(pos1.x, pos1.y, pos1.z);
		glm::vec3 v2(pos2.x, pos2.y, pos2.z);
		glm::vec3 va = v0 - v1;
		glm::vec3 vb = v2 - v1;
		glm::vec3 sn = glm::cross(va, vb);
		sn = glm::normalize(sn);

		for(int l = 0; l < 4; l++)
		{
			tvertices_data[i + l].nrm.nx = sn.x;
			tvertices_data[i + l].nrm.ny = sn.y;
			tvertices_data[i + l].nrm.nz = sn.z;
		}
	}

	for (int k = 0; k <tvertices_data_length; k++)
	{
		vx_pos_coord_3f& pos = tvertices_data[k].pos;

		pos.x *= idx;
		pos.y *= idy;
		pos.z *= idz;
	}

	set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_icosahedron::gfx_icosahedron() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_icosahedron::set_dimensions(float iradius)
{
	float size = iradius;
	float t = (1.f + glm::sqrt(5.f)) / 2.f;


	static vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// x,y,z,	nx,ny,nz,	u,v,
	{
		{{-size, t*size, 0}, {}, {0, 0,}},
		{{size, t*size, 0}, {}, {0, 0,}},
		{{-size, -t*size, 0}, {}, {0, 0,}},
		{{size, -t*size, 0}, {}, {0, 0,}},

		{{0, -size, t*size}, {}, {0, 0,}},
		{{0, size, t*size}, {}, {0, 0,}},
		{{0, -size, -t*size}, {}, {0, 0,}},
		{{0, size, -t*size}, {}, {0, 0,}},

		{{t*size, 0, -size}, {}, {0, 0,}},
		{{t*size, 0, size}, {}, {0, 0,}},
		{{-t*size, 0, -size}, {}, {0, 0,}},
		{{-t*size, 0, size}, {}, {0, 0,}},
	};
	const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);

	const gfx_indices_type tindices_data[] =
	{
		// 5 faces around point 0
		0, 11, 5,
		0, 5, 1,
		0, 1, 7,
		0, 7, 10,
		0, 10, 11,

		// 5 adjacent faces
		1, 5, 9,
		5, 11, 4,
		11, 10, 2,
		10, 7, 6,
		7, 1, 8,

		// 5 faces around point 3
		3, 9, 4,
		3, 4, 2,
		3, 2, 6,
		3, 6, 8,
		3, 8, 9,

		// 5 adjacent faces
		4, 9, 5,
		2, 4, 11,
		6, 2, 10,
		8, 6, 7,
		9, 8, 1,
	};

	set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_vpc_box::gfx_vpc_box() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_vpc_box::set_dimensions(float iradius, int isegments)
{
	vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// x,y,z,	nx,ny,nz,	u,v,
	{
		{{-1, -1, +1,}, {}, {0, 0,}},		{{-1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, +1,}, {}, {1, 1,}},		{{+1, -1, +1,}, {}, {1, 0,}}, // front
		{{+1, -1, -1,}, {}, {0, 0,}},		{{+1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, -1,}, {}, {1, 1,}},		{{-1, -1, -1,}, {}, {1, 0,}}, // back
		{{-1, +1, +1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, +1, +1,}, {}, {1, 0,}}, // top
		{{+1, -1, +1,}, {}, {0, 0,}},		{{+1, -1, -1,}, {}, {0, 1,}},		{{-1, -1, -1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // bottom
		{{-1, -1, -1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, +1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // left
		{{+1, -1, +1,}, {}, {0, 0,}},		{{+1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // right
	};
	const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);

	for (int k = 0; k < 6; k++)
	{
		int i = k * 4;
		vx_pos_coord_3f& pos0 = tvertices_data[i + 0].pos;
		vx_pos_coord_3f& pos1 = tvertices_data[i + 1].pos;
		vx_pos_coord_3f& pos2 = tvertices_data[i + 2].pos;
		glm::vec3 v0(pos0.x, pos0.y, pos0.z);
		glm::vec3 v1(pos1.x, pos1.y, pos1.z);
		glm::vec3 v2(pos2.x, pos2.y, pos2.z);
		glm::vec3 va = v0 - v1;
		glm::vec3 vb = v2 - v1;
		glm::vec3 sn = glm::cross(va, vb);
		sn = glm::normalize(sn);

		for(int l = 0; l < 4; l++)
		{
			tvertices_data[i + l].nrm.nx = sn.x;
			tvertices_data[i + l].nrm.ny = sn.y;
			tvertices_data[i + l].nrm.nz = sn.z;
		}
	}

	for (int k = 0; k <tvertices_data_length; k++)
	{
		vx_pos_coord_3f& pos = tvertices_data[k].pos;

		pos.x *= iradius;
		pos.y *= iradius;
		pos.z *= iradius;
	}

	int side_vertex_count = isegments + 1;
	int face_vertex_count = side_vertex_count * side_vertex_count;
	int total_face_vertex_count = 6 * face_vertex_count;
	std::vector<vx_fmt_p3f_n3f_t2f> ks_vertices_data;
	std::vector<gfx_indices_type> ks_indices_data;

	for (int k = 0, idx = 0; k < 6; k++)
	{
		const vx_fmt_p3f_n3f_t2f& bl = tvertices_data[4 * k + 0];
		const vx_fmt_p3f_n3f_t2f& tl = tvertices_data[4 * k + 1];
		const vx_fmt_p3f_n3f_t2f& tr = tvertices_data[4 * k + 2];
		const vx_fmt_p3f_n3f_t2f& br = tvertices_data[4 * k + 3];

		for (int i = 0; i < isegments; i++)
		{
			float vt1 = float(i) / isegments;
			float vt2 = float(i + 1) / isegments;
			vx_pos_coord_3f hstart_pos1 = lerp_pos(tl.pos, bl.pos, vt1);
			vx_pos_coord_3f hstart_pos2 = lerp_pos(tl.pos, bl.pos, vt2);
			vx_pos_coord_3f hend_pos1 = lerp_pos(tr.pos, br.pos, vt1);
			vx_pos_coord_3f hend_pos2 = lerp_pos(tr.pos, br.pos, vt2);
			vx_norm_coord_3f hstart_nrm1 = lerp_nrm(tl.nrm, bl.nrm, vt1);
			vx_norm_coord_3f hstart_nrm2 = lerp_nrm(tl.nrm, bl.nrm, vt2);
			vx_norm_coord_3f hend_nrm1 = lerp_nrm(tr.nrm, br.nrm, vt1);
			vx_norm_coord_3f hend_nrm2 = lerp_nrm(tr.nrm, br.nrm, vt2);
			vx_tex_coord_2f hstart_tex1 = lerp_tex(tl.tex, bl.tex, vt1);
			vx_tex_coord_2f hstart_tex2 = lerp_tex(tl.tex, bl.tex, vt2);
			vx_tex_coord_2f hend_tex1 = lerp_tex(tr.tex, br.tex, vt1);
			vx_tex_coord_2f hend_tex2 = lerp_tex(tr.tex, br.tex, vt2);

			for (int j = 0; j < isegments; j++, idx++)
			{
				int i_off = idx * 4;
				float ht1 = float(j) / isegments;
				float ht2 = float(j + 1) / isegments;
				vx_fmt_p3f_n3f_t2f vx0 = {lerp_pos(hstart_pos2, hend_pos2, ht1), lerp_nrm(hstart_nrm2, hend_nrm2, ht1), lerp_tex(hstart_tex2, hend_tex2, ht1)};
				vx_fmt_p3f_n3f_t2f vx1 = {lerp_pos(hstart_pos1, hend_pos1, ht1), lerp_nrm(hstart_nrm1, hend_nrm1, ht1), lerp_tex(hstart_tex1, hend_tex1, ht1)};
				vx_fmt_p3f_n3f_t2f vx2 = {lerp_pos(hstart_pos1, hend_pos1, ht2), lerp_nrm(hstart_nrm1, hend_nrm1, ht2), lerp_tex(hstart_tex1, hend_tex1, ht2)};
				vx_fmt_p3f_n3f_t2f vx3 = {lerp_pos(hstart_pos2, hend_pos2, ht2), lerp_nrm(hstart_nrm2, hend_nrm2, ht2), lerp_tex(hstart_tex2, hend_tex2, ht2)};

				ks_vertices_data.push_back(vx0);
				ks_vertices_data.push_back(vx1);
				ks_vertices_data.push_back(vx2);
				ks_vertices_data.push_back(vx3);
				ks_indices_data.push_back(i_off + 1);
				ks_indices_data.push_back(i_off + 0);
				ks_indices_data.push_back(i_off + 2);
				ks_indices_data.push_back(i_off + 2);
				ks_indices_data.push_back(i_off + 0);
				ks_indices_data.push_back(i_off + 3);
			}
		}
	}

	int vdata_size = ks_vertices_data.size() * sizeof(vx_fmt_p3f_n3f_t2f);
	int idata_size = ks_indices_data.size() * sizeof(gfx_indices_type);
	set_mesh_data((const uint8*)begin_ptr(ks_vertices_data), vdata_size, begin_ptr(ks_indices_data), idata_size, std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_vpc_kubic_sphere::gfx_vpc_kubic_sphere() : gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v3_normal, a_v2_tex_coord"))
{
}

enum segment_point
{
	start_point,
	end_point,
};

enum cube_face_type
{
	front_face,
	back_face,
	top_face,
	bottom_face,
	left_face,
	right_face,
};
struct segment
{
	segment_point start;
	segment_point end;
};

struct cube_edge
{
	cube_face_type face_type;
	segment vertical;
	segment horizontal;
};

struct common_edge
{
	cube_edge first;
	cube_edge second;
};

void gfx_vpc_kubic_sphere::set_dimensions(float iradius, int isegments)
{
	static vx_fmt_p3f_n3f_t2f tvertices_data[] =
		// x,y,z,	nx,ny,nz,	u,v,
	{
		{{-1, -1, +1,}, {}, {0, 0,}},		{{-1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, +1,}, {}, {1, 1,}},		{{+1, -1, +1,}, {}, {1, 0,}}, // front
		{{+1, -1, -1,}, {}, {0, 0,}},		{{+1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, -1,}, {}, {1, 1,}},		{{-1, -1, -1,}, {}, {1, 0,}}, // back
		{{-1, +1, +1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, +1, +1,}, {}, {1, 0,}}, // top
		{{-1, -1, -1,}, {}, {0, 0,}},		{{-1, -1, +1,}, {}, {0, 1,}},		{{+1, -1, +1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // bottom
		{{-1, -1, -1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, +1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // left
		{{+1, -1, +1,}, {}, {0, 0,}},		{{+1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // right
	};
	const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);
	int side_vertex_count = isegments + 1;
	int face_vertex_count = side_vertex_count * side_vertex_count;
	int total_face_vertex_count = 6 * face_vertex_count;
	std::vector<vx_fmt_p3f_c4b_n3f_t2f> ks_vertices_data;
	std::vector<gfx_indices_type> ks_indices_data;
	RNG rng;
	std::vector<std::vector<std::vector<float> > > displ_map(6);
	float displ_delta = iradius * 0.005f;
	float displ_pos_range = 100000.f;

	static int face_links[6][4] =
	{
		{top_face, right_face, bottom_face, left_face}, // front_face
		{top_face, left_face, bottom_face, right_face}, // back_face
		{back_face, right_face, front_face, left_face}, // top_face
		{front_face, right_face, back_face, left_face}, // bottom_face
		{top_face, front_face, bottom_face, back_face}, // left_face
		{top_face, back_face, bottom_face, front_face}, // right_face
	};

	static common_edge common_edges_tab[] =
	{
		{{front_face, {start_point, start_point}, {start_point, end_point}}, {top_face, {end_point, end_point}, {start_point, end_point}}},
		{{front_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {start_point, end_point}, {start_point, start_point}}},
		{{top_face, {start_point, start_point}, {start_point, end_point}}, {back_face, {start_point, start_point}, {end_point, start_point}}},
		{{top_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {start_point, start_point}, {end_point, start_point}}},
		{{left_face, {start_point, start_point}, {start_point, end_point}}, {top_face, {start_point, end_point}, {start_point, start_point}}},
		{{left_face, {start_point, end_point}, {end_point, end_point}}, {front_face, {start_point, end_point}, {start_point, start_point}}},
		{{left_face, {start_point, end_point}, {start_point, start_point}}, {back_face, {start_point, end_point}, {end_point, end_point}}},
		{{left_face, {end_point, end_point}, {start_point, end_point}}, {bottom_face, {end_point, start_point}, {start_point, start_point}}},
		{{bottom_face, {start_point, start_point}, {start_point, end_point}}, {front_face, {end_point, end_point}, {start_point, end_point}}},
		{{bottom_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {end_point, end_point}, {start_point, end_point}}},
		{{right_face, {start_point, end_point}, {end_point, end_point}}, {back_face, {start_point, end_point}, {start_point, start_point}}},
		{{back_face, {end_point, end_point}, {start_point, end_point}}, {bottom_face, {end_point, end_point}, {end_point, start_point}}},
	};
	vx_color_coord_4b face_colors[6] =
	{
		{255, 000, 000, 255},
		{000, 255, 000, 255},
		{000, 000, 255, 255},
		{255, 255, 000, 255},
		{000, 255, 255, 255},
		{255, 000, 255, 255},
	};

	for (int k = 0; k < 6; k++)
	{
		displ_map[k].resize(side_vertex_count);

		for (int i = 0; i < side_vertex_count; i++)
		{
			displ_map[k][i].resize(side_vertex_count);

			for (int j = 0; j < side_vertex_count; j++)
			{
				displ_map[k][i][j] = rng.range(-displ_pos_range, displ_pos_range) / displ_pos_range * displ_delta;
			}
		}
	}

	struct point_val
	{
		int operator()(segment_point isp, int istart, int iend) const
		{
			if(isp == start_point)
			{
				return istart;
			}

			return iend;
		}
	};
	point_val pv;

	for (int k = 0; k < 12; k++)
	{
		common_edge c = common_edges_tab[k];
		cube_edge fe = c.first;
		cube_edge se = c.second;
		// vertical coord, first, second
		int vfs = pv(fe.vertical.start, 0, isegments);
		int vfe = pv(fe.vertical.end, 0, isegments);
		int vfi = (vfe - vfs) / isegments;
		int vss = pv(se.vertical.start, 0, isegments);
		int vse = pv(se.vertical.end, 0, isegments);
		int vsi = (vse - vss) / isegments;
		// horizontal coord, first, second
		int hfs = pv(fe.horizontal.start, 0, isegments);
		int hfe = pv(fe.horizontal.end, 0, isegments);
		int hfi = (hfe - hfs) / isegments;
		int hss = pv(se.horizontal.start, 0, isegments);
		int hse = pv(se.horizontal.end, 0, isegments);
		int hsi = (hse - hss) / isegments;

		for (int idx = 0; idx < side_vertex_count; idx++)
		{
			float& fv = displ_map[fe.face_type][vfs][hfs];
			float& sv = displ_map[se.face_type][vss][hss];
			float av = (fv + sv) * 0.5f;
			fv = sv = av;
			vfs += vfi;
			vss += vsi;
			hfs += hfi;
			hss += hsi;
		}
	}

	int vi[4] = {1, 0, 0, 1};
	int vj[4] = {0, 0, 1, 1};

	for (int k = 0, idx = 0; k < 6; k++)
	{
		const vx_fmt_p3f_n3f_t2f& bl = tvertices_data[4 * k + 0];
		const vx_fmt_p3f_n3f_t2f& tl = tvertices_data[4 * k + 1];
		const vx_fmt_p3f_n3f_t2f& tr = tvertices_data[4 * k + 2];
		const vx_fmt_p3f_n3f_t2f& br = tvertices_data[4 * k + 3];

		for (int i = 0; i < isegments; i++)
		{
			float vt1 = float(i) / isegments;
			float vt2 = float(i + 1) / isegments;
			vx_pos_coord_3f hstart_pos1 = lerp_pos(tl.pos, bl.pos, vt1);
			vx_pos_coord_3f hstart_pos2 = lerp_pos(tl.pos, bl.pos, vt2);
			vx_pos_coord_3f hend_pos1 = lerp_pos(tr.pos, br.pos, vt1);
			vx_pos_coord_3f hend_pos2 = lerp_pos(tr.pos, br.pos, vt2);
			vx_tex_coord_2f hstart_tex1 = lerp_tex(tl.tex, bl.tex, vt1);
			vx_tex_coord_2f hstart_tex2 = lerp_tex(tl.tex, bl.tex, vt2);
			vx_tex_coord_2f hend_tex1 = lerp_tex(tr.tex, br.tex, vt1);
			vx_tex_coord_2f hend_tex2 = lerp_tex(tr.tex, br.tex, vt2);

			for (int j = 0; j < isegments; j++, idx++)
			{
				int i_off = idx * 4;
				float ht1 = float(j) / isegments;
				float ht2 = float(j + 1) / isegments;
				vx_pos_coord_3f pos[4] =
				{
					lerp_pos(hstart_pos2, hend_pos2, ht1),
					lerp_pos(hstart_pos1, hend_pos1, ht1),
					lerp_pos(hstart_pos1, hend_pos1, ht2),
					lerp_pos(hstart_pos2, hend_pos2, ht2),
				};
				vx_norm_coord_3f nrm[4];
				glm::vec3 gpos[4];

				for (int v = 0; v < 4; v++)
				{
					gpos[v] = glm::normalize(glm::vec3(pos[v].x, pos[v].y, pos[v].z));
					nrm[v].nx = 0;//gpos[v].x;
					nrm[v].ny = 0;//gpos[v].y;
					nrm[v].nz = 0;//gpos[v].z;

					float height_val = 0;//displ_map[k][i + vi[v]][j + vj[v]];

					gpos[v] *= (iradius + height_val);
					pos[v].x = gpos[v].x;
					pos[v].y = gpos[v].y;
					pos[v].z = gpos[v].z;
					//pos[v].x *= iradius;
					//pos[v].y *= iradius;
					//pos[v].z *= iradius;
				}

				vx_fmt_p3f_c4b_n3f_t2f vx0 = {pos[0], face_colors[k], nrm[0], lerp_tex(hstart_tex2, hend_tex2, ht1)};
				vx_fmt_p3f_c4b_n3f_t2f vx1 = {pos[1], face_colors[k], nrm[1], lerp_tex(hstart_tex1, hend_tex1, ht1)};
				vx_fmt_p3f_c4b_n3f_t2f vx2 = {pos[2], face_colors[k], nrm[2], lerp_tex(hstart_tex1, hend_tex1, ht2)};
				vx_fmt_p3f_c4b_n3f_t2f vx3 = {pos[3], face_colors[k], nrm[3], lerp_tex(hstart_tex2, hend_tex2, ht2)};

				ks_vertices_data.push_back(vx0);
				ks_vertices_data.push_back(vx1);
				ks_vertices_data.push_back(vx2);
				ks_vertices_data.push_back(vx3);
				ks_indices_data.push_back(i_off + 1);
				ks_indices_data.push_back(i_off + 0);
				ks_indices_data.push_back(i_off + 2);
				ks_indices_data.push_back(i_off + 2);
				ks_indices_data.push_back(i_off + 0);
				ks_indices_data.push_back(i_off + 3);
			}
		}
	}

	for(int i = 0; i < ks_indices_data.size(); i += 3)
	{
		vx_fmt_p3f_c4b_n3f_t2f& tv0 = ks_vertices_data[ks_indices_data[i]];
		vx_fmt_p3f_c4b_n3f_t2f& tv1 = ks_vertices_data[ks_indices_data[i+1]];
		vx_fmt_p3f_c4b_n3f_t2f& tv2 = ks_vertices_data[ks_indices_data[i+2]];
		glm::vec3 v0(tv0.pos.x, tv0.pos.y, tv0.pos.z);
		glm::vec3 v1(tv1.pos.x, tv1.pos.y, tv1.pos.z);
		glm::vec3 v2(tv2.pos.x, tv2.pos.y, tv2.pos.z);

		glm::vec3 normal = -glm::normalize(glm::cross(v2 - v0, v1 - v0));
		vx_norm_coord_3f nrm = {normal.x, normal.y, normal.z};

		vx_norm_coord_3f& n0 = ks_vertices_data[ks_indices_data[i]].nrm;
		vx_norm_coord_3f& n1 = ks_vertices_data[ks_indices_data[i+1]].nrm;
		vx_norm_coord_3f& n2 = ks_vertices_data[ks_indices_data[i+2]].nrm;
		n0.nx += nrm.nx;
		n0.ny += nrm.ny;
		n0.nz += nrm.nz;
		n1.nx += nrm.nx;
		n1.ny += nrm.ny;
		n1.nz += nrm.nz;
		n2.nx += nrm.nx;
		n2.ny += nrm.ny;
		n2.nz += nrm.nz;
	}

	for(int i = 0; i < ks_vertices_data.size(); i++)
	{
		glm::vec3 n(ks_vertices_data[i].nrm.nx, ks_vertices_data[i].nrm.ny, ks_vertices_data[i].nrm.nz);
		n = glm::normalize(n);
		vx_norm_coord_3f nn = {n.x, n.y, n.z};
		ks_vertices_data[i].nrm = nn;
	}

	int vdata_size = ks_vertices_data.size() * sizeof(vx_fmt_p3f_c4b_n3f_t2f);
	int idata_size = ks_indices_data.size() * sizeof(gfx_indices_type);
	set_mesh_data((const uint8*)begin_ptr(ks_vertices_data), vdata_size, begin_ptr(ks_indices_data), idata_size, std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}


gfx_vpc_ring_sphere::gfx_vpc_ring_sphere() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_vpc_ring_sphere::set_dimensions(float iradius, int igrid_point_count)
{
	int gridPointsWidth = igrid_point_count;

	if(gridPointsWidth % 2 == 0)
		// the number horizontal points has to be odd, not even and > 1
	{
		gridPointsWidth++;
	}

	// the number vertical points has to be odd, not even and > 1
	int gridPointsHeight = gridPointsWidth / 2 + 1;
	int gridSizeWidth = gridPointsWidth - 1;
	int gridSizeHeight = gridPointsHeight - 1;
	float igdWidth = 1.0 / float(gridPointsWidth - 1);
	float igdHeight = 1.0 / float(gridPointsHeight - 1);
	int indicesLength = 6 * gridSizeWidth * gridSizeHeight + 6 * (gridSizeWidth - 1);
	int verticesLength = gridPointsWidth * gridPointsHeight;
	std::vector<vx_fmt_p3f_n3f_t2f> rs_vertices_data;
	std::vector<gfx_indices_type> rs_indices_data;

	rs_vertices_data.resize(verticesLength);
	rs_indices_data.resize(indicesLength);

	// form a triangle strip
	//unsigned short ind[] = {
	//	3 * 0 + 0, 3 * 0 + 1, 3 * 1 + 0,
	//	3 * 0 + 1, 3 * 1 + 0, 3 * 1 + 1,
	//	3 * 1 + 0, 3 * 1 + 1, 3 * 2 + 0,
	//	3 * 1 + 1, 3 * 2 + 0, 3 * 2 + 1,
	//	// distorted triangles
	//	3 * 2 + 0, 3 * 2 + 1, 3 * 2 + 1,
	//	3 * 2 + 1, 3 * 2 + 1, 3 * 2 + 2,
	//	// second strip
	//	3 * 2 + 1, 3 * 2 + 2, 3 * 1 + 1,
	//	3 * 2 + 2, 3 * 1 + 1, 3 * 1 + 2,
	//	3 * 1 + 1, 3 * 1 + 2, 3 * 0 + 1,
	//	3 * 1 + 2, 3 * 0 + 1, 3 * 0 + 2,
	//};
	//RNG rng;

	// set vertex and texture coordinates
	for(int h = 0; h < gridPointsHeight; h++)
	{
		int idx = gridPointsWidth * h;
		// spherical coordinates theta; theta in [0*, 180*]
		float theta = (h / float(gridPointsHeight - 1)) * glm::pi<float>();
		// spherical coordinates z is mapped as y, with y in [-1, +1]
		float y = iradius * -glm::cos(theta);
		float sintheta = glm::sin(theta);

		for(int w = 0; w < gridPointsWidth; w++, idx++)
		{
			// spherical coordinates phi; phi in [0*, 360*]
			float phi = (w / float(gridPointsWidth - 1)) * 2 * glm::pi<float>();
			//radius = rng.range(1000, 1025) / 1000.f;
			float x = iradius * glm::sin(phi) * sintheta;
			float z = iradius * glm::cos(phi) * sintheta;

			//vertexCrd[idx].x = x;
			//vertexCrd[idx].y = y;
			//vertexCrd[idx].z = z;
			//vertexCrd[idx].u = w * igdWidth;
			//vertexCrd[idx].v = h * igdHeight;
			//vertexCrd[idx].a = 255;
			//vertexCrd[idx].r = 255;
			//vertexCrd[idx].g = 255;
			//vertexCrd[idx].b = 255;
			glm::vec3 nrm(x, y, z);
			nrm = glm::normalize(nrm);
			vx_fmt_p3f_n3f_t2f vx = {{x, y, z}, {nrm.x, nrm.y, nrm.z}, {w * igdWidth, 1.f - h * igdHeight}};
			rs_vertices_data[idx] = vx;
		}
	}

	// set indices
	for(int hIdx = 0, idx = 0; hIdx < gridSizeWidth;)
	{
		// left column
		for(int vIdx = 0; vIdx < gridSizeHeight; vIdx++)
		{
			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx;
			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx + 1;
			rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx;

			rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx;
			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx + 1;
			rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx + 1;
		}

		// distorted triangles
		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx;
		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;

		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
		//rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 2;

		hIdx++;

		// right column
		for(int vIdx = gridSizeHeight; vIdx > 0; vIdx--)
		{
			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx + 1;
			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx;
			rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx;

			rs_indices_data[idx++] = gridPointsWidth *  vIdx + hIdx + 1;
			rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx;
			rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx + 1;
		}

		// distorted triangles. if this is the last column, do not continue
		//if(hIdx < gridSizeWidth - 1)
		//{
		//	rs_indices_data[idx++] = hIdx;
		//	rs_indices_data[idx++] = hIdx + 1;
		//	rs_indices_data[idx++] = hIdx + 1;

		//	rs_indices_data[idx++] = hIdx + 1;
		//	rs_indices_data[idx++] = hIdx + 1;
		//	rs_indices_data[idx++] = hIdx + 2;
		//}

		hIdx++;
	}

	int vdata_size = rs_vertices_data.size() * sizeof(vx_fmt_p3f_n3f_t2f);
	int idata_size = rs_indices_data.size() * sizeof(gfx_indices_type);
	set_mesh_data((const uint8*)begin_ptr(rs_vertices_data), vdata_size, begin_ptr(rs_indices_data), idata_size, std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
	//trx("ind length %d") % indicesLength;
}
