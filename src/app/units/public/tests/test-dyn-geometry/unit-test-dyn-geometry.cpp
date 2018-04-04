#include "stdafx.h"

#include "unit-test-dyn-geometry.hpp"

#ifdef UNIT_TEST_DYN_GEOMETRY

#include "com/unit/input-ctrl.hpp"
#include "com/mws/font-db.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-camera.hpp"
#include "com/mws/mws-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include "gfx-color.hpp"
#include "ext/gfx-surface.hpp"
#include "control-curve.hpp"
#include "natural-cubic-spline.hpp"
#include "curve-mesh.hpp"
#include <utils/free-camera.hpp>
#include <rng/rng.hpp>
#include <glm/glm.hpp>


class Gaussian
{
public:
	// return phi(x) = standard Gaussian pdf
	static float phi(float x) {
		return glm::exp(-x*x / 2) / glm::sqrt(2 * glm::pi<float>());
	}

	// return phi(x, mu, signma) = Gaussian pdf with mean mu and stddev sigma
	static float phi(float x, float mu, float sigma) {
		return phi((x - mu) / sigma) / sigma;
	}

	// return Phi(z) = standard Gaussian cdf using Taylor approximation
	static float Phi(float z) {
		if (z < -8.0) return 0.0;
		if (z > 8.0) return 1.0;
		float sum = 0.0, term = z;
		for (int i = 3; sum + term != sum; i += 2) {
			sum = sum + term;
			term = term * z * z / i;
		}
		return 0.5 + sum * phi(z);
	}

	// return Phi(z, mu, sigma) = Gaussian cdf with mean mu and stddev sigma
	static float Phi(float z, float mu, float sigma) {
		return Phi((z - mu) / sigma);
	}

	// Compute z such that Phi(z) = y via bisection search
	static float PhiInverse(float y) {
		return PhiInverse(y, .00000001, -8, 8);
	}

	// bisection search
	static float PhiInverse(float y, float delta, float lo, float hi) {
		float mid = lo + (hi - lo) / 2;
		if (hi - lo < delta) return mid;
		if (Phi(mid) > y) return PhiInverse(y, delta, lo, mid);
		else              return PhiInverse(y, delta, mid, hi);
	}
};


float clip(float x, float min, float max)
{
	if (min > max) return x;
	else if (x < min) return min;
	else if (x > max) return max;
	return x;
}

shared_ptr<std::vector<glm::vec2> > generatePolygon(float ctrX, float ctrY, float aveRadius, float irregularity, float spikeyness, int numVerts)
{
	//Start with the centre of the polygon at ctrX, ctrY, 
	//then creates the polygon by sampling points on a circle around the centre.
	//Randon noise is added by varying the angular spacing between sequential points,
	//and by varying the radial distance of each point from the centre.

	//Params :
	//	   ctrX, ctrY - coordinates of the "centre" of the polygon
	//	   aveRadius - in px, the average radius of this polygon, this roughly controls how large the polygon is, really only useful for order of magnitude.
	//	   irregularity - [0, 1] indicating how much variance there is in the angular spacing of vertices.[0, 1] will map to[0, 2pi / numberOfVerts]
	//	   spikeyness - [0, 1] indicating how much variance there is in each vertex from the circle of radius aveRadius.[0, 1] will map to[0, aveRadius]
	//	   numVerts - self - explanatory
	//
	//	   Returns a list of vertices, in CCW order.

	RNG random;
	irregularity = clip(irregularity, 0, 1) * 2 * glm::pi<float>() / numVerts;
	spikeyness = clip(spikeyness, 0, 1) * aveRadius;

	// generate n angle steps
	std::vector<float> angleSteps;
	float lower = (2 * glm::pi<float>() / numVerts) - irregularity;
	float upper = (2 * glm::pi<float>() / numVerts) + irregularity;
	float sum = 0;

	for (int i = 0; i < numVerts; i++)
	{
		float tmp = random.range_float(lower, upper);
		angleSteps.push_back(tmp);
		sum = sum + tmp;
	}

	// normalize the steps so that point 0 and point n + 1 are the same
	float k = sum / (2 * glm::pi<float>());
	for (int i = 0; i < numVerts; i++)
	{
		angleSteps[i] = angleSteps[i] / k;
	}

	// now generate the points
	shared_ptr<std::vector<glm::vec2> > point_list(new std::vector<glm::vec2>());
	std::vector<glm::vec2>& points = *point_list;
	float angle = random.range_float(0, 2 * glm::pi<float>());

	for (int i = 0; i < numVerts; i++)
	{
		float r_i = clip(random.random_gauss(aveRadius, spikeyness), 0, 2 * aveRadius);
		float x = ctrX + r_i*glm::cos(angle);
		float y = ctrY + r_i*glm::sin(angle);
		points.push_back(glm::vec2(int(x), int(y)));

		angle = angle + angleSteps[i];
	}

	return point_list;
}


class mws_select_button : public gfx_plane
{
public:
	mws_select_button(shared_ptr<unit_test_dyn_geometry> iunit, int ibutton_id, std::vector<std::string>& istate_list)
	{
		unit = iunit;
		button_id = ibutton_id;
		state_id = 0;
		state_list = istate_list;
		z_pos = -1;
		is_init = false;
	}

	shared_ptr<unit_test_dyn_geometry> get_unit()
	{
		return unit.lock();
	}

	bool is_hit(float ix, float iy)
	{
		float x_size = x_percent_size * get_unit()->get_width();
		float y_size = y_percent_size * get_unit()->get_height();
		float boxx = x_percent_pos * get_unit()->get_width();// + x_size / 2;
		float boxy = y_percent_pos * get_unit()->get_height();// + y_size / 2;
		float boxWidth = x_size;
		float boxHeight = y_size;

		return is_inside_box(ix, iy, boxx, boxy, boxWidth, boxHeight);
	}

	// percent of screen res
	void set_dim(float ix_percent_pos, float iy_percent_pos, float ix_percent_size, float iy_percent_size)
	{
		init();

		x_percent_size = ix_percent_size;
		y_percent_size = iy_percent_size;

		float x_size = x_percent_size * get_unit()->get_width();
		float y_size = y_percent_size * get_unit()->get_height();

		set_dimensions(x_size, y_size);

		x_percent_pos = ix_percent_pos;
		y_percent_pos = iy_percent_pos;

		float x_pos = x_percent_pos * get_unit()->get_width() + x_size / 2;
		float y_pos = y_percent_pos * get_unit()->get_height() + y_size / 2;

		glm::vec3 pos(x_pos, y_pos, z_pos);

		position = pos;
	}

	void next_state()
	{
		mws_select_button& inst = *static_pointer_cast<mws_select_button>(get_shared_ptr());

		state_id++;
		state_id %= state_list.size();
		inst["u_s2d_tex"] = state_list[state_id];
	}

	void init()
	{
		if (!is_init)
		{
			is_init = true;
			mws_select_button& inst = *static_pointer_cast<mws_select_button>(get_shared_ptr());

			inst[MP_SHADER_NAME] = "basic_tex";
			inst[MP_BLENDING] = MV_ALPHA;
			//inst["u_v4_color"] = ia_color::colors::dodger_blue.to_vec4();
			inst["u_s2d_tex"] = state_list[state_id];
		}
	}

	int button_id;
	int state_id;
	float z_pos;
	float x_percent_pos, y_percent_pos;
	float x_percent_size, y_percent_size;
	std::vector<std::string> state_list;
	bool is_init;
	weak_ptr<unit_test_dyn_geometry> unit;
};


class unit_test_dyn_geometry_impl
{
public:
	unit_test_dyn_geometry_impl(shared_ptr<unit_test_dyn_geometry> iunit)
	{
		unit = iunit;
		recalc_points = false;
		cm = shared_ptr<curve_mesh>(new curve_mesh());

		shared_ptr<mws_select_button> b;
		std::vector<std::string> state_list;
		float y = 0.01;
		float off = 0.14;

		state_list.push_back("wireframe_off");
		state_list.push_back("wireframe_on");
		b = shared_ptr<mws_select_button>(new mws_select_button(iunit, 0, state_list));
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("touch_points");
		state_list.push_back("ncs_points");
		b = shared_ptr<mws_select_button>(new mws_select_button(iunit, 1, state_list));
		y += off;
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("mobile");
		state_list.push_back("fixed");
		b = shared_ptr<mws_select_button>(new mws_select_button(iunit, 2, state_list));
		y += off;
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("vertex-arrays");
		state_list.push_back("vertex-buffer-objects");
		state_list.push_back("unsynchronized");
		b = shared_ptr<mws_select_button>(new mws_select_button(iunit, 3, state_list));
		y += off;
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		//point_list.push_back(Point(200, 150));
		//point_list.push_back(Point(250, 150));
		//point_list.push_back(Point(300, 150));
		//point_list.push_back(Point(300+25.5, 150));
		//point_list.push_back(Point(300+25.5+126, 150));
		//calc_points();
	}

	shared_ptr<unit_test_dyn_geometry> get_unit()
	{
		return unit.lock();
	}

	void calc_points()
	{
		cm->calc_points(point_list);
	}

	void draw_curve()
	{
		std::vector<glm::vec3>& min_dist_point_list = cm->sampled_point_list;

		if (min_dist_point_list.size() < 2)
		{
			return;
		}

		std::vector<glm::vec3>& curve_point_list = cm->curve->final_point_list;

		for (int k = 1; k < curve_point_list.size(); k++)
		{
			glm::vec3& p0 = curve_point_list[k - 1];
			glm::vec3& p1 = curve_point_list[k];

			ortho_cam->draw_line(p0, p1, gfx_color::colors::cyan.to_vec4(), 2.f);
		}

		//for (int k = 1; k < min_dist_point_list.size(); k++)
		//{
		//	glm::vec3& p0 = min_dist_point_list[k - 1];
		//	glm::vec3& p1 = min_dist_point_list[k];

		//	ortho_cam->draw_line(p0, p1, ia_color::colors::cyan.to_vec4(), 2.f);
		//}

		for (int k = 0; k < min_dist_point_list.size(); k++)
		{
			glm::vec3& p = min_dist_point_list[k];

			ortho_cam->draw_point(p, gfx_color::colors::red.to_vec4(), 5);
		}
	}

	bool process_input(control_curve_ns::Point& ipoint)
	{
		int width = get_unit()->get_width();
		int height = get_unit()->get_height();

		for (int k = 0; k < button_list.size(); k++)
		{
			shared_ptr<mws_select_button> btn = button_list[k];

			if (btn->is_hit(ipoint.x, ipoint.y))
			{
				switch (btn->button_id)
				{
				case 0:
				{
					btn->next_state();
					curve_mesh& r_cm = *cm;

					switch (btn->state_id)
					{
					case 0:
						r_cm[MP_WIREFRAME_MODE] = MV_WF_NONE;
						break;

					case 1:
						r_cm[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
						break;
					}
					break;
				}

				case 1:
				{
					btn->next_state();

					switch (btn->state_id)
					{
					case 0:
						cm->set_curve_type(curve_mesh::e_touch_points);
						break;

					case 1:
						cm->set_curve_type(curve_mesh::e_ncs_points);
						break;
					}

					break;
				}

				case 2:
				{
					btn->next_state();

					switch (btn->state_id)
					{
					case 0:
						cm->set_curve_mobility_type(curve_mesh::e_mobile);
						break;

					case 1:
						cm->set_curve_mobility_type(curve_mesh::e_fixed);
						break;
					}

					break;
				}

				case 3:
				{
					btn->next_state();

					switch (btn->state_id)
					{
					case 0:
						cm->set_geometry_update_type(curve_mesh::e_vertex_arrays);
						break;

					case 1:
						cm->set_geometry_update_type(curve_mesh::e_vertex_buffer_objects);
						break;

					case 2:
						cm->set_geometry_update_type(curve_mesh::e_unsynchronized);
						break;
					}

					break;
				}
				}

				break;
			}
		}

		return true;
	}

	shared_ptr<gfx_plane> q2d;
	shared_ptr<gfx_shader> texture_display;
	std::vector<shared_ptr<mws_select_button> > button_list;
	bool recalc_points;
	shared_ptr<curve_mesh> cm;
	std::vector<pointer_evt::touch_point> point_list;
	shared_ptr<gfx_camera> persp_cam;
	shared_ptr<mws_camera> ortho_cam;
	shared_ptr<std::vector<glm::vec2> > poly;
	weak_ptr<unit_test_dyn_geometry> unit;
};


unit_test_dyn_geometry::unit_test_dyn_geometry()
{
	set_name("test-dyn-geometry");
}

shared_ptr<unit_test_dyn_geometry> unit_test_dyn_geometry::new_instance()
{
	return shared_ptr<unit_test_dyn_geometry>(new unit_test_dyn_geometry());
}

void unit_test_dyn_geometry::init()
{
	//touch_ctrl->add_receiver(get_smtp_instance());
	//key_ctrl->add_receiver(get_smtp_instance());
}

void unit_test_dyn_geometry::load()
{
	p = shared_ptr<unit_test_dyn_geometry_impl>(new unit_test_dyn_geometry_impl(static_pointer_cast<unit_test_dyn_geometry>(get_smtp_instance())));

	float ctrX = 600;
	float ctrY = 310;
	float aveRadius = 150;
	float irregularity = 0;
	float spikeyness = 0;
	int numVerts = 6;
	p->poly = generatePolygon(ctrX, ctrY, aveRadius, irregularity, spikeyness, numVerts);

	p->persp_cam = gfx_camera::new_inst();
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 0.01f;
	p->persp_cam->far_clip_distance = 50000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;
	p->persp_cam->position = glm::vec3(0.f, 0.f, 250.f);

	p->ortho_cam = mws_camera::new_inst();
	p->ortho_cam->camera_id = "ortho_cam";
	p->ortho_cam->rendering_priority = 1;
	p->ortho_cam->projection_type = "orthographic";
	p->ortho_cam->near_clip_distance = -100;
	p->ortho_cam->far_clip_distance = 100;
	p->ortho_cam->clear_color = true;
	p->ortho_cam->clear_color_value = gfx_color::colors::black;
	p->ortho_cam->clear_depth = true;

	curve_mesh& r_cm = *p->cm;
	r_cm.camera_id_list.clear();
	r_cm.camera_id_list.push_back(p->ortho_cam->camera_id());
	r_cm[MP_SHADER_NAME] = "c_o";
	r_cm[MP_CULL_BACK] = false;
	r_cm[MP_CULL_FRONT] = false;
	r_cm["u_v4_color"] = glm::vec4(0.99, 1, 0.15, 1.f);
	r_cm[MP_WIREFRAME_MODE] = MV_WF_NONE;

	p->texture_display = gfx::shader::new_program("basic_tex", "basic_tex.vsh", "basic_tex.fsh");
	p->q2d = shared_ptr<gfx_plane>(new gfx_plane());
	gfx_plane& rq2d = *p->q2d;
	rq2d.set_dimensions(2, 2);
	rq2d[MP_CULL_BACK] = false;
	rq2d[MP_DEPTH_TEST] = false;
	rq2d[MP_BLENDING] = MV_ALPHA;
	rq2d[MP_SHADER_NAME] = "basic_tex";
	rq2d.position = glm::vec3(850.f, 350.f, 0.f);
	rq2d.scaling = glm::vec3(256, 256, 1.f);

	//gfx_scene_inst->attach(impl->persp_cam);
	gfx_scene_inst->attach(p->ortho_cam);
	gfx_scene_inst->attach(p->cm);

	for (int k = 0; k < p->button_list.size(); k++)
	{
		p->button_list[k]->camera_id_list.clear();
		p->button_list[k]->camera_id_list.push_back(p->ortho_cam->camera_id());
		gfx_scene_inst->attach(p->button_list[k]);
	}

	p->persp_cam->position = glm::vec3(0.f, 0.f, 200.f);

	mws_report_gfx_errs();
}

bool unit_test_dyn_geometry::update()
{
	if (p->recalc_points)
	{
		p->recalc_points = false;
		p->calc_points();
	}

	shared_ptr<gfx_tex> atlas = font_db::inst()->get_texture_atlas();

	if (atlas && atlas->is_valid())
	{
		//(*p->q2d)["u_s2d_tex"] = atlas->get_name();
		//mws_cam->draw_mesh(p->q2d);
	}

	std::vector<glm::vec2>& poly = *p->poly;
	for (int i = 0; i < poly.size() - 1; i++)
	{
		p->ortho_cam->draw_line(glm::vec3(poly[i], 0), glm::vec3(poly[i + 1], 0), glm::vec4(1.0), 1);
	}
	p->ortho_cam->draw_line(glm::vec3(poly[0], 0), glm::vec3(poly[poly.size() - 1], 0), glm::vec4(1.0), 1);

	//p->draw_curve();
	//p->ortho_cam->draw_line(glm::vec3(50, 50, 0), glm::vec3(150, 150, 0), glm::vec4(1.0), 1);
	//p->ortho_cam->setColor(0xffff0000);
	//p->ortho_cam->fillRect(50, 50, 150, 200);

	mws_report_gfx_errs();

	return unit::update();
}

void unit_test_dyn_geometry::receive(shared_ptr<iadp> idp)
{
	if (!idp->is_processed())
	{
		if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

			//vprint("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
			switch (ts->get_type())
			{
			case touch_sym_evt::TS_PRESS_AND_DRAG:
			{
				p->point_list.push_back(ts->crt_state.te->points[0]);
				p->recalc_points = true;
				ts->process();
				//vprint("tn %s %f %f\n", ts->get_type_name(ts->get_type()).c_str(), ts->crt_state.pos.x, ts->crt_state.pos.y);

				break;
			}

			case touch_sym_evt::TS_MOUSE_WHEEL:
			{
				shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

				break;
			}

			case touch_sym_evt::TS_PRESSED:
			{
				p->point_list.clear();
				p->point_list.push_back(ts->crt_state.te->points[0]);
				p->recalc_points = true;
				ts->process();

				break;
			}

			case touch_sym_evt::TS_RELEASED:
			{
				control_curve_ns::Point pt(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y);

				p->point_list.clear();
				p->recalc_points = false;
				p->process_input(pt);
				ts->process();

				break;
			}
			}
		}
		else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if (ke->get_type() != key_evt::KE_RELEASED)
			{
				bool isAction = true;

				switch (ke->get_key())
				{
				case KEY_Q:
				{
					break;
				}

				default:
					isAction = false;
				}

				if (!isAction && ke->get_type() != key_evt::KE_REPEATED)
				{
					isAction = true;

					switch (ke->get_key())
					{
					case KEY_R:
					{
						float ctrX = 600;
						float ctrY = 310;
						float aveRadius = 250;
						float irregularity = 1.0;
						float spikeyness = 0.1;
						int numVerts = 57;
						p->poly = generatePolygon(ctrX, ctrY, aveRadius, irregularity, spikeyness, numVerts);
						break;
					}

					default:
						isAction = false;
					}
				}

				if (isAction)
				{
					ke->process();
				}
			}
		}
	}
}

#endif
