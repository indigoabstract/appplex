#pragma once

#include "unit-test-dyn-geometry.hxx"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "pfm.hxx"
#include "com/util/util.hxx"
#include "gfx-vxo.hxx"
#include <vector>

class NatCubic;
class Point;


class curve_mesh : public gfx_vxo
{
public:
	enum curve_types
	{
		e_touch_points,
		e_ncs_points,
	};

	enum curve_mobility_types
	{
		e_mobile,
		e_fixed,
	};

	enum geometry_update_types
	{
		e_vertex_arrays,
		e_vertex_buffer_objects,
		e_unsynchronized,
	};

	curve_mesh();

	void reset();
	void set_curve_type(curve_types icurve_type);
	void set_curve_mobility_type(curve_mobility_types icurve_mobility);
	void set_geometry_update_type(geometry_update_types iupdate_type);
	void calc_points(std::vector<pointer_evt::touch_point>& ipoint_list);
	void calc_geometry();

	void sample_touch_points(int istart_idx, int iend_idx, std::vector<pointer_evt::touch_point>& ipoint_list);
	void sample_ncs_points(int istart_idx, int iend_idx, std::vector<pointer_evt::touch_point>& ipoint_list);
	virtual void draw_in_sync(mws_sp<gfx_camera> icamera);
	void draw_using_va(mws_sp<gfx_camera> icamera);
	void draw_using_vbo(mws_sp<gfx_camera> icamera);
	void draw_unsynchronized(mws_sp<gfx_camera> icamera);

	std::vector<gfx_uint> buffer_id_list;
	bool drawing_mode_changed;
	curve_types curve_type;
	curve_mobility_types curve_mobility;
	geometry_update_types geometry_update_type;
	mws_sp<NatCubic> curve;
	std::vector<pointer_evt::touch_point> point_list;
	std::vector<glm::vec3> sampled_point_list;
	std::vector<vx_fmt_p3f_t2f> vx_tab;
	std::vector<gfx_indices_type> ix_tab;
	float MIN_DIST;
};

#endif
