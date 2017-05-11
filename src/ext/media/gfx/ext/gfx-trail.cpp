#include "stdafx.h"

#include "gfx-trail.hpp"


gfx_trail::gfx_trail() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
	setup_tangent_basis = false;
	max_positions = 120;
	line_thickness = 150.f;
}

gfx_trail::~gfx_trail()
{
}

void gfx_trail::add_position(glm::vec3 ipos)
{
	positions.push_front(ipos);

	if (positions.size() > max_positions)
	{
		positions.pop_back();
	}

	if (positions.size() < 2)
	{
		return;
	}

	auto inst = std::static_pointer_cast<gfx_vxo>(get_shared_ptr());
	float vertices_data[] =
	{
		0, 0, 0,		0, 0, 0,	-1, 0,
		0, 0, 0,		0, 0, 0,	1, 0,
	};
	int vertices_data_size = sizeof(vertices_data) / sizeof(float);

	std::vector<float> tvertices_data;

	for (int k = 0; k < positions.size(); k++)
	{
		vertices_data[0] = vertices_data[8] = k;

		for (int i = 0; i < vertices_data_size; i++)
		{
			tvertices_data.push_back(vertices_data[i]);
		}
	}

	std::vector<uint32> tindices_data;
	int indices_size =  positions.size() - 1;
	tindices_data.resize(indices_size * 6);

	for (int k = 0, index = 0; k < indices_size; k++)
	{
		int i4 = 2 * k;
		tindices_data[index++] = i4 + 0;
		tindices_data[index++] = i4 + 1;
		tindices_data[index++] = i4 + 2;
		tindices_data[index++] = i4 + 1;
		tindices_data[index++] = i4 + 3;
		tindices_data[index++] = i4 + 2;
	}

	gfx_vxo_util::set_mesh_data((const uint8*)&tvertices_data[0], tvertices_data.size() * sizeof(float), &tindices_data[0], tindices_data.size() * sizeof(uint32), inst);

	std::vector<glm::vec3> pos_vect({ positions.begin(), positions.end() });
	(*this)["u_v3_positions"] = pos_vect;
	(*this)["u_v1_total_positions"] = (float)pos_vect.size();
	(*this)["u_v1_line_thickness"] = line_thickness;
}
