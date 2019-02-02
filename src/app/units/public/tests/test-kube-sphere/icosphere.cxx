#include "stdafx.hxx"

#include "unit-test-kube-sphere.hxx"

#ifdef UNIT_TEST_KUBE_SPHERE

#include "icosphere.hxx"
#include "pfm.hxx"
#include "gfx-camera.hxx"
#include "rng/rng.hxx"
#include <unordered_map>


static uint64 get_key(glm::vec3& pos)
{
	float k1 = pos.x * 1.23f + pos.y * 4.56f + pos.z * 7.89f;
	float k2 = pos.x * 9.87f + pos.y * 6.54f + pos.z * 3.21f;
	int* x = (int*)&k1;
	int* y = (int*)&k2;
	uint64 x64 = *x & 0xffffffff;
	uint64 y64 = *y & 0xffffffff;
	uint64 r = (x64 << 32) | y64;

	return r;
}


icosphere_face::icosphere_face() : gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v3_normal, a_v2_tex_coord"), true)
{
	subdiv_count = -1;
	check_numbers.push_back(6);
	check_numbers.push_back(15);
	check_numbers.push_back(45);
	check_numbers.push_back(153);
	check_numbers.push_back(561);
	check_numbers.push_back(2145);
	check_numbers.push_back(8385);

	check_ind.push_back(3);
	check_ind.push_back(12);
	check_ind.push_back(48);
	check_ind.push_back(192);
	check_ind.push_back(768);
	check_ind.push_back(3072);
	check_ind.push_back(12288);
}

int icosphere_face::get_subdiv_count(){return subdiv_count;}

void icosphere_face::gen_geometry(int isubdiv_count)
{
	if(subdiv_count == isubdiv_count)
	{
		return;
	}

	std::vector<vx_fmt_p3f_c4b_n3f_t2f> vx_data;
	std::vector<gfx_indices_type> ind_data;
	std::vector<gfx_indices_type> ind_data_new;
	std::unordered_map<uint64, uint32> vertex_map;
	gfx_color& c = face_color;
	vx_color_coord_4b clr = {c.r, c.g, c.b, c.a};

	subdiv_count = isubdiv_count;

	for (int idx = 0; idx < 3; idx++)
	{
		glm::vec3& v = corners[idx];
		vx_fmt_p3f_c4b_n3f_t2f vx = {{v.x, v.y, v.z}, clr, {}, {0, 0,}};

		vx_data.push_back(vx);
		ind_data.push_back(idx);
		vertex_map[get_key(vx_data[idx].pos)] = idx;
	}

	for(int i = 0; i < subdiv_count; i++)
	{
		ind_data_new.clear();

		for(int j = 0; j < ind_data.size(); j += 3)
		{
			glm::vec3 va = vx_data[ind_data[j + 0]].pos;
			glm::vec3 vb = vx_data[ind_data[j + 1]].pos;
			glm::vec3 vc = vx_data[ind_data[j + 2]].pos;
			int va_idx = vertex_map[get_key(va)];
			int vb_idx = vertex_map[get_key(vb)];
			int vc_idx = vertex_map[get_key(vc)];

			//mws_print("va (%f, %f, %f) ", va.x, va.y, va.z);
			//mws_print("vb (%f, %f, %f) ", vb.x, vb.y, vb.z);
			//mws_print("vc (%f, %f, %f)\n", vc.x, vc.y, vc.z);
			glm::vec3 ab = glm::vec3(va.x + vb.x, va.y + vb.y, va.z + vb.z) * 0.5f;
			glm::vec3 cb = glm::vec3(vc.x + vb.x, vc.y + vb.y, vc.z + vb.z) * 0.5f;
			glm::vec3 ac = glm::vec3(va.x + vc.x, va.y + vc.y, va.z + vc.z) * 0.5f;

			ab = glm::normalize(ab) * radius;
			cb = glm::normalize(cb) * radius;
			ac = glm::normalize(ac) * radius;

			vx_fmt_p3f_c4b_n3f_t2f vab = {{ab.x, ab.y, ab.z}, clr, {}, {0, 0,}};
			vx_fmt_p3f_c4b_n3f_t2f vcb = {{cb.x, cb.y, cb.z}, clr, {}, {0, 0,}};
			vx_fmt_p3f_c4b_n3f_t2f vac = {{ac.x, ac.y, ac.z}, clr, {}, {0, 0,}};

			uint64 r1 = get_key(vab.pos);
			uint64 r2 = get_key(vcb.pos);
			uint64 r3 = get_key(vac.pos);
			int vab_idx = -1;
			int vcb_idx = -1;
			int vac_idx = -1;

			mws_assert(r1 != r2);
			mws_assert(r1 != r3);
			mws_assert(r2 != r3);

			if (vertex_map.find(r1) == vertex_map.end())
			{
				vx_data.push_back(vab);
				vab_idx = vx_data.size() - 1;
				vertex_map[r1] = vab_idx;
			}
			else
			{
				vab_idx = vertex_map[r1];
			}

			if (vertex_map.find(r2) == vertex_map.end())
			{
				vx_data.push_back(vcb);
				vcb_idx = vx_data.size() - 1;
				vertex_map[r2] = vcb_idx;
			}
			else
			{
				vcb_idx = vertex_map[r2];
			}

			if (vertex_map.find(r3) == vertex_map.end())
			{
				vx_data.push_back(vac);
				vac_idx = vx_data.size() - 1;
				vertex_map[r3] = vac_idx;
			}
			else
			{
				vac_idx = vertex_map[r3];
			}

			ind_data_new.push_back(va_idx);
			ind_data_new.push_back(vab_idx);
			ind_data_new.push_back(vac_idx);

			ind_data_new.push_back(vab_idx);
			ind_data_new.push_back(vb_idx);
			ind_data_new.push_back(vcb_idx);

			ind_data_new.push_back(vab_idx);
			ind_data_new.push_back(vcb_idx);
			ind_data_new.push_back(vac_idx);

			ind_data_new.push_back(vac_idx);
			ind_data_new.push_back(vcb_idx);
			ind_data_new.push_back(vc_idx);
		}

		int limit = check_numbers[i];

		mws_assert(vertex_map.size() == limit);
		mws_assert(vx_data.size() == limit);
		ind_data = ind_data_new;
	}

	int vdata_size = vx_data.size() * sizeof(vx_fmt_p3f_c4b_n3f_t2f);
	int idata_size = ind_data.size() * sizeof(gfx_indices_type);
	gfx_vxo_util::set_mesh_data((const uint8*)begin_ptr(vx_data), vdata_size, begin_ptr(ind_data), idata_size, static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


icosphere::icosphere() : gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v3_normal, a_v2_tex_coord"))
{
}

void icosphere::update()
{
	std::vector<mws_sp<gfx_node> >::iterator it = children.begin();
	mws_sp<gfx_node> n = get_root()->find_node_by_name("defcam");
	mws_sp<gfx_camera> camera;

	if(n)
	{
		camera = static_pointer_cast<gfx_camera>(n);
	}

	for (; it != children.end(); it++)
	{
		(*it)->update();

		if(camera)
		{
			mws_sp<icosphere_face> face = static_pointer_cast<icosphere_face>(*it);
			camera->draw_line(position, face->face_normal * radius * 1.5f, glm::vec4(1, 0, 0.f, 1.f), 1.f);

			//for (int k = 0; k < 3; k++)
			//{
			//	camera->draw_line(position, face->corner_normals[k] * radius * 1.5f, ia_color::colors::cyan.to_vec4(), 1.f);
			//}
		}
	}
}

int icosphere::get_face_count()
{
	return children.size();
}

mws_sp<icosphere_face> icosphere::get_face_at(int iidx)
{
	mws_sp<gfx_node> n = children[iidx];

	return static_pointer_cast<icosphere_face>(n);
}

void icosphere::set_dimensions(float iradius, int isubdiv_count)
{
	float size = 1.f;
	float t = (1.f + glm::sqrt(5.f)) / 2.f;
	radius = iradius;

	glm::vec3 tvertices_data[] =
	{
		glm::vec3(-size, t*size, 0),
		glm::vec3(size, t*size, 0),
		glm::vec3(-size, -t*size, 0),
		glm::vec3(size, -t*size, 0),

		glm::vec3(0, -size, t*size),
		glm::vec3(0, size, t*size),
		glm::vec3(0, -size, -t*size),
		glm::vec3(0, size, -t*size),

		glm::vec3(t*size, 0, -size),
		glm::vec3(t*size, 0, size),
		glm::vec3(-t*size, 0, -size),
		glm::vec3(-t*size, 0, size),
	};
	glm::vec3 vertices_normals[12];

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

	face_colors.clear();
	face_colors.push_back(gfx_color::colors::blue);
	face_colors.push_back(gfx_color::colors::blue_violet);
	face_colors.push_back(gfx_color::colors::cyan);
	face_colors.push_back(gfx_color::colors::dark_orange);
	face_colors.push_back(gfx_color::colors::yellow);
	face_colors.push_back(gfx_color::colors::dark_red);
	face_colors.push_back(gfx_color::colors::deep_pink);
	face_colors.push_back(gfx_color::colors::dodger_blue);
	face_colors.push_back(gfx_color::colors::gold);
	face_colors.push_back(gfx_color::colors::gray);
	face_colors.push_back(gfx_color::colors::green);
	face_colors.push_back(gfx_color::colors::indigo);
	face_colors.push_back(gfx_color::colors::lavender);
	face_colors.push_back(gfx_color::colors::magenta);
	face_colors.push_back(gfx_color::colors::orange);
	face_colors.push_back(gfx_color::colors::orchid);
	face_colors.push_back(gfx_color::colors::papaya_whip);
	face_colors.push_back(gfx_color::colors::pink);
	face_colors.push_back(gfx_color::colors::plum);
	face_colors.push_back(gfx_color::colors::red);

	for (int k = 0; k < 12; k++)
	{
		glm::vec3& p = tvertices_data[k];

		p = glm::normalize(p);
		vertices_normals[k] = p;
		p *= iradius;
	}

	RNG rng;

	for (int k = 0; k < 20; k++)
	{
		mws_sp<icosphere_face> face(new icosphere_face());

		face->middle_point = face->face_normal = glm::vec3(0.f);

		for (int idx = 0; idx < 3; idx++)
		{
			int v_idx = tindices_data[3 * k + idx];
			glm::vec3 v = tvertices_data[v_idx];
			face->corners[idx] = v;
			face->corner_normals[idx] = vertices_normals[v_idx];
			face->face_normal += face->corner_normals[idx];
			face->middle_point += face->corners[idx];
		}

		isubdiv_count = 0;//rng.range(2, 7);
		face->radius = iradius;
		face->face_color = face_colors[k];
		face->face_normal = glm::normalize(face->face_normal / 3.f);
		face->middle_point = face->face_normal * iradius;
		face->gen_geometry(isubdiv_count);
		attach(face);
	}
}

#endif
