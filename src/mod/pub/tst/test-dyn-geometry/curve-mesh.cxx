#include "stdafx.hxx"

#include "mod-test-dyn-geometry.hxx"

#ifdef MOD_TEST_DYN_GEOMETRY

#include "curve-mesh.hxx"
#include "natural-cubic-spline.hxx"
#include "gfx.hxx"
#include "gfx-camera.hxx"
#include "pfm-gl.h"
#include <glm/inc.hpp>


curve_mesh::curve_mesh() : gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord"))
{
	curve = mws_sp<NatCubic>(new NatCubic());
	curve_type = e_touch_points;
	curve_mobility = e_mobile;
	geometry_update_type = e_vertex_arrays;
	drawing_mode_changed = false;
	MIN_DIST = 75;
}

void curve_mesh::reset()
{
	curve->reset();
}

void curve_mesh::set_curve_type(curve_types icurve_type)
{
	curve_type = icurve_type;
	calc_points(point_list);
}

void curve_mesh::set_curve_mobility_type(curve_mobility_types icurve_mobility)
{
	curve_mobility = icurve_mobility;
	point_list.clear();
}

void curve_mesh::set_geometry_update_type(geometry_update_types iupdate_type)
{
	geometry_update_type = iupdate_type;
	drawing_mode_changed = true;

	switch(geometry_update_type)
	{
	case e_vertex_arrays:
		break;

	case e_vertex_buffer_objects:
		break;

	case e_unsynchronized:
		break;
	}
}

void curve_mesh::calc_points(std::vector<mws_ptr_evt::touch_point>& ipoint_list)
{
	if(!ipoint_list.empty() && &point_list != &ipoint_list)
	{
		point_list = ipoint_list;
	}

	int max_points = 20;
	int end_idx = 0;
	int start_idx = 0;

	switch(curve_mobility)
	{
	case e_mobile:
		end_idx = point_list.size() - 1;
		start_idx = end_idx - std::min(end_idx, max_points);
		break;

	case e_fixed:
		end_idx = point_list.size() - 1;
		start_idx = 0;
		break;
	}

	sampled_point_list.clear();

	switch(curve_type)
	{
	case e_touch_points:
		sample_touch_points(start_idx, end_idx, point_list);
		break;

	case e_ncs_points:
		sample_ncs_points(start_idx, end_idx, point_list);
		break;
	}

	if (sampled_point_list.size() >= 2)
	{
		reset();

		for (int k = 0; k < sampled_point_list.size(); k++)
		{
			glm::vec3& p = sampled_point_list[k];

			curve->addPoint(p.x, p.y);
		}

		calc_geometry();
	}
}

void curve_mesh::calc_geometry()
{
	std::vector<glm::vec3>* p_final_point_list = 0;

	switch(curve_type)
	{
	case e_touch_points:
		p_final_point_list = &sampled_point_list;
		break;

	case e_ncs_points:
		curve->calcFinalPoints();
		p_final_point_list = &curve->final_point_list;
		break;
	}

	std::vector<glm::vec3> final_point_list = *p_final_point_list;
	float trail_hsize = 15;
	int size = final_point_list.size();
	int half_triangle_count = size - 1;

	if(size < 2)
	{
		return;
	}

	glm::vec3 up_dir = glm::vec3(0, 0, 1.f);

	vx_tab.resize(size * 2);
	ix_tab.resize(half_triangle_count * 6);

	for (int k = 0; k < size - 1; k++)
	{
		glm::vec3& p0 = final_point_list[k];
		glm::vec3& p1 = final_point_list[k + 1];
		glm::vec3 forward_dir = p1 - p0;
		glm::vec3 right_dir = glm::cross(forward_dir, up_dir);

		vx_fmt_p3f_t2f& left_vx = vx_tab[2 * k + 0];
		vx_fmt_p3f_t2f& right_vx = vx_tab[2 * k + 1];

		right_dir = glm::normalize(right_dir);

		glm::vec3 l = p0 - right_dir * trail_hsize;
		glm::vec3 r = p0 + right_dir * trail_hsize;

		left_vx.pos = l;
		right_vx.pos = r;
	}

	int k = size - 1;
	glm::vec3& p0 = final_point_list[k - 1];
	glm::vec3& p1 = final_point_list[k];
	glm::vec3 forward_dir = p1 - p0;
	glm::vec3 right_dir = glm::cross(forward_dir, up_dir);

	vx_fmt_p3f_t2f& left_vx = vx_tab[2 * k + 0];
	vx_fmt_p3f_t2f& right_vx = vx_tab[2 * k + 1];

	right_dir = glm::normalize(right_dir);

	glm::vec3 l = p1 - right_dir * trail_hsize;
	glm::vec3 r = p1 + right_dir * trail_hsize;

	left_vx.pos = l;
	right_vx.pos = r;

	for(int k = 0, index = 0; k < half_triangle_count; k++)
	{
		int i4 = 2 * k;

		ix_tab[index + 0] = i4 + 0;
		ix_tab[index + 1] = i4 + 1;
		ix_tab[index + 2] = i4 + 3;
		ix_tab[index + 3] = i4 + 2;
		ix_tab[index + 4] = i4 + 0;
		ix_tab[index + 5] = i4 + 3;
		index += 6;
	}

	int vdata_size = vx_tab.size() * sizeof(vx_fmt_p3f_t2f);
	int idata_size = ix_tab.size() * sizeof(gfx_indices_type);
	gfx_vxo_util::set_mesh_data((const uint8*)begin_ptr(vx_tab), vdata_size, begin_ptr(ix_tab), idata_size, std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
	drawing_mode_changed = true;
}

void curve_mesh::sample_touch_points(int istart_idx, int iend_idx, std::vector<mws_ptr_evt::touch_point>& ipoint_list)
{
	if(istart_idx < iend_idx && !ipoint_list.empty())
	{
		mws_ptr_evt::touch_point& p = ipoint_list[istart_idx];

		sampled_point_list.push_back(glm::vec3(p.x, p.y, 0));
	}

	for (int k = istart_idx + 1, idx = 1; k <= iend_idx; k++)
	{
		mws_ptr_evt::touch_point& p = point_list[k];

		glm::vec3& p0 = sampled_point_list[idx - 1];
		glm::vec3 p1(p.x, p.y, 0);

		//if(glm::distance(p0, p1) > MIN_DIST)
		{
			sampled_point_list.push_back(p1);
			idx++;
		}
	}

	if(istart_idx < iend_idx)
	{
		mws_ptr_evt::touch_point& last_point = ipoint_list.back();
		glm::vec3& last_point_s = sampled_point_list.back();

		if(last_point.x != last_point_s.x || last_point.y != last_point_s.y)
		{
			sampled_point_list.push_back(glm::vec3(last_point.x, last_point.y, 0));
		}
	}
}

void curve_mesh::sample_ncs_points(int istart_idx, int iend_idx, std::vector<mws_ptr_evt::touch_point>& ipoint_list)
{
	if(istart_idx < iend_idx && !ipoint_list.empty())
	{
		mws_ptr_evt::touch_point& p = ipoint_list[istart_idx];

		sampled_point_list.push_back(glm::vec3(p.x, p.y, 0));
	}

	for (int k = istart_idx + 1, idx = 0; k <= iend_idx; k++)
	{
		mws_ptr_evt::touch_point& p = point_list[k];

		glm::vec3& p0 = sampled_point_list[idx];
		glm::vec3 p1(p.x, p.y, 0);

		if(glm::distance(p0, p1) > MIN_DIST)
		{
			sampled_point_list.push_back(p1);
			idx++;
		}
	}

	if(istart_idx < iend_idx)
	{
		mws_ptr_evt::touch_point& last_point = ipoint_list.back();
		glm::vec3& last_point_s = sampled_point_list.back();

		if(last_point.x != last_point_s.x || last_point.y != last_point_s.y)
		{
			sampled_point_list.push_back(glm::vec3(last_point.x, last_point.y, 0));
		}
	}
}

void curve_mesh::draw_in_sync(mws_sp<gfx_camera> icamera)
{
	if(!visible || vertices_buffer.empty() || indices_buffer.empty())
	{
		return;
	}

	switch(geometry_update_type)
	{
	case e_vertex_arrays:
		draw_using_va(icamera);
		break;

	case e_vertex_buffer_objects:
		draw_using_vbo(icamera);
		break;

	case e_unsynchronized:
		draw_unsynchronized(icamera);
		break;
	}

	mws_report_gfx_errs();
	drawing_mode_changed = false;
}

void curve_mesh::draw_using_va(mws_sp<gfx_camera> icamera)
{
	gfx_material& mat = *get_material();
	mws_sp<gfx_shader> glp = mat.get_shader();
	wireframe_mode wf_mode = static_cast<wireframe_mode>(mat[MP_WIREFRAME_MODE].get_value<int>());
	int offset = 0;
	gfx_uint method = method_type[render_method];

	for(std::vector<mws_sp<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
	{
		mws_sp<vx_attribute> at = *it;
		gfx_int loc_idx = glp->get_param_location(at->get_name());

		if(loc_idx != -1)
		{
			bool normalized = false;
			gfx_enum gl_type = GL_FLOAT;

			switch (at->get_data_type())
			{
			case gfx_input::ivec1:
			case gfx_input::ivec2:
			case gfx_input::ivec3:
			case gfx_input::ivec4:
				normalized = true;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case gfx_input::vec1:
			case gfx_input::vec2:
			case gfx_input::vec3:
			case gfx_input::vec4:
				normalized = false;
				gl_type = GL_FLOAT;
				break;

			default:
            mws_throw mws_exception("unknown value");
			}

			glVertexAttribPointer(loc_idx, at->get_component_count(), gl_type, normalized, vxi.vertex_size, begin_ptr(vertices_buffer) + offset);
			glEnableVertexAttribArray(loc_idx);
		}

		offset += at->get_aligned_size();
	}

	glDrawElements(method, indices_buffer.size(), GL_UNSIGNED_INT, begin_ptr(indices_buffer));

	if(wf_mode == MV_WF_OVERLAY)
	{
		mws_sp<gfx_shader> p = gfx::i()->shader.get_program_by_name(gfx::wireframe_sh_id);

		gfx::i()->shader.set_current_program(p);

		if(is_submesh)
		{
			icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_parent()), p);
		}
		else
		{
			icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_mws_sp()), p);
		}

		glDrawElements(GL_LINE_STRIP, indices_buffer.size(), GL_UNSIGNED_INT, begin_ptr(indices_buffer));
		gfx::i()->shader.set_current_program(glp);
	}

	for(std::vector<mws_sp<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
	{
		gfx_int loc_idx = glp->get_param_location((*it)->get_name());

		if(loc_idx != - 1)
		{
			glDisableVertexAttribArray(loc_idx);
		}
	}
}

void curve_mesh::draw_using_vbo(mws_sp<gfx_camera> icamera)
{
	gfx_material& mat = *get_material();
	mws_sp<gfx_shader> glp = mat.get_shader();
   wireframe_mode wf_mode = static_cast<wireframe_mode>(mat[MP_WIREFRAME_MODE].get_value<int>());
	gfx_uint offset = 0;
	gfx_uint method = method_type[render_method];

	if (buffer_id_list.empty())
	{
		int max_vertices = 1000;
		int max_indices = (max_vertices / 2 - 1) * 2 * 3;

		buffer_id_list.resize(2);
		glGenBuffers(2, begin_ptr(buffer_id_list));
		glBindBuffer(GL_ARRAY_BUFFER, buffer_id_list[0]);
		glBufferData(GL_ARRAY_BUFFER, vxi.vertex_size * max_vertices, 0, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id_list[1]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gfx_indices_type) * max_indices, 0, GL_DYNAMIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffer_id_list[0]);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id_list[1]);

	if(drawing_mode_changed)
	{
		glBufferData(GL_ARRAY_BUFFER, vertices_buffer.size(), begin_ptr(vertices_buffer), GL_DYNAMIC_DRAW);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gfx_indices_type) * indices_buffer.size(), begin_ptr(indices_buffer), GL_DYNAMIC_DRAW);
	}

	for(std::vector<mws_sp<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
	{
		mws_sp<vx_attribute> at = *it;
		gfx_int loc_idx = glp->get_param_location(at->get_name());

		if(loc_idx != -1)
		{
			bool normalized = false;
			gfx_enum gl_type = GL_FLOAT;

			switch (at->get_data_type())
			{
			case gfx_input::ivec1:
			case gfx_input::ivec2:
			case gfx_input::ivec3:
			case gfx_input::ivec4:
				normalized = true;
				gl_type = GL_UNSIGNED_BYTE;
				break;

			case gfx_input::vec1:
			case gfx_input::vec2:
			case gfx_input::vec3:
			case gfx_input::vec4:
				normalized = false;
				gl_type = GL_FLOAT;
				break;

			default:
            mws_throw mws_exception("unknown value");
			}

			glVertexAttribPointer(loc_idx, at->get_component_count(), gl_type, normalized, vxi.vertex_size, (const void*)offset);
			glEnableVertexAttribArray(loc_idx);
		}

		offset += at->get_aligned_size();
	}

	glDrawElements(method, indices_buffer.size(), GL_UNSIGNED_INT, 0);

	if(wf_mode == MV_WF_OVERLAY)
	{
		mws_sp<gfx_shader> p = gfx::i()->shader.get_program_by_name(gfx::wireframe_sh_id);

		gfx::i()->shader.set_current_program(p);

		if(is_submesh)
		{
			icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_parent()), p);
		}
		else
		{
			icamera->update_glp_params(static_pointer_cast<gfx_vxo>(get_mws_sp()), p);
		}

		glDrawElements(GL_LINE_STRIP, indices_buffer.size(), GL_UNSIGNED_INT, 0);
		gfx::i()->shader.set_current_program(glp);
	}

	for(std::vector<mws_sp<vx_attribute> >::iterator it = vxi.vx_attr_vect.begin(); it != vxi.vx_attr_vect.end(); it++)
	{
		gfx_int loc_idx = glp->get_param_location((*it)->get_name());

		if(loc_idx != - 1)
		{
			glDisableVertexAttribArray(loc_idx);
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void curve_mesh::draw_unsynchronized(mws_sp<gfx_camera> icamera)
{
	draw_using_va(icamera);
}

#endif
