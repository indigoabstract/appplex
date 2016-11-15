#include "stdafx.h"

#include "unit.hpp"
#include "com/ux/ux-camera.hpp"
#include "com/ux/ux.hpp"
#include "com/ux/ux-com.hpp"
#include "unit-ctrl.hpp"
#include "min.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/unit/update-ctrl.hpp"
#include "gfx.hpp"
#include "gfx-scene.hpp"
#include "gfx-state.hpp"
#include <boost/filesystem.hpp>
#include "media/res-ld/res-ld.hpp"
#include "com/ux/font-db.hpp"
#include <cstdio>

using std::string;
using std::vector;
namespace bfs = ::boost::filesystem;


int unit::unit_count = 0;

unit::unit()
{
	char unit_name[256];

	sprintf(unit_name, "unit#%d", unit_count);
	unit_count++;
	initVal = false;
	name = string(unit_name);
	prefs = shared_ptr<unit_preferences>(new unit_preferences());
	game_time = 0;

	if (is_gfx_unit())
	{
		update_ctrl = updatectrl::new_instance();
		touch_ctrl = touchctrl::new_instance();
		key_ctrl = keyctrl::new_instance();
		gfx_scene_inst = shared_ptr<gfx_scene>(new gfx_scene());
		gfx_scene_inst->init();
	}
}

unit::~unit()
{
}

unit::unit_type unit::get_unit_type()
{
	return e_unit_base;
}

int unit::get_width()
{
	return pfm::screen::get_width();
}

int unit::get_height()
{
	return pfm::screen::get_height();
}

const string& unit::get_name()
{
	return name;
}

void unit::set_name(string iname)
{
	name = iname;
}

void unit::set_app_exit_on_next_run(bool iapp_exit_on_next_run)
{
	unit_ctrl::inst()->set_app_exit_on_next_run(iapp_exit_on_next_run);
}

bool unit::gfx_available()
{
	return pfm::screen::is_gfx_available();
}

shared_ptr<unit> unit::get_smtp_instance()
{
	return shared_from_this();
}

bool unit::update()
{
	int updateCount = 1;//update_ctrl->update();

	for(int k = 0; k < updateCount; k++)
	{
		touch_ctrl->update();
		key_ctrl->update();
		game_time += update_ctrl->getTimeStepDuration();
	}

	gfx_scene_inst->update();
	uxroot->update_state();

	gfx_scene_inst->draw();
	update_view(updateCount);

	// update fps
	frame_count++;
	uint32 now = pfm::time::get_time_millis();
	uint32 dt = now - last_frame_time;

	if(dt >= 1000)
	{
		fps = frame_count * 1000.f / dt;
		last_frame_time = now;
		frame_count = 0;
	}

	return true;
}

void unit::on_resize()
{
	if (is_gfx_unit() && gfx::is_init())
	{
		shared_ptr<gfx_state> gfx_st = gfx::get_gfx_state();
		int w = get_width();
		int h = get_height();

		decl_gfxpl(pl1)
		{
			{gl::VIEWPORT, 0, 0, w, h},
			{},
		};

		gfx_st->set_state(pl1);

		if (uxroot)
		{
			uxroot->on_resize();
		}
	}
}

void unit::receive(shared_ptr<iadp> idp)
{
	send(uxroot, idp);

	if(!idp->is_processed())
	{
		if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

			//if(ts->get_type() == touch_sym_evt::TS_BACKWARD_SWIPE)
			//{
			//	back();
			//	ts->process();
			//}
		}
		else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

			if(ke->get_type() != key_evt::KE_RELEASED)
			{
				bool isAction = false;

				if(ke->get_type() != key_evt::KE_REPEATED)
				{
					isAction = true;

					switch(ke->get_key())
					{
					case KEY_ESCAPE:
						back();
						break;

					case KEY_F1:
						unit_ctrl::inst()->pause();
						break;

					case KEY_F2:
						unit_ctrl::inst()->resume();
						break;

					case KEY_F6:
						unit_ctrl::inst()->set_app_exit_on_next_run(true);
						break;

					case KEY_F11:
						pfm::screen::set_full_screen_mode(!pfm::screen::is_full_screen_mode());
						break;

					default:
						isAction = false;
					}
				}

				if(isAction)
				{
					ke->process();
				}
			}
		}
	}
}

void unit::iInit()
{
	init();
	storage.u = get_smtp_instance();

#ifdef MOD_GFX
	// getInst() doesn't work in the constructor
	if (is_gfx_unit())
	{
		touch_ctrl->add_receiver(get_smtp_instance());
		key_ctrl->add_receiver(get_smtp_instance());
		uxroot = ux_page_tab::new_instance(get_smtp_instance());
		ux_cam = ux_camera::new_inst();
		ux_cam->camera_id = "ux_cam";
		ux_cam->projection_type = "orthographic";
		ux_cam->near_clip_distance = -100;
		ux_cam->far_clip_distance = 100;
		ux_cam->clear_color = false;
		ux_cam->clear_color_value = gfx_color::colors::black;
		ux_cam->clear_depth = true;
		gfx_scene_inst->attach(ux_cam);

		init_ux();
		uxroot->init();
	}
#endif
}

/**
* Called on entering the unit for the first time.
*/
void unit::init()
{
}

/**
* Called before the unit is destroyed.
*/
void unit::on_destroy()
{
	if (isInit())
	{
		//uxroot->on_destroy();
	}
}

void unit::init_ux()
{
}

/**
* Called on entering the unit.
*/
void unit::load()
{
}

/**
* Called on leaving the unit.
*/
void unit::unload()
{
}

bool unit::rsk_was_hit(int x0, int y0)
{
	int w = 32, h = 32;
	int radius = std::max(w, h) / 2;
	int cx = get_width() - w / 2 - 4;
	int cy = get_height() - h / 2 - 4;
	int dx = cx - x0;
	int dy = cy - y0;

	if (sqrtf(dx * dx + dy * dy) <= radius)
	{
		return true;
	}

	return false;
}

int unit::schedule_operation(const std::function<void()> &ioperation)
{
	operation_mutex.lock();
	operation_list.push_back(ioperation);
	operation_mutex.unlock();

	return 0;
}

bool unit::cancel_operation(int ioperation_id)
{
	return false;
}

void unit::back()
{
	unit_list::up_one_level();
}

bool unit::is_gfx_unit()
{
	return get_preferences()->requires_gfx();
}

shared_ptr<unit_preferences> unit::get_preferences()
{
	return prefs;
}

bool unit::isInit()
{
	return initVal;
}

shared_ptr<ia_sender> unit::sender_inst()
{
	return get_smtp_instance();
}

bool unit::iRunFrame()
{
	font_db::inst()->on_frame_start();

	if (!operation_list.empty())
	{
		operation_mutex.lock();
		auto temp = operation_list;

		operation_list.clear();
		operation_mutex.unlock();

		for (const auto &function : temp)
		{
			function();
		}
	}

	bool drawFrame = update();

	return drawFrame;
}

void unit::iLoad()
{
	fps = 0;
	frame_count = 0;
	last_frame_time = pfm::time::get_time_millis();

	load();
	//update_ctrl->started();
}

void unit::iUnload()
{
	unload();
	//update_ctrl->stopped();
}

void unit::setInit(bool isInit0)
{
	initVal = isInit0;
}

void unit::update_view(int update_count)
{
	//shared_ptr<ux_camera> gfx = gfx_openvg::get_instance();
	//sprenderer r = renderer::get_instance();
	//glm::mat4 cam, tm;
	//cam = glm::ortho(0.f, (float)get_width(), (float)get_height(), 0.f, -1.f, 1000.f);
	//r->mx.set_projection_matrix(cam);
	//r->mx.set_view_matrix(tm);

	//decl_scglpl(pl1)
	//{
	//	{gl::CULL_FACE, gl::FALSE_GL}, {gl::DEPTH_TEST, gl::FALSE_GL},
	//	{gl::DEPTH_WRITEMASK, gl::FALSE_GL}, {gl::VERTEX_ARRAY, gl::TRUE_GL},
	//	{gl::COLOR_ARRAY, gl::FALSE_GL}, {gl::NORMAL_ARRAY, gl::FALSE_GL}, 
	//	{gl::TEXTURE_COORD_ARRAY, gl::FALSE_GL}, {gl::TEXTURE_2D, gl::FALSE_GL},
	//	{},
	//};
	//r->st.set_state(pl1);

	shared_ptr<ux_camera> gfx = ux_cam;
	//gfx->sync_with_openvg();
	uxroot->update_view(gfx);

	//r->mx.set_projection_matrix(cam);
	//r->mx.set_view_matrix(tm);

	//if(prefs->show_onscreen_console())
	//{
	//	pfm::get_console()->draw(gfx);
	//}

	if(prefs->draw_touch_symbols_trail() && !touch_ctrl->is_pointer_released())
	{
		const vector<pointer_sample>& ps = touch_ctrl->get_pointer_samples();

		gfx->setColor(0xff0000);

		for(int k = 0; k < ps.size() - 1; k++)
		{
			const pointer_sample& p1 = ps[k];
			const pointer_sample& p2 = ps[k + 1];

			gfx->drawLine(p1.pos.x, p1.pos.y, p2.pos.x, p2.pos.y);
		}
	}

#ifdef MOD_BOOST
	if(fps > 0)
	{
		float ups = 1000.f / update_ctrl->getTimeStepDuration();
		string f = trs("uc %d u %02.1f f %02.1f") % update_count % ups % fps;

		gfx->drawText(f, get_width() - 220, 0);
	}
#endif

	//signal_opengl_error();
}

shared_ptr<std::vector<uint8> > unit::ustorage::load_unit_byte_vect(string name)
{
	return pfm::filesystem::load_unit_byte_vect(u.lock(), name);
}

//shared_array<uint8> unit::ustorage::load_unit_byte_array(string name, int& size)
//{
//	return pfm::storage::load_unit_byte_array(u.lock(), name, size);
//}

bool unit::ustorage::store_unit_byte_array(string name, const uint8* resPtr, int size)
{
	return pfm::filesystem::store_unit_byte_array(u.lock(), name, resPtr, size);
}

bool unit::ustorage::store_unit_byte_vect(string name, const std::vector<uint8>& resPtr)
{
	return pfm::filesystem::store_unit_byte_vect(u.lock(), name, resPtr);
}

shared_ptr<pfm_file> unit::ustorage::random_access(std::string name)
{
	return pfm::filesystem::random_access(u.lock(), name);
}

void unit::ustorage::save_screenshot(std::string ifilename)
{
	if (!u.lock()->is_gfx_unit())
	{
		return;
	}

	shared_ptr<std::vector<uint32> > pixels = gfx::rt::get_render_target_pixels<uint32>();
	shared_ptr<pfm_file> screenshot_file;

	if (ifilename.size() == 0)
	{
		string file_root = trs("%1%-") % u.lock()->get_name();
		string img_ext = ".png";
		string zeroes[] =
		{
			"00", "0"
		};
		std::string dir_name = trs("f:\\data\\media\\work\\screens\\%1%") % u.lock()->get_name();
		//shared_ptr<pfm_file> dir = pfm_file::get_inst(dir_name);
		bfs::path dst_dir(dir_name);
		//bfs::path screenshot_file;
		int screenshot_idx = 0;

		if (!bfs::exists(dst_dir))
			// if dir doesn't exist, create it
		{
			bfs::create_directory(dst_dir);
		}

		do
			// find the first available file name.
		{
			string idx_nr = "";
			int digits = 0;
			int ssi = screenshot_idx;

			// calc. number of digits in a number.
			while ((ssi /= 10) > 0)
			{
				digits++;
			}

			// assign a zero prefix.
			if (digits < 2)
			{
				idx_nr = trs("%1%%2%%3%") % file_root % zeroes[digits] % screenshot_idx;
			}
			else
			{
				idx_nr = trs("%1%%2%") % file_root % screenshot_idx;
			}

			//screenshot_file = dst_dir / bfs::path(trs("%1%%2%") % idx_nr % img_ext);
			std::string file_name = trs("%1%%2%") % idx_nr % img_ext;
			screenshot_file = pfm_file::get_inst(dir_name + "\\" + file_name);
			screenshot_idx++;
		}
		//while (bfs::exists(screenshot_file));
		while (screenshot_file->exists());
	}
	else
	{
		screenshot_file = pfm_file::get_inst(ifilename);
	}

	res_ld::inst()->save_image(screenshot_file, gfx::rt::get_screen_width(), gfx::rt::get_screen_height(), (uint8*)begin_ptr(pixels), res_ld::e_vertical_flip);
}


int unit_list::unit_list_count = 0;


unit_list::unit_list()
{
	char unit_name[256];

	sprintf(unit_name, "unit-list#%d", unit_list_count);
	set_name(string(unit_name));

	unit_list_count++;
}

shared_ptr<unit_list> unit_list::new_instance()
{
	return shared_ptr<unit_list>(new unit_list());
}

unit::unit_type unit_list::get_unit_type()
{
	return e_unit_list;
}

void unit_list::add(shared_ptr<unit> iunit)
{
	ia_assert(iunit != shared_ptr<unit>(), "IllegalArgumentException");

	iunit->parent = get_smtp_instance();
	ulist.push_back(iunit);
	//ulmodel.lock()->notify_update();
}

shared_ptr<unit> unit_list::unit_at(int iindex)
{
	return ulist[iindex];
}

shared_ptr<unit> unit_list::unit_by_name(string iname)
{
	int size = ulist.size();

	for (int i = 0; i < size; i++)
	{
		shared_ptr<unit> u = ulist[i];

		if (u->get_name().compare(iname) == 0)
		{
			return u;
		}
	}

	return shared_ptr<unit>();
}

int unit_list::get_unit_count()const
{
	return ulist.size();
}

void unit_list::on_resize()
{
	unit::on_resize();

	//BOOST_FOREACH(shared_ptr<unit> u, ulist)
	//{
	//	if(u->isInit())
	//	{
	//		u->on_resize();
	//	}
	//}
}

void unit_list::receive(shared_ptr<iadp> idp)
{
	if (!idp->is_processed() && idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		//if (ts->get_type() == touch_sym_evt::TS_FORWARD_SWIPE)
		//{
		//	forward();
		//	ts->process();
		//}
	}

	if (!idp->is_processed())
	{
		unit::receive(idp);
	}
}

void unit_list::forward()
{
	if (ulist.size() > 0)
	{
		shared_ptr<unit> u = ulist[ulmodel.lock()->get_selected_elem()];
		unit_ctrl::inst()->set_next_unit(u);// , get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
	}
}

void unit_list::up_one_level()
{
#ifndef SINGLE_UNIT_BUILD
	shared_ptr<unit> u = unit_ctrl::inst()->get_current_unit();
	shared_ptr<unit> parent = u->parent.lock();

	if (parent != NULL)
	{
		if (parent->get_unit_type() == e_unit_list)
		{
			shared_ptr<unit_list> ul = static_pointer_cast<unit_list>(parent);
			int idx = std::find(ul->ulist.begin(), ul->ulist.end(), u) - ul->ulist.begin();

			if (idx < ul->ulist.size())
			{
				ul->ulmodel.lock()->set_selected_elem(idx);
			}
		}

		unit_ctrl::inst()->set_next_unit(parent);// , get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE));
	}
#endif
}

void unit_list::on_destroy()
{
	//BOOST_FOREACH(shared_ptr<unit> u, ulist)
	//{
	//	u->on_destroy();
	//}

	ulist.clear();
}

void unit_list::init_ux()
{
	class lmodel : public ux_list_model
	{
	public:
		lmodel(shared_ptr<unit_list> iul) : ul(iul){}

		int get_length()
		{
			return get_unit_list()->ulist.size();
		}

		std::string elem_at(int idx)
		{
			return get_unit_list()->ulist[idx]->get_name();
		}

		void on_elem_selected(int idx)
		{
			shared_ptr<unit> u = get_unit_list()->ulist[idx];

			//trx("item %1%") % elemAt(idx);
			unit_ctrl::inst()->set_next_unit(u);// , get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
		}

	private:
		shared_ptr<unit_list> get_unit_list()
		{
			return ul.lock();
		}

		weak_ptr<unit_list> ul;
	};

	shared_ptr<unit_list> ul = static_pointer_cast<unit_list>(get_smtp_instance());

	shared_ptr<ux_list_model> lm((ux_list_model*)new lmodel(ul));
	shared_ptr<ux_page> p = ux_page::new_instance(uxroot);
	shared_ptr<ux_list> l = ux_list::new_instance(p);

	ulmodel = lm;
	l->set_model(lm);

	ux_cam->clear_color = true;
}


weak_ptr<unit_list> app_units_setup::ul;
weak_ptr<unit> app_units_setup::next_crt_unit;


shared_ptr<unit_list> app_units_setup::get_unit_list()
{
	return ul.lock();
}

void app_units_setup::add_unit(shared_ptr<unit> u0, bool set_current)
{
	if (get_unit_list())
	{
		get_unit_list()->add(u0);

		if (set_current)
		{
			next_crt_unit = u0;
		}
	}
}
