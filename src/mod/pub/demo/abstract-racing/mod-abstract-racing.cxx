#include "stdafx.hxx"

#include "mod-abstract-racing.hxx"
#include "mws/mws-camera.hxx"
#include "gfx-state.hxx"
#include "gfx-vxo-ext.hxx"
//#include "renderer-openvg.h"
#include "mws/mws.hxx"
//#include "renderer.h"
//#include "3d-objects.h"
#include "track.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>
#include <string>

using std::string;

mod_abstract_racing::mod_abstract_racing() : mws_mod(mws_stringify(MOD_ABSTRACT_RACING)) {}

mws_sp<mod_abstract_racing> mod_abstract_racing::nwi()
{
	return mws_sp<mod_abstract_racing>(new mod_abstract_racing());
}

namespace
{
	class main_page : public mws_page
	{
	public:
		virtual void init()
		{
			mws_page::init();

			on_resize();

			camera_tm = camera_tm * glm::rotate(180.f, glm::vec3(0.f, 1.f, 0.f));
			camera_tm = camera_tm * glm::rotate(-30.f, glm::vec3(1.f, 0.f, 0.f));
			camera_tm = camera_tm * glm::translate(glm::vec3(0.f, 0.f, 3500.f));

			duringSegmentCrossing = false;
			t = std::make_shared<track>(get_mod());
			t->loadTrackData("04_02.dat");
			t->generateTrackVertices();
			segmentIdx = t->start_point * t->interpolation_steps_count;
			crtPos = t->segment_crd[segmentIdx];
			last_time = mws::time::get_time_millis();
		}

		virtual void receive(mws_sp<mws_dp> idp)
		{
			if(idp->is_type(mws_key_evt::key_evt_type))
			{
				auto mod = get_mod();
				mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

				if(ke->get_type() != mws_key_evt::ke_released)
				{
					bool do_action = false;

					if(do_action)
					{
						mod->process(ke);
					}
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();

			int segmentCrossTime = 250;
			duringSegmentCrossing = false;
			uint32_t now = mws::time::get_time_millis();
			uint32_t delta = now - last_time;

			if(delta > segmentCrossTime)
			{
				duringSegmentCrossing = true;
				last_time = now;

				segmentIdx++;
				segmentIdx %= t->segment_crd.size();
				crtPos = t->get_segment_pos_at(segmentIdx);
			}
			else
			{
				float d = delta / segmentCrossTime;
				float md = 1 - d;
				glm::vec3 p1 = t->get_segment_pos_at(segmentIdx);
				glm::vec3 p2 = t->get_segment_pos_at(segmentIdx + 1);

				crtPos.x = p1.x * md + p2.x * d;
				crtPos.y = p1.y * md + p2.y * d;
				crtPos.z = p1.z * md + p2.z * d;
			}
		}

		virtual void update_view(mws_sp<mws_camera> i_g)
		{
			/*
			mws_sp<mod_abstract_racing> u = ar_mod();
			renderer& r = *renderer::get_instance();

			decl_scgfxpl(pl1)
			{
				{gl::DEPTH_TEST, gl::TRUE_GL}, {gl::DEPTH_WRITEMASK, gl::TRUE_GL},
				{gl::DEPTH_FUNC, gl::LESS_GL}, {gl::COLOR_CLEAR_VALUE, 1.f, 1.f, 0.9f, 1.f},
				{gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL},
				{gl::CULL_FACE, gl::TRUE_GL}, {gl::CULL_FACE_MODE, gl::BACK_GL},
				{},
			};
			r.st.set_state(pl1);

			r.mx.push_projection_matrix(camera);
			r.mx.push_view_matrix(camera_tm);


			glm::vec3 sp = t->start_pos;
			glm::vec3 v1 = crtPos;
			glm::vec3 v2 = t->get_segment_pos_at(segmentIdx + 1);
			glm::vec3 direction = v1 - v2;
			direction.y = 0;
			//direction.Normalise();
			glm::vec3 xaxis(1, 0, 0);
			float dotp = glm::dot(direction, xaxis);
			float angleRad = acosf(dotp / (glm::length(direction) * glm::length(xaxis)));
			float angleDeg = 90 - angleRad * 180.f / M_PI;

			//angleDeg = angleDeg - ((int)angleDeg / 90) * 90;
			//if(angleDeg < 0)
			//{
			//	angleDeg = -angleDeg;
			//}

			glm::vec3 carPos = crtPos;
			carPos.y += 25;
			glm::vec3 cp = carPos;
			cp.y += 50;

			t->drawTrack(&r);

			glm::mat4 tf;
			float sl = 50;
			sp.y += 35;
			tf = tf * glm::translate(glm::vec3(sp.x, sp.y, sp.z));
			tf = tf * glm::scale(glm::vec3(sl, sl, sl));

			r.tx.set_texture_id(0);
			r.mx.push_matrix(tf);
			cube1.draw(&r);
			r.mx.pop_matrix();

			if(duringSegmentCrossing)trx("%1%") % (int)angleDeg;

			// car
			sl = 30;
			tf = glm::mat4(1.f);
			tf = tf * glm::translate(glm::vec3(carPos.x, carPos.y, carPos.z));
			tf = tf * glm::rotate(angleDeg, glm::vec3(0.f, 1.f, 0.f));
			tf = tf * glm::scale(glm::vec3(sl, sl, 5.f * sl));

			r.mx.push_matrix(tf);
			cube2.draw(&r);
			r.mx.pop_matrix();


			r.mx.pop_projection_matrix();
			r.mx.pop_view_matrix();

			mws_page::update_view(i_g);
			const string& text = ar_mod()->get_name();
			i_g->drawText(text, 10, 10);
			*/
		}

		virtual void on_resize()
		{
			mws_page::on_resize();

			float cnear = 1.f;
			float cfar = 10000.0f;
			float cfovy = 45.0f;

			camera = glm::perspectiveFov(cfovy, (float)ar_mod()->get_width(), (float)ar_mod()->get_height(), cnear, cfar);
		}

		mws_sp<mod_abstract_racing> ar_mod()
		{
			return static_pointer_cast<mod_abstract_racing>(get_mod());
		}

		glm::mat4 camera;
		glm::mat4 camera_tm;
		mws_sp<track> t;
		int segmentIdx;
		glm::vec3 crtPos;
		bool duringSegmentCrossing;
		uint32_t last_time;
		mws_sp<gfx_box> cube1, cube2;
	};
}


void mod_abstract_racing::init_mws()
{
   mws_root->new_page<main_page>();
}

void mod_abstract_racing::init()
{
}

void mod_abstract_racing::load()
{
	//decl_scgfxpl(pl1)
	//{
	//	{gl::DEPTH_TEST, gl::TRUE_GL}, {gl::DEPTH_WRITEMASK, gl::TRUE_GL}, {gl::DEPTH_FUNC, gl::LESS_GL},
	//	{gl::SHADE_MODEL, gl::SMOOTH_GL}, {gl::PERSPECTIVE_CORRECTION_HINT, gl::NICEST_GL},
	//	{gl::CULL_FACE, gl::TRUE_GL}, {gl::CULL_FACE_MODE, gl::BACK_GL},
	//	{},
	//};
	//renderer::get_instance()->st.set_state(pl1);
}
