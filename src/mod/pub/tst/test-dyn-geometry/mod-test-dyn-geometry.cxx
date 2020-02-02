#include "stdafx.hxx"

#include "mod-test-dyn-geometry.hxx"
#include "input/input-ctrl.hxx"
#include "input/gesture-detectors.hxx"
#include "fonts/mws-font-db.hxx"
#include "gfx.hxx"
#include "gfx-rt.hxx"
#include "gfx-camera.hxx"
#include "mws/mws-camera.hxx"
#include "gfx-shader.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-vxo.hxx"
#include "gfx-state.hxx"
#include "gfx-color.hxx"
#include "gfx-vxo-ext.hxx"
#include "control-curve.hxx"
#include "natural-cubic-spline.hxx"
#include "curve-mesh.hxx"
#include "util/free-camera.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>


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

mws_sp<std::vector<glm::vec2> > generatePolygon(float ctrX, float ctrY, float aveRadius, float irregularity, float spikeyness, int numVerts)
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
	mws_sp<std::vector<glm::vec2> > point_list(new std::vector<glm::vec2>());
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
	mws_select_button(mws_sp<mod_test_dyn_geometry> i_mod, int ibutton_id, std::vector<std::string>& istate_list)
	{
		mws_mod = i_mod;
		button_id = ibutton_id;
		state_id = 0;
		state_list = istate_list;
		z_pos = -1;
		is_init = false;
	}

	mws_sp<mod_test_dyn_geometry> get_mod()
	{
		return mws_mod.lock();
	}

	bool is_hit(float ix, float iy)
	{
		float x_size = x_percent_size * get_mod()->get_width();
		float y_size = y_percent_size * get_mod()->get_height();
		float boxx = x_percent_pos * get_mod()->get_width();// + x_size / 2;
		float boxy = y_percent_pos * get_mod()->get_height();// + y_size / 2;
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

		float x_size = x_percent_size * get_mod()->get_width();
		float y_size = y_percent_size * get_mod()->get_height();

		set_dimensions(x_size, y_size);

		x_percent_pos = ix_percent_pos;
		y_percent_pos = iy_percent_pos;

		float x_pos = x_percent_pos * get_mod()->get_width() + x_size / 2;
		float y_pos = y_percent_pos * get_mod()->get_height() + y_size / 2;

		glm::vec3 pos(x_pos, y_pos, z_pos);

		position = pos;
	}

	void next_state()
	{
		mws_select_button& inst = *static_pointer_cast<mws_select_button>(get_mws_sp());

		state_id++;
		state_id %= state_list.size();
		inst["u_s2d_tex"] = state_list[state_id];
	}

	void init()
	{
		if (!is_init)
		{
			is_init = true;
			mws_select_button& inst = *static_pointer_cast<mws_select_button>(get_mws_sp());

			inst[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
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
	mws_wp<mod_test_dyn_geometry> mws_mod;
};


class mod_test_dyn_geometry_impl
{
public:
	mod_test_dyn_geometry_impl(mws_sp<mod_test_dyn_geometry> i_mod)
	{
		mws_mod = i_mod;
		recalc_points = false;
		cm = mws_sp<curve_mesh>(new curve_mesh());

		mws_sp<mws_select_button> b;
		std::vector<std::string> state_list;
		float y = 0.01;
		float off = 0.14;

		state_list.push_back("wireframe_off");
		state_list.push_back("wireframe_on");
		b = mws_sp<mws_select_button>(new mws_select_button(i_mod, 0, state_list));
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("touch_points");
		state_list.push_back("ncs_points");
		b = mws_sp<mws_select_button>(new mws_select_button(i_mod, 1, state_list));
		y += off;
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("mobile");
		state_list.push_back("fixed");
		b = mws_sp<mws_select_button>(new mws_select_button(i_mod, 2, state_list));
		y += off;
		b->set_dim(0.01, y, 0.2, 0.1);
		button_list.push_back(b);

		state_list.clear();
		state_list.push_back("vertex-arrays");
		state_list.push_back("vertex-buffer-objects");
		state_list.push_back("unsynchronized");
		b = mws_sp<mws_select_button>(new mws_select_button(i_mod, 3, state_list));
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

	mws_sp<mod_test_dyn_geometry> get_mod()
	{
		return mws_mod.lock();
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
		int width = get_mod()->get_width();
		int height = get_mod()->get_height();

		for (int k = 0; k < button_list.size(); k++)
		{
			mws_sp<mws_select_button> btn = button_list[k];

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

	dragging_detector dragging_det;
	mws_sp<gfx_plane> q2d;
	mws_sp<gfx_shader> texture_display;
	std::vector<mws_sp<mws_select_button> > button_list;
	bool recalc_points;
	mws_sp<curve_mesh> cm;
	std::vector<mws_ptr_evt::touch_point> point_list;
	mws_sp<gfx_camera> persp_cam;
	mws_sp<mws_camera> ortho_cam;
	mws_sp<std::vector<glm::vec2> > poly;
	mws_wp<mod_test_dyn_geometry> mws_mod;
};


mod_test_dyn_geometry::mod_test_dyn_geometry() : mws_mod(mws_stringify(MOD_TEST_DYN_GEOMETRY)) {}

mws_sp<mod_test_dyn_geometry> mod_test_dyn_geometry::nwi()
{
	return mws_sp<mod_test_dyn_geometry>(new mod_test_dyn_geometry());
}

void mod_test_dyn_geometry::init()
{
	//touch_ctrl_inst->add_receiver(get_smtp_instance());
	//key_ctrl_inst->add_receiver(get_smtp_instance());
}

void mod_test_dyn_geometry::load()
{
	p = mws_sp<mod_test_dyn_geometry_impl>(new mod_test_dyn_geometry_impl(static_pointer_cast<mod_test_dyn_geometry>(get_smtp_instance())));

	float ctrX = 600;
	float ctrY = 310;
	float aveRadius = 150;
	float irregularity = 0;
	float spikeyness = 0;
	int numVerts = 6;
	p->poly = generatePolygon(ctrX, ctrY, aveRadius, irregularity, spikeyness, numVerts);

	p->persp_cam = gfx_camera::nwi();
	p->persp_cam->camera_id = "default";
	p->persp_cam->rendering_priority = 0;
	p->persp_cam->near_clip_distance = 0.01f;
	p->persp_cam->far_clip_distance = 50000.f;
	p->persp_cam->fov_y_deg = 60.f;
	p->persp_cam->clear_color = true;
	p->persp_cam->clear_color_value = gfx_color::colors::black;
	p->persp_cam->clear_depth = true;
	p->persp_cam->position = glm::vec3(0.f, 0.f, 250.f);

	p->ortho_cam = mws_camera::nwi();
	p->ortho_cam->camera_id = "ortho_cam";
	p->ortho_cam->rendering_priority = 1;
	p->ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
	p->ortho_cam->near_clip_distance = -100;
	p->ortho_cam->far_clip_distance = 100;
	p->ortho_cam->clear_color = true;
	p->ortho_cam->clear_color_value = gfx_color::colors::black;
	p->ortho_cam->clear_depth = true;

	curve_mesh& r_cm = *p->cm;
	r_cm.camera_id_list.clear();
	r_cm.camera_id_list.push_back(p->ortho_cam->camera_id());
	r_cm[MP_SHADER_NAME] = gfx::c_o_sh_id;
	r_cm[MP_CULL_BACK] = false;
	r_cm[MP_CULL_FRONT] = false;
	r_cm["u_v4_color"] = glm::vec4(0.99, 1, 0.15, 1.f);
	r_cm[MP_WIREFRAME_MODE] = MV_WF_NONE;

	p->texture_display = gfx::i()->shader.get_program_by_name(gfx::basic_tex_sh_id);
	p->q2d = mws_sp<gfx_plane>(new gfx_plane());
	gfx_plane& rq2d = *p->q2d;
	rq2d.set_dimensions(2, 2);
	rq2d[MP_CULL_BACK] = false;
	rq2d[MP_DEPTH_TEST] = false;
	rq2d[MP_BLENDING] = MV_ALPHA;
	rq2d[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
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

bool mod_test_dyn_geometry::update()
{
	if (p->recalc_points)
	{
		p->recalc_points = false;
		p->calc_points();
	}

	mws_sp<gfx_tex> atlas = mws_font_db::inst()->get_texture_atlas();

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

	return mws_mod::update();
}

void mod_test_dyn_geometry::receive(mws_sp<mws_dp> idp)
{
	if (!idp->is_processed())
	{
		if (idp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
		{
			mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);
			bool is_dragging = p->dragging_det.detect_helper(ts);

			if (is_dragging)
			{
            p->point_list.push_back(ts->points[0]);
            p->recalc_points = true;
            //mws_print("tn %s %f %f\n", ts->get_type_name(ts->get_type()).c_str(), ts->crt_state.pos.x, ts->crt_state.pos.y);
			}

			//mws_print("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
			switch (ts->type)
			{
			case mws_ptr_evt::touch_began:
			{
				p->point_list.clear();
				p->point_list.push_back(ts->points[0]);
				p->recalc_points = true;
				process(ts);
				break;
			}

			case mws_ptr_evt::touch_ended:
			{
				control_curve_ns::Point pt(ts->points[0].x, ts->points[0].y);

				p->point_list.clear();
				p->recalc_points = false;
				p->process_input(pt);
				process(ts);
				break;
			}
			}
		}
		else if (idp->is_type(mws_key_evt::KEYEVT_EVT_TYPE))
		{
			mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

			if (ke->get_type() != mws_key_evt::KE_RELEASED)
			{
				bool do_action = true;

				switch (ke->get_key())
				{
				case KEY_Q:
				{
					break;
				}

				default:
					do_action = false;
				}

				if (!do_action && ke->get_type() != mws_key_evt::KE_REPEATED)
				{
					do_action = true;

					switch (ke->get_key())
					{
					case KEY_R:
					{
						float ctr_x = 600;
						float ctr_y = 310;
						float avg_radius = 250;
						float irregularity = 1.0;
						float spikeyness = 0.1;
						int num_verts = 57;
						p->poly = generatePolygon(ctr_x, ctr_y, avg_radius, irregularity, spikeyness, num_verts);
						break;
					}

					default:
						do_action = false;
					}
				}

				if (do_action)
				{
					process(ke);
				}
			}
		}
	}
}
