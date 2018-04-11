#include "stdafx.h"

#include "unit-exercise.hpp"

#ifdef UNIT_EXERCISE

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "com/unit/transitions.hpp"

using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::time_duration;
using boost::posix_time::milliseconds;

namespace unit_exercise_impl
{
	void draw_bar(shared_ptr<mws_camera> g, int x, int y, int w, int h, int color)
	{
		//gfx_ubyte ca[] = { 15, 255, 205, 0 };
		//ia_color cl(color);
		//int cr = cl.r;
		//int cg = cl.g;
		//int cb = cl.b;

		//vxfmt_v2fc4b va[] =
		//{
		//	{ x, y, cr, cg, cb, ca[0], },
		//	{ x + w, y, cr, cg, cb, ca[1], },
		//	{ x, y + h, cr, cg, cb, ca[3], },
		//	{ x + w, y + h, cr, cg, cb, ca[2], },
		//};

		//g->fillAlphaGradientQuad(va);
	}
}
using namespace unit_exercise_impl;


void unit_exercise::init_mws()
{
	class expage : public mws_page
	{
	public:
		expage(shared_ptr<mws_page_tab> iu) : mws_page(iu)
		{
			d = total = milliseconds(0);
			runCount = 0;
		}

		void on_hide_transition(const shared_ptr<linear_transition> itransition)
		{
			if (itransition->get_position() == 0 && !mst->is_finished() && !mst->is_paused())
			{
				mst->pause();
			}
		}

		shared_ptr<ms_linear_transition> getTransition()
		{
			return mst;
		}

	protected:
		shared_ptr<ms_linear_transition> mst;
		time_duration d;
		time_duration total;
		int runCount;
	};


	class mainpage : public mws_page
	{
	public:
		mainpage(shared_ptr<mws_page_tab> ipt) : mws_page(ipt){ is_init_pages = false; }

		virtual void init()
		{
			struct breset : mws_button
			{
				breset(shared_ptr<mws_page> p) : mws_button(p){}
				void on_click()
				{
					mainpage* m = (mainpage*)get_parent().get();
					m->reset();
				}
			};

			shared_ptr<mws_button> b1 = mws_button::new_shared_instance(new breset(get_mws_page_instance()));
			b1->init(mws_rect(100, 300, 200, 100), 0x8200b4, "reset");
			reset();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(transition_evt::TRANSITION_EVT_TYPE))
			{
				bool record = false;
				const std::string& evtName = idp->get_name();
				shared_ptr<ia_sender> source = idp->source();

				if (evtName == transition_evt::TRANSITION_STOPPED || evtName == transition_evt::TRANSITION_FINISHED)
				{
					record = true;
				}

				if (source == t1.lock())
				{
					d1 = t1.lock()->get_elapsed_time();

					if (record)
					{
						total1 += d1;
						d1 = 0;
						runCount1++;
					}
				}
				else if (source == t2.lock())
				{
					d2 = t2.lock()->get_elapsed_time();

					if (record)
					{
						total2 += d2;
						d2 = 0;
						runCount2++;
					}
				}
				else if (source == t3.lock())
				{
					d3 = t3.lock()->get_elapsed_time();

					if (record)
					{
						total3 += d3;
						d3 = 0;
						runCount3++;
					}
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();

			if (!is_init_pages)
			{
				init_pages();
			}
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			int fh = 16;
			int fy = 0;
			int uw = get_unit()->get_width();
			int uh = get_unit()->get_height();
			float bh = 10.7;
			uint8 a1 = 60;
			uint8 a2 = 225;
			int color = 0x5050ff;
			float t = 3;

			int xw = get_unit()->get_width() / 3;
			int xh = get_unit()->get_height() / 3;
			draw_bar(g, xw / 2, xh / 2, xw, xh, 0x8200b4);

			time_duration td1 = milliseconds(total1 + d1);
			time_duration td2 = milliseconds(total2 + d2);
			time_duration td3 = milliseconds(total3 + d3);
			time_duration td = td1 + td2 + td3;
			float ts = 2;

			fy = fh;

			g->drawText("total", 10, fy += fh);
			g->drawText(trs("exercitiul [1]   {0}x	[{1} m, {2} s]", runCount1, td1.minutes(), td1.seconds()), 10, fy += fh);
			g->drawText(trs("exercitiul [2]   {0}x	[{1} m, {2} s]", runCount2, td2.minutes(), td2.seconds()), 10, fy += fh);
			g->drawText(trs("exercitiul [3]   {0}x	[{1} m, {2} s]", runCount3, td3.minutes(), td3.seconds()), 10, fy += fh);
			g->drawText(trs("toate exercitiile  [{0} m, {1} s]", td.minutes(), td.seconds()), 10, fy += fh);
		}

	private:
		void reset()
		{
			d1 = d2 = d3 = 0;
			total1 = total2 = total3 = 0;
			runCount1 = runCount2 = runCount3 = 0;
		}

		void init_pages()
		{
			shared_ptr<mws_page_tab> ptab = get_mws_page_parent();
			expage* ep1 = (expage*)ptab->get_page_at(1).get();
			expage* ep2 = (expage*)ptab->get_page_at(2).get();
			expage* ep3 = (expage*)ptab->get_page_at(3).get();

			if (ep1 && ep2 && ep3)
			{
				t1 = ep1->getTransition();
				t2 = ep2->getTransition();
				t3 = ep3->getTransition();

				t1.lock()->add_receiver(get_instance());
				t2.lock()->add_receiver(get_instance());
				t3.lock()->add_receiver(get_instance());
				is_init_pages = true;
			}
		}

		weak_ptr<ms_linear_transition> t1;
		weak_ptr<ms_linear_transition> t2;
		weak_ptr<ms_linear_transition> t3;
		int d1;
		int d2;
		int d3;
		int total1;
		int total2;
		int total3;
		int runCount1;
		int runCount2;
		int runCount3;
		bool is_init_pages;
	};


	class ex1page : public expage
	{
	public:
		enum extype{ type1, type2, };
		ex1page(shared_ptr<mws_page_tab> iu, extype iExtype) : expage(iu), et(iExtype){}

		virtual void init()
		{
			std::vector<int> tduration;
			std::vector<shared_ptr<linear_transition> > tr;

			switch (et)
			{
			case type1:
			{
				int ttduration[] = { 500, 1500, 1500, 500 };
				float lim[] = { 0, 0, 1, 0, 0 };

				for(int td : ttduration)
				{
					tduration.push_back(td);
				}

				for(int tl : lim)
				{
					limits.push_back(tl);
				}

				break;
			}

			case type2:
			{
				int ttduration[] = { 500, 2000, 500, 15000, 500 };
				float lim[] = { 0, 0, 1, 1, 0, 0 };

            for (int td : ttduration)
				{
					tduration.push_back(td);
				}

            for (int tl : lim)
				{
					limits.push_back(tl);
				}

				break;
			}
			}

			for (int k = 0; k < 5; k++)
			{
				shared_ptr<linear_transition> tt = ms_linear_transition::new_instance(ms_transition_data::new_duration_data(tduration));
				tr.push_back(tt);
			}

			mst = ms_linear_transition::new_instance(ms_transition_data::new_transition_data(tr));


			// ui

			struct bstart : mws_button
			{
				bstart(shared_ptr<mws_page> p) : mws_button(p){}

				virtual void on_click()
				{
					ex1page* p = (ex1page*)get_parent().get();

					active = !active;

					if (active)
					{
						set_text("stop");
						p->mst->start();
					}
					else
					{
						p->mst->stop();
						set_text("start");
					}
				}

				virtual void receive(shared_ptr<iadp> idp)
				{
					mws_button::receive(idp);

					if (idp->is_processed() || !idp->is_type(transition_evt::TRANSITION_EVT_TYPE))
					{
						return;
					}

					ex1page* p = (ex1page*)get_parent().get();
					const std::string& evtName = idp->get_name();

					p->d = milliseconds(p->mst->get_elapsed_time());

					if (evtName == transition_evt::TRANSITION_STOPPED || evtName == transition_evt::TRANSITION_FINISHED)
					{
						p->d = milliseconds(p->mst->get_elapsed_time());
						p->total += p->d;
						p->d = milliseconds(0);
						p->runCount++;

						set_text("start");
						active = false;
					}
				}

				bool active;
			};

			struct bpause : mws_button
			{
				bpause(shared_ptr<mws_page> p) : mws_button(p){ enabled = false; }

				virtual void on_click()
				{
					ex1page* p = (ex1page*)get_parent().get();

					if (isPaused)
					{
						p->mst->resume();
					}
					else
					{
						p->mst->pause();
					}
				}

				virtual void receive(shared_ptr<iadp> idp)
				{
					mws_button::receive(idp);

					if (idp->is_processed() || !idp->is_type(transition_evt::TRANSITION_EVT_TYPE))
					{
						return;
					}

					ex1page* p = (ex1page*)get_parent().get();
					const std::string& evtName = idp->get_name();

					if (evtName == transition_evt::TRANSITION_STARTED || evtName == transition_evt::TRANSITION_RESUMED)
					{
						set_text("pauza");
						enabled = true;
						isPaused = false;
					}
					else if (evtName == transition_evt::TRANSITION_PAUSED)
					{
						set_text("continua");
						isPaused = true;
					}
					else if (evtName == transition_evt::TRANSITION_STOPPED || evtName == transition_evt::TRANSITION_FINISHED)
					{
						enabled = false;
					}
				}

				bool isPaused;
			};

			struct breset : mws_button
			{
				breset(shared_ptr<mws_page> p) : mws_button(p){}

				void on_click()
				{
					ex1page* p = (ex1page*)get_parent().get();

					p->d = milliseconds(0);
					p->total = milliseconds(0);
					p->runCount = 0;
				}
			};

			shared_ptr<mws_button> b;

			b = mws_button::new_shared_instance(new bstart(get_mws_page_instance()));
			b->init(mws_rect(100, 300, 200, 100), 0x8200b4, "start");
			mst->add_receiver(b);

			b = mws_button::new_shared_instance(new bpause(get_mws_page_instance()));
			b->init(mws_rect(get_unit()->get_width() - 250, 300, 200, 100), 0x8200b4, "n/a");
			mst->add_receiver(b);

			b = mws_button::new_shared_instance(new breset(get_mws_page_instance()));
			b->init(mws_rect(get_unit()->get_width() - 150, 50, 100, 50), 0x8200b4, "reset");
		}

		virtual void update_state()
		{
			expage::update_state();

			mst->update();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			expage::update_view(g);

			int fh = 16;
			int fy = 0;
			int uw = get_unit()->get_width();
			int uh = get_unit()->get_height();

			if (!mst->is_finished())
			{
				float tbw = uw * interpolate_smooth_step(mst->get_position(), 0, 1, 0);
				draw_bar(g, 10, 10, tbw, 20, 0xffff00);

				for (int k = 0; k <= mst->get_interval_index(); k++)
				{
					shared_ptr<ms_linear_transition> tr = static_pointer_cast<ms_linear_transition>(mst->get_transition_at(k));
					int iidx = tr->get_interval_index();
					shared_ptr<linear_transition> tr2 = tr->get_transition_at(iidx);
					float bw1 = uw * interpolate_smooth_step(tr2->get_position(), limits[iidx], limits[iidx + 1], 1);
					float bw2 = uw * interpolate_smooth_step(tr->get_position(), 0, 1, 0);

					draw_bar(g, 10, 30 + k * 80, bw1, 50, 0x8200b4);
					draw_bar(g, 10, 30 + 50 + k * 80, bw2, 20, 0xffff00);
				}
			}

			std::string text = (et == type1) ? "exercitiul [1]" : "exercitiul [2]";
			time_duration td = total + d;

			float ts = 2;

			fy = fh;

			g->drawText(text, 10, fy += fh);
			g->drawText(trs("numar de exercitii efectuate [{}]", runCount), 10, fy += fh);
			g->drawText(trs("durata exercitiu curent [{0} s, {1} ms]", d.seconds(), (d.total_milliseconds() % 1000)), 10, fy += fh);
			g->drawText(trs("durata totala [{0} m, {1} s]", td.minutes(), td.seconds()), 10, fy += fh);
		}

	private:
		std::vector<float> limits;
		extype et;
	};


	class ex3page : public expage
	{
	public:
		ex3page(shared_ptr<mws_page_tab> iu) : expage(iu){}

		virtual void init()
		{
			int tduration[] = { 500, 2000, 8000, 4000, 500 };
			int tdurationLength = sizeof(tduration) / sizeof(int);
			float t1lim[] = { 0, 0, 1, 1, 0, 0 };
			float t2lim[] = { 0, 0, 0, 1, 0, 0 };

			std::vector<shared_ptr<linear_transition> > tr;

			for (int k = 0; k < 5; k++)
			{
				shared_ptr<linear_transition> tt = ms_linear_transition::new_instance(ms_transition_data::new_duration_data(tduration, tdurationLength));
				tr.push_back(tt);
			}

			mst = ms_linear_transition::new_instance(ms_transition_data::new_transition_data(tr));

         for (float tl : t1lim)
			{
				limits1.push_back(tl);
			}

         for (float tl : t2lim)
			{
				limits2.push_back(tl);
			}

			// ui

			struct bstart : mws_button
			{
				bstart(shared_ptr<mws_page> p) : mws_button(p){}
				virtual void on_click()
				{
					ex3page* p = (ex3page*)get_parent().get();

					active = !active;

					if (active)
					{
						set_text("stop");
						p->mst->start();
					}
					else
					{
						p->mst->stop();
						set_text("start");
					}
				}

				virtual void receive(shared_ptr<iadp> idp)
				{
					mws_button::receive(idp);

					if (idp->is_processed() || !idp->is_type(transition_evt::TRANSITION_EVT_TYPE))
					{
						return;
					}

					ex3page* p = (ex3page*)get_parent().get();
					const std::string& evtName = idp->get_name();

					p->d = milliseconds(p->mst->get_elapsed_time());

					if (evtName == transition_evt::TRANSITION_STOPPED || evtName == transition_evt::TRANSITION_FINISHED)
					{
						p->d = milliseconds(p->mst->get_elapsed_time());
						p->total += p->d;
						p->d = milliseconds(0);
						p->runCount++;

						set_text("start");
						active = false;
					}
				}

				bool active;
			};

			struct bpause : mws_button
			{
				bpause(shared_ptr<mws_page> p) : mws_button(p){ enabled = false; }

				virtual void on_click()
				{
					ex3page* p = (ex3page*)get_parent().get();

					if (isPaused)
					{
						p->mst->resume();
					}
					else
					{
						p->mst->pause();
					}
				}

				virtual void receive(shared_ptr<iadp> idp)
				{
					mws_button::receive(idp);

					if (idp->is_processed() || !idp->is_type(transition_evt::TRANSITION_EVT_TYPE))
					{
						return;
					}

					ex3page* p = (ex3page*)get_parent().get();
					const std::string& evtName = idp->get_name();

					if (evtName == transition_evt::TRANSITION_STARTED || evtName == transition_evt::TRANSITION_RESUMED)
					{
						set_text("pauza");
						enabled = true;
						isPaused = false;
					}
					else if (evtName == transition_evt::TRANSITION_PAUSED)
					{
						set_text("continua");
						isPaused = true;
					}
					else if (evtName == transition_evt::TRANSITION_STOPPED || evtName == transition_evt::TRANSITION_FINISHED)
					{
						enabled = false;
					}
				}

				bool isPaused;
			};

			struct breset : mws_button
			{
				breset(shared_ptr<mws_page> p) : mws_button(p){}

				void on_click()
				{
					ex3page* p = (ex3page*)get_parent().get();

					p->d = milliseconds(0);
					p->total = milliseconds(0);
					p->runCount = 0;
				}
			};

			shared_ptr<mws_button> b;

			b = mws_button::new_shared_instance(new bstart(get_mws_page_instance()));
			b->init(mws_rect(100, 300, 200, 100), 0x8200b4, "start");
			mst->add_receiver(b);

			b = mws_button::new_shared_instance(new bpause(get_mws_page_instance()));
			b->init(mws_rect(get_unit()->get_width() - 250, 300, 200, 100), 0x8200b4, "n/a");
			mst->add_receiver(b);

			b = mws_button::new_shared_instance(new breset(get_mws_page_instance()));
			b->init(mws_rect(get_unit()->get_width() - 150, 50, 100, 50), 0x8200b4, "reset");
		}

		virtual void update_state()
		{
			expage::update_state();

			mst->update();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			expage::update_view(g);

			int fh = 16;
			int fy = 0;
			int uw = get_unit()->get_width();
			int uh = get_unit()->get_height();

			if (!mst->is_finished())
			{
				float tbw = uw * interpolate_smooth_step(mst->get_position(), 0, 1, 0);
				draw_bar(g, 10, 10, tbw, 20, 0xffff00);

				for (int k = 0; k <= mst->get_interval_index(); k++)
				{
					shared_ptr<ms_linear_transition> tr = static_pointer_cast<ms_linear_transition>(mst->get_transition_at(k));
					int iidx = tr->get_interval_index();
					shared_ptr<linear_transition> tr2 = tr->get_transition_at(iidx);
					float bw1 = uw * interpolate_smooth_step(tr2->get_position(), limits1[iidx], limits1[iidx + 1], 0);
					float bw2 = uw * interpolate_smooth_step(tr2->get_position(), limits2[iidx], limits2[iidx + 1], 0);
					float bw3 = uw * interpolate_smooth_step(tr->get_position(), 0, 1, 0);

					draw_bar(g, 10, 30 + k * 80, bw1, 25, 0x8200b4);
					draw_bar(g, 10, 30 + 25 + k * 80, bw2, 25, 0x8200b4);
					draw_bar(g, 10, 30 + 50 + k * 80, bw3, 20, 0xffff00);
				}
			}

			std::string text = "exercitiul [3]";
			time_duration td = total + d;

			float ts = 2;

			fy = fh;

			g->drawText(text, 10, fy += fh);
			g->drawText(trs("numar de exercitii efectuate [{}]", runCount), 10, fy += fh);
			g->drawText(trs("durata exercitiu curent [{0} s, {1} ms]", d.seconds(), (d.total_milliseconds() % 1000)), 10, fy += fh);
			g->drawText(trs("durata totala [{0} m, {1} s]", td.minutes(), td.seconds()), 10, fy += fh);
		}

	private:
		std::vector<float> limits1;
		std::vector<float> limits2;
	};


	class astropage : public mws_page
	{
	public:
		astropage(shared_ptr<mws_page_tab> iu) : mws_page(iu){}

		virtual void init()
		{
			int tduration[] = { 2000, 8000, 4000, 1000 };
			int tdurationLength = sizeof(tduration) / sizeof(int);
			float lim[] = { 0, 1.f / 3, 2.f / 3, 1, 1 };

			mslt = ms_linear_transition::new_instance(ms_transition_data::new_duration_data(tduration, tdurationLength));

         for (float tl : lim)
			{
				limits.push_back(tl);
			}
		}

		virtual void update_state()
		{
			mws_page::update_state();

			if (mslt->is_finished())
			{
				mslt->start();
			}

			mslt->update();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			g->drawText("astropage", 10, 10);

			int uw = get_unit()->get_width();
			int uh = get_unit()->get_height();
			int xw = uw / 3;
			int xh = uh / 3;
			float x = xw / 2;
			float y = xh / 2;
			float w = xw;
			float h = xh;
			int color = 0x8200b4;
			gfx_ubyte ca[] = { 15, 255, 205, 0 };
			int cr = 0x82;
			int cg = 0;
			int cb = 0xb4;

			//vxfmt_v2fc4b va[] =
			//{
			//	{x, y, cr, cg, cb, ca[0],},
			//	{x + w, y, cr, cg, cb, ca[1],},
			//	{x, y + h, cr, cg, cb, ca[3],},
			//	{x + w, y + h, cr, cg, cb, ca[2],},
			//};

			//g->fillAlphaGradientQuad(va);

			g->setColor(0);
			g->fillRect(315, 150, 2, 2);

			int intervalIdx = mslt->get_interval_index();
			const shared_ptr<linear_transition> lt = mslt->get_transition_at(intervalIdx);
			float miw = uw * interpolate_smooth_step(lt->get_position(), limits[intervalIdx], limits[intervalIdx + 1], 0);
			float miw2 = uw * interpolate_smooth_step(mslt->get_position(), 0, 1, 0);

			draw_bar(g, 0, 350, miw, 30, color);
			draw_bar(g, 0, 370 + 30, miw2, 20, 0xffff00);
		}

	private:
		shared_ptr<ms_linear_transition> mslt;
		std::vector<float> limits;
	};


	// mws-start
	mws_page::new_shared_instance(new mainpage(mws_root));
	mws_page::new_shared_instance(new ex1page(mws_root, ex1page::type1));
	mws_page::new_shared_instance(new ex1page(mws_root, ex1page::type2));
	mws_page::new_shared_instance(new ex3page(mws_root));
	mws_page::new_shared_instance(new astropage(mws_root));
}

#endif