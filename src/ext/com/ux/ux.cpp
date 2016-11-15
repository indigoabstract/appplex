#include "stdafx.h"

#include "ux.hpp"
#include "ux-camera.hpp"
#include "ux-font.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "unit.hpp"
#include "com/unit/transitions.hpp"
#include <boost/foreach.hpp>
#include <algorithm>

using std::string;
using std::vector;


slide_scrolling::scroll_dir get_scroll_dir(touch_sym_evt::touch_sym_evt_types swipe_type)
{
	switch (swipe_type)
	{
	case touch_sym_evt::TS_BACKWARD_SWIPE:
		return slide_scrolling::SD_LEFT_RIGHT;

	case touch_sym_evt::TS_FORWARD_SWIPE:
		return slide_scrolling::SD_RIGHT_LEFT;

	case touch_sym_evt::TS_UPWARD_SWIPE:
		return slide_scrolling::SD_DOWN_UP;

	case touch_sym_evt::TS_DOWNWARD_SWIPE:
		return slide_scrolling::SD_UP_DOWN;
	}

	throw ia_exception("not a swipe type");
}


shared_ptr<ux_model> ux_model::get_instance()
{
	return shared_from_this();
}

void ux_model::receive(shared_ptr<iadp> idp)
{
}

void ux_model::set_view(shared_ptr<ux> iview)
{
	view = iview;
	notify_update();
}

void ux_model::notify_update()
{
	if (get_view())
	{
		send(get_view(), iadp::new_instance(UX_EVT_MODEL_UPDATE));
	}
}

shared_ptr<ux> ux_model::get_view()
{
	return view.lock();
}

shared_ptr<ia_sender> ux_model::sender_inst()
{
	return get_instance();
}


ux::ux()
// for rootless / parentless ux inst
{
	enabled = true;
	is_opaque = true;
}

ux::ux(shared_ptr<ux> iparent)
{
	enabled = true;
	is_opaque = true;
	parent = iparent;

	if (!iparent)
	{
		throw ia_exception("init error. parent must not be null");
	}

	root = iparent->root;

	if (root.expired())
	{
		throw ia_exception("init error. parent's root must be null");
	}
}

shared_ptr<ux> ux::get_instance()
{
	return shared_from_this();
}

void ux::set_visible(bool iis_visible)
{
	enabled = iis_visible;
}

bool ux::is_visible()const
{
	return enabled;
}

void ux::set_id(string iid)
{
	id = iid;
}

const string& ux::get_id()
{
	return id;
}

shared_ptr<ux> ux::find_by_id(const string& iid)
{
	return root.lock()->contains_id(iid);
}

shared_ptr<ux> ux::contains_id(const string& iid)
{
	if (iid == id)
	{
		return get_instance();
	}

	return shared_ptr<ux>();
}

bool ux::contains_ux(const shared_ptr<ux> iux)
{
	return iux == get_instance();
}

shared_ptr<ux> ux::get_parent()
{
	return parent.lock();
}

shared_ptr<ux_page_tab> ux::get_root()
{
	return root.lock();
}

shared_ptr<unit> ux::get_unit()
{
	return static_pointer_cast<unit>(root.lock()->get_unit());
}

void ux::receive(shared_ptr<iadp> idp) {}
void ux::update_state() {}
void ux::update_view(shared_ptr<ux_camera> g) {}

ux_rect ux::get_pos()
{
	return uxr;
}

bool ux::is_hit(float x, float y)
{
	return is_inside_box(x, y, uxr.x, uxr.y, uxr.w, uxr.h);
}

shared_ptr<ia_sender> ux::sender_inst()
{
	return get_instance();
}


shared_ptr<ux_page_transition> ux_page_transition::new_instance(shared_ptr<ux_page> ipage)
{
	return shared_ptr<ux_page_transition>(new ux_page_transition(ipage));
}

shared_ptr<ux_page_transition> ux_page_transition::new_instance(shared_ptr<ux_page_tab> iuxroot, string iid)
{
	return shared_ptr<ux_page_transition>(new ux_page_transition(iuxroot, iid));
}

ux_page_transition::ux_page_transition(shared_ptr<ux_page> ipage) : iadp(UX_EVT_PAGE_TRANSITION)
{
	page = ipage;
	dir = slide_scrolling::SD_RIGHT_LEFT;
	pt_type = REPLACE_CURRENT_PAGE;
	pj_type = HISTORY_ADD_PAGE;
}

ux_page_transition::ux_page_transition(shared_ptr<ux_page_tab> iuxroot, string iid) : iadp(UX_EVT_PAGE_TRANSITION)
{
	uxroot = iuxroot;
	id = iid;
	dir = slide_scrolling::SD_RIGHT_LEFT;
	pt_type = REPLACE_CURRENT_PAGE;
	pj_type = HISTORY_ADD_PAGE;
}

shared_ptr<ux_page> ux_page_transition::get_target_page()
{
	if (!page.expired())
	{
		return page.lock();
	}
	else
	{
		shared_ptr<ux> u = uxroot.lock()->contains_id(id);

		if (u)
		{
			return static_pointer_cast<ux_page>(u);
		}
	}

	vprint("target page with id [%s] is not available\n", id.c_str());

	return ux_page::PAGE_NONE;
}

slide_scrolling::scroll_dir ux_page_transition::get_scroll_dir()
{
	return dir;
}

ux_page_transition::page_transition_types ux_page_transition::get_transition_type()
{
	return pt_type;
}

ux_page_transition::page_jump_types ux_page_transition::get_jump_type()
{
	return pj_type;
}

shared_ptr<ux_page_transition> ux_page_transition::set_scroll_dir(slide_scrolling::scroll_dir idir)
{
	dir = idir;

	return get_instance();
}

shared_ptr<ux_page_transition> ux_page_transition::set_transition_type(page_transition_types iptType)
{
	pt_type = iptType;

	return get_instance();
}

shared_ptr<ux_page_transition> ux_page_transition::set_jump_type(page_jump_types ipjType)
{
	pj_type = ipjType;

	return get_instance();
}

shared_ptr<ux_page_transition> ux_page_transition::get_instance()
{
	return shared_from_this();
}


static shared_ptr<gfx_tex> keyboardImg;


class uxpagetab_vkeyboard_page : public ux_page
{
public:
	uxpagetab_vkeyboard_page(shared_ptr<ux_page_tab> iparent, string iid) : ux_page(iparent)
	{
		set_id(iid);
		tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = ux_page_transition::new_instance(ux_page::PREV_PAGE)
			->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE))
			->set_transition_type(ux_page_transition::POP_CURRENT_PAGE);
		tmap.erase(touch_sym_evt::TS_FORWARD_SWIPE);
	}

	virtual void receive(shared_ptr<iadp> idp)
	{
		if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
		{
			shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

			if (ts->get_type() == touch_sym_evt::TS_FIRST_TAP)
			{
				float x = ts->pressed.pos.x;
				float y = ts->pressed.pos.y;

				if (is_inside_box(x, y, uxr.x, uxr.h - 40, uxr.w, uxr.h))
				{
					shared_ptr<ux_page_transition> upt = ux_page_transition::new_instance(ux_page::PREV_PAGE)
						->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
						->set_transition_type(ux_page_transition::POP_CURRENT_PAGE);

					send(get_parent(), upt);
				}
			}
		}

		ux_page::receive(idp);
	}

	virtual void update_view(shared_ptr<ux_camera> g)
	{
		ux_page::update_view(g);

		if (keyboardImg)
		{
			g->push_transform_state();
			g->scale((float)pfm::screen::get_width() / keyboardImg->get_width(), (float)pfm::screen::get_height() / keyboardImg->get_height());
			g->drawImage(keyboardImg, 0, 0);
			g->pop_transform_state();
		}
		else
		{
			//keyboardImg = vg_image::load_image("voronoi/voronoi-vkb3.png");
		}

		g->drawText(get_id(), 10, 10);
	}
};


const string ux_page_tab::VKEYBOARD_MAIN_PAGE = "vkeyboard-main-page";
const string ux_page_tab::VKEYBOARD_UP_PAGE = "vkeyboard-up-page";
const string ux_page_tab::VKEYBOARD_RIGHT_PAGE = "vkeyboard-right-page";
const string ux_page_tab::VKEYBOARD_DOWN_PAGE = "vkeyboard-down-page";


ux_page_tab::ux_page_tab(shared_ptr<unit> iu) : ux(), ss(550)
{
	if (!iu)
	{
		throw ia_exception("unit cannot be null");
	}

	u = iu;
	uxr.set(0, 0, iu->get_width(), iu->get_height());
}

shared_ptr<ux_page_tab> ux_page_tab::new_instance(shared_ptr<unit> iu)
{
	shared_ptr<ux_page_tab> pt(new ux_page_tab(iu));
	pt->new_instance_helper();
	return pt;
}

shared_ptr<ux_page_tab> ux_page_tab::new_shared_instance(ux_page_tab* inew_page_tab_class_instance)
{
	shared_ptr<ux_page_tab> pt(inew_page_tab_class_instance);
	pt->new_instance_helper();
	return pt;
}

void ux_page_tab::init()
{
	root = get_ux_page_tab_instance();
	ss.get_transition()->add_receiver(get_instance());

	if (!is_empty())
	{
		current_page = pages[4];
		page_history.push_back(current_page);
		page_stack.push_back(current_page);
	}

	BOOST_FOREACH(shared_ptr<ux_page> p, pages)
	{
		p->init();
	}
}

void ux_page_tab::on_destroy()
{
	BOOST_FOREACH(shared_ptr<ux_page> p, pages)
	{
		p->on_destroy();
	}
}

shared_ptr<ux> ux_page_tab::contains_id(const string& iid)
{
	if (iid.length() > 0)
	{
		if (iid == get_id())
		{
			return get_instance();
		}

		BOOST_FOREACH(shared_ptr<ux_page> p, pages)
		{
			shared_ptr<ux> u = p->contains_id(iid);

			if (u)
			{
				return u;
			}
		}
	}

	return shared_ptr<ux>();
}

bool ux_page_tab::contains_ux(const shared_ptr<ux> iux)
{
	BOOST_FOREACH(shared_ptr<ux_page> p, pages)
	{
		if (iux == p || p->contains_ux(iux))
		{
			return true;
		}
	}

	return false;
}

shared_ptr<ux_page_tab> ux_page_tab::get_ux_page_tab_instance()
{
	return static_pointer_cast<ux_page_tab>(get_instance());
}

shared_ptr<unit> ux_page_tab::get_unit()
{
	return static_pointer_cast<unit>(u.lock());
}

bool ux_page_tab::is_empty()
{
	return pages.size() <= 4;
}

void ux_page_tab::receive(shared_ptr<iadp> idp)
{
	if (idp->is_processed())
	{
		return;
	}

	shared_ptr<ia_sender> source = idp->source();
	shared_ptr<ms_linear_transition> mst = ss.get_transition();

	if (idp->is_type(UX_EVT_PAGE_TRANSITION))
	{
		current_transition = static_pointer_cast<ux_page_transition>(idp);
		shared_ptr<ux_page> targetPage = current_transition->get_target_page();

		switch (current_transition->get_transition_type())
		{
		case ux_page_transition::REPLACE_CURRENT_PAGE:
			//current_page->on_visibility_changed(false);
			//current_page = current_transition->get_target_page();
			//page_stack.back() = current_page;
			break;

		case ux_page_transition::PUSH_CURRENT_PAGE:
			current_page->on_visibility_changed(false);
			current_page = current_transition->get_target_page();
			page_stack.push_back(current_page);
			break;

		case ux_page_transition::POP_CURRENT_PAGE:
			current_page->on_visibility_changed(false);
			page_stack.erase(page_stack.end() - 1);
			current_page = page_stack.back();
			break;

		case ux_page_transition::CLEAR_PAGE_STACK:
			break;
		}

		//mst->start();
		//ss.set_scroll_dir(current_transition->get_scroll_dir());
		//ss.start();
		current_page->on_visibility_changed(true);
	}
	else if (!is_empty())
	{
		send(current_page, idp);
	}

	return;

	//-------------------- old code. inactivated.
	//if (source == mst)
	//{
	//	last_page->on_hide_transition(mst);
	//	current_page->on_show_transition(mst);
	//}
	//else if (idp->is_type(UX_EVT_PAGE_TRANSITION))
	//{
	//	current_transition = static_pointer_cast<ux_page_transition>(idp);
	//	shared_ptr<ux_page> targetPage = current_transition->get_target_page();
	//	bool startTransition = false;

	//	if (targetPage == ux_page::PAGE_NONE)
	//	{
	//	}
	//	else if (targetPage == ux_page::PREV_PAGE)
	//	{
	//		if (page_history.size() > 1)
	//		{
	//			page_history.erase(page_history.end() - 1);
	//			last_page = current_page;
	//			current_page = page_history.back();
	//			startTransition = true;
	//		}
	//		else
	//		{
	//			//get_unit()->back();
	//		}
	//	}
	//	else if (targetPage == ux_page::NEXT_PAGE)
	//	{
	//		int idx = get_page_index(current_page);

	//		if (idx < pages.size() - 1)
	//		{
	//			last_page = current_page;
	//			current_page = pages[idx + 1];
	//			page_history.push_back(current_page);
	//			startTransition = true;
	//		}
	//	}
	//	else
	//	{
	//		int idx = get_page_index(targetPage);

	//		if (idx < 0)
	//		{
	//			throw ia_exception("target page cannot be found");
	//		}

	//		last_page = current_page;
	//		current_page = pages[idx];

	//		switch (current_transition->get_jump_type())
	//		{
	//		case ux_page_transition::HISTORY_ADD_PAGE:
	//			page_history.push_back(current_page);
	//			break;

	//		case ux_page_transition::HISTORY_REWIND_TO_PAGE:
	//		{
	//			vector<shared_ptr<ux_page> >::reverse_iterator it = std::find(page_history.rbegin(), page_history.rend(), targetPage);

	//			if (it != page_history.rend())
	//			{
	//				page_history.erase(it.base(), page_history.end());
	//			}
	//			else
	//			{
	//				page_history.clear();
	//				page_history.push_back(targetPage);
	//			}

	//			break;
	//		}

	//		case ux_page_transition::HISTORY_IGNORE_PAGE:
	//			break;
	//		}

	//		startTransition = true;
	//	}

	//	if (startTransition)
	//	{
	//		switch (current_transition->get_transition_type())
	//		{
	//		case ux_page_transition::REPLACE_CURRENT_PAGE:
	//			page_stack.back() = current_page;
	//			break;

	//		case ux_page_transition::PUSH_CURRENT_PAGE:
	//			page_stack.push_back(current_page);
	//			break;

	//		case ux_page_transition::POP_CURRENT_PAGE:
	//			page_stack.erase(page_stack.end() - 1);
	//			page_stack.back() = current_page;
	//			break;

	//		case ux_page_transition::CLEAR_PAGE_STACK:
	//			break;
	//		}

	//		mst->start();
	//		ss.set_scroll_dir(current_transition->get_scroll_dir());
	//		ss.start();
	//		current_page->on_visibility_changed(true);
	//	}
	//}
	//else if (!is_empty())
	//{
	//	send(current_page, idp);
	//}
}

void ux_page_tab::update_state()
{
	if (!is_empty())
	{
		if (!ss.is_finished())
		{
			ss.update();

			switch (current_transition->get_transition_type())
			{
			case ux_page_transition::REPLACE_CURRENT_PAGE:
				last_page->update_state();

				BOOST_FOREACH(shared_ptr<ux_page> p, page_stack)
				{
					p->update_state();
				}

				break;

			case ux_page_transition::PUSH_CURRENT_PAGE:
				BOOST_FOREACH(shared_ptr<ux_page> p, page_stack)
				{
					p->update_state();
				}

				break;

			case ux_page_transition::POP_CURRENT_PAGE:
				last_page->update_state();

				BOOST_FOREACH(shared_ptr<ux_page> p, page_stack)
				{
					p->update_state();
				}

				break;

			case ux_page_transition::CLEAR_PAGE_STACK:
				current_page->update_state();

				BOOST_FOREACH(shared_ptr<ux_page> p, page_stack)
				{
					p->update_state();
				}

				break;
			}

			if (ss.is_finished())
			{
				last_page->on_visibility_changed(false);

				if (current_transition->get_transition_type() == ux_page_transition::CLEAR_PAGE_STACK)
				{
					page_stack.clear();
					page_stack.push_back(current_page);
				}

				current_transition.reset();
			}
		}
		else
		{
			BOOST_FOREACH(shared_ptr<ux_page> p, page_stack)
			{
				p->update_state();
			}
		}
	}
}

void ux_page_tab::update_view(shared_ptr<ux_camera> g)
{
	if (!is_empty())
	{
		int c = 0;

		for (int k = page_stack.size() - 1; k >= 0; k--)
		{
			if (page_stack[k]->is_opaque)
			{
				c = k;
				break;
			}
		}

		for (int k = c; k < page_stack.size(); k++)
		{
			shared_ptr<ux_page> p = page_stack[k];

			g->push_transform_state();
			g->translate(p->uxr.x, p->uxr.y);
			p->update_view(g);
			g->pop_transform_state();
		}
	}

	return;

	//-------------------- old code. inactivated.
	//if (!is_empty())
	//{
	//	if (!ss.is_finished())
	//	{
	//		float sx = ss.srcpos.x * get_unit()->get_width();
	//		float sy = ss.srcpos.y * get_unit()->get_height();
	//		float dx = ss.dstpos.x * get_unit()->get_width();
	//		float dy = ss.dstpos.y * get_unit()->get_height();

	//		switch (current_transition->get_transition_type())
	//		{
	//		case ux_page_transition::REPLACE_CURRENT_PAGE:
	//		{
	//			int size = page_stack.size() - 1;

	//			for (int k = 0; k < size; k++)
	//			{
	//				shared_ptr<ux_page> p = page_stack[k];

	//				g->push_transform_state();
	//				g->translate(p->uxr.x, p->uxr.y);

	//				p->update_view(g);

	//				g->pop_transform_state();
	//			}

	//			g->push_transform_state();
	//			g->translate(sx, sy);
	//			g->push_transform_state();
	//			g->translate(last_page->uxr.x, last_page->uxr.y);

	//			last_page->update_view(g);

	//			g->pop_transform_state();
	//			g->pop_transform_state();

	//			g->push_transform_state();
	//			g->translate(dx, dy);
	//			g->push_transform_state();
	//			g->translate(current_page->uxr.x, current_page->uxr.y);

	//			current_page->update_view(g);

	//			g->pop_transform_state();
	//			g->pop_transform_state();
	//			break;
	//		}

	//		case ux_page_transition::PUSH_CURRENT_PAGE:
	//		{
	//			int size = page_stack.size() - 1;

	//			for (int k = 0; k < size; k++)
	//			{
	//				shared_ptr<ux_page> p = page_stack[k];

	//				g->push_transform_state();
	//				g->translate(p->uxr.x, p->uxr.y);

	//				p->update_view(g);

	//				g->pop_transform_state();
	//			}

	//			g->push_transform_state();
	//			g->translate(dx, dy);
	//			g->push_transform_state();
	//			g->translate(current_page->uxr.x, current_page->uxr.y);

	//			current_page->update_view(g);

	//			g->pop_transform_state();
	//			g->pop_transform_state();
	//			break;
	//		}

	//		case ux_page_transition::POP_CURRENT_PAGE:
	//		{
	//			int size = page_stack.size();

	//			for (int k = 0; k < size; k++)
	//			{
	//				shared_ptr<ux_page> p = page_stack[k];

	//				g->push_transform_state();
	//				g->translate(p->uxr.x, p->uxr.y);

	//				p->update_view(g);

	//				g->pop_transform_state();
	//			}

	//			g->push_transform_state();
	//			g->translate(sx, sy);
	//			g->push_transform_state();
	//			g->translate(last_page->uxr.x, last_page->uxr.y);

	//			last_page->update_view(g);

	//			g->pop_transform_state();
	//			g->pop_transform_state();
	//			break;
	//		}

	//		case ux_page_transition::CLEAR_PAGE_STACK:
	//		{
	//			int size = page_stack.size();

	//			for (int k = 0; k < size; k++)
	//			{
	//				shared_ptr<ux_page> p = page_stack[k];

	//				g->push_transform_state();
	//				g->translate(sx, sy);
	//				g->push_transform_state();
	//				g->translate(p->uxr.x, p->uxr.y);

	//				p->update_view(g);

	//				g->pop_transform_state();
	//				g->pop_transform_state();
	//			}

	//			g->push_transform_state();
	//			g->translate(dx, dy);
	//			g->push_transform_state();
	//			g->translate(current_page->uxr.x, current_page->uxr.y);

	//			current_page->update_view(g);

	//			g->pop_transform_state();
	//			g->pop_transform_state();
	//			break;
	//		}
	//		}
	//	}
	//	else
	//	{
	//		int c = 0;

	//		for (int k = page_stack.size() - 1; k >= 0; k--)
	//		{
	//			if (page_stack[k]->is_opaque)
	//			{
	//				c = k;
	//				break;
	//			}
	//		}

	//		for (int k = c; k < page_stack.size(); k++)
	//		{
	//			shared_ptr<ux_page> p = page_stack[k];

	//			g->push_transform_state();
	//			g->translate(p->uxr.x, p->uxr.y);
	//			p->update_view(g);
	//			g->pop_transform_state();
	//		}
	//	}
	//}
}

shared_ptr<ux_page> ux_page_tab::get_page_at(int idx)
{
	return pages[idx + 4];
}

void ux_page_tab::set_first_page(shared_ptr<ux_page> up)
{
	int idx = get_page_index(up);

	if (idx > 0)
	{
		shared_ptr<ux_page> swp = pages[idx];

		pages.erase(pages.begin() + idx);
		pages.insert(pages.begin() + 4, swp);
	}
	else if (idx == -1)
	{
		throw ia_exception("page is not a member of this container");
	}
}

void ux_page_tab::show_vkeyboard()
{
	shared_ptr<ux_page_transition> upt = ux_page_transition::new_instance(get_ux_page_tab_instance(), VKEYBOARD_MAIN_PAGE)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
		->set_transition_type(ux_page_transition::PUSH_CURRENT_PAGE);

	send(get_instance(), upt);
}

void ux_page_tab::on_resize()
{
	uxr.w = u.lock()->get_width();
	uxr.h = u.lock()->get_height();

	BOOST_FOREACH(shared_ptr<ux_page> p, pages)
	{
		p->on_resize();
	}
}

void ux_page_tab::add(shared_ptr<ux_page> p)
{
	if (contains_ux(p))
	{
		throw ia_exception();//trs("page with id [%1%] already exists") % p->get_id());
	}

	pages.push_back(p);
}

int ux_page_tab::get_page_index(shared_ptr<ux_page> ipage)
{
	int k = 0;

	BOOST_FOREACH(shared_ptr<ux_page> p, pages)
	{
		if (ipage == p)
		{
			return k;
		}

		k++;
	}

	return -1;
}

void ux_page_tab::new_instance_helper()
{
	shared_ptr<ux_page_tab> uxroot = get_ux_page_tab_instance();
	root = uxroot;
	shared_ptr<ux_page> vkmainpage = ux_page::new_shared_instance(new uxpagetab_vkeyboard_page(uxroot, VKEYBOARD_MAIN_PAGE));
	shared_ptr<ux_page> vkuppage = ux_page::new_shared_instance(new uxpagetab_vkeyboard_page(uxroot, VKEYBOARD_UP_PAGE));
	shared_ptr<ux_page> vkrightpage = ux_page::new_shared_instance(new uxpagetab_vkeyboard_page(uxroot, VKEYBOARD_RIGHT_PAGE));
	shared_ptr<ux_page> vkdownpage = ux_page::new_shared_instance(new uxpagetab_vkeyboard_page(uxroot, VKEYBOARD_DOWN_PAGE));

	vkmainpage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = ux_page_transition::new_instance(vkdownpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);
	vkmainpage->tmap[touch_sym_evt::TS_FORWARD_SWIPE] = ux_page_transition::new_instance(vkrightpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);
	vkmainpage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = ux_page_transition::new_instance(vkuppage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);

	vkuppage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = ux_page_transition::new_instance(vkdownpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);
	vkuppage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = ux_page_transition::new_instance(vkmainpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);

	vkrightpage->tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = ux_page_transition::new_instance(vkmainpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);

	vkdownpage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = ux_page_transition::new_instance(vkuppage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);
	vkdownpage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = ux_page_transition::new_instance(vkmainpage)
		->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
		->set_jump_type(ux_page_transition::HISTORY_IGNORE_PAGE);
}


const shared_ptr<ux_page> ux_page::PAGE_NONE = ux_page::new_standalone_instance();
const shared_ptr<ux_page> ux_page::PREV_PAGE = ux_page::new_standalone_instance();
const shared_ptr<ux_page> ux_page::NEXT_PAGE = ux_page::new_standalone_instance();


ux_page::ux_page() : ux()
{
}

ux_page::ux_page(shared_ptr<ux_page_tab> iparent) : ux(iparent)
{
	shared_ptr<unit> tu = iparent->get_unit();

	uxr.set(0, 0, tu->get_width(), tu->get_height());

	tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = ux_page_transition::new_instance(PREV_PAGE)->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE));
	tmap[touch_sym_evt::TS_FORWARD_SWIPE] = ux_page_transition::new_instance(NEXT_PAGE)->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
}

shared_ptr<ux_page> ux_page::new_instance(shared_ptr<ux_page_tab> iparent)
{
	shared_ptr<ux_page> u(new ux_page(iparent));
	iparent->add(u);
	return u;
}

shared_ptr<ux_page> ux_page::new_shared_instance(ux_page* inew_page_class_instance)
{
	shared_ptr<ux_page> u(inew_page_class_instance);
	shared_ptr<ux_page_tab> pt = static_pointer_cast<ux_page_tab>(u->get_parent());
	pt->add(u);
	return u;
}

void ux_page::init() {}

void ux_page::on_destroy()
{
	BOOST_FOREACH(shared_ptr<ux_page_item> p, mlist)
	{
		p->on_destroy();
	}
}

shared_ptr<ux> ux_page::contains_id(const string& iid)
{
	if (iid.length() > 0)
	{
		if (iid == get_id())
		{
			return get_instance();
		}

		BOOST_FOREACH(shared_ptr<ux_page_item> p, mlist)
		{
			shared_ptr<ux> u = p->contains_id(iid);

			if (u)
			{
				return u;
			}
		}
	}

	return shared_ptr<ux>();
}

bool ux_page::contains_ux(const shared_ptr<ux> iux)
{
	BOOST_FOREACH(shared_ptr<ux_page_item> p, mlist)
	{
		if (iux == p)
		{
			return true;
		}
	}

	return false;
}

shared_ptr<ux_page> ux_page::get_ux_page_instance()
{
	return static_pointer_cast<ux_page>(get_instance());
}

shared_ptr<ux_page_tab> ux_page::get_ux_page_parent()
{
	return static_pointer_cast<ux_page_tab>(get_parent());
}

void ux_page::on_visibility_changed(bool iis_visible) {}
void ux_page::on_show_transition(const shared_ptr<linear_transition> itransition) {}
void ux_page::on_hide_transition(const shared_ptr<linear_transition> itransition) {}

void ux_page::receive(shared_ptr<iadp> idp)
{
	update_input_subux(idp);
	update_input_std_behaviour(idp);
}

void ux_page::update_input_subux(shared_ptr<iadp> idp)
{
	if (idp->is_processed())
	{
		return;
	}

	if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		float x = ts->crt_state.pos.x;
		float y = ts->crt_state.pos.y;

		BOOST_FOREACH(shared_ptr<ux> b, mlist)
		{
			if (b->is_hit(x, y))
			{
				send(b, idp);

				if (idp->is_processed())
				{
					break;
				}
			}
		}
	}
}

void ux_page::update_input_std_behaviour(shared_ptr<iadp> idp)
{
	if (idp->is_processed())
	{
		return;
	}

	if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		switch (ts->get_type())
		{
		case touch_sym_evt::TS_PRESSED:
		{
			float x = ts->pressed.pos.x;
			float y = ts->pressed.pos.y;

			if (ts->tap_count == 1)
			{
				ks.grab(x, y);
				ts->process();
			}

			break;
		}

		case touch_sym_evt::TS_PRESS_AND_DRAG:
		{
			float x = ts->crt_state.pos.x;
			float y = ts->crt_state.pos.y;

			switch (ts->tap_count)
			{
			case 1:
			{
				if (ts->is_finished)
				{
					ks.start_slowdown();
				}
				else
				{
					ks.begin(ts->crt_state.pos.x, ts->crt_state.pos.y);
				}

				uxr.x += ts->crt_state.pos.x - ts->prev_state.pos.x;
				uxr.y += ts->crt_state.pos.y - ts->prev_state.pos.y;

				if (uxr.x > 0)
				{
					uxr.x = 0;
				}
				else if (uxr.x < -uxr.w + pfm::screen::get_width())
				{
					uxr.x = -uxr.w + pfm::screen::get_width();
				}

				if (uxr.y > 0)
				{
					uxr.y = 0;
				}
				else if (uxr.y < -uxr.h + pfm::screen::get_height())
				{
					uxr.y = -uxr.h + pfm::screen::get_height();
				}

				ts->process();
			}
			}

			break;
		}

		case touch_sym_evt::TS_MOUSE_WHEEL:
		{
			shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

			uxr.y += float(mw->wheel_delta) * 50.f;

			if (uxr.y > 0)
			{
				uxr.y = 0;
			}
			else if (uxr.y < -uxr.h + pfm::screen::get_height())
			{
				uxr.y = -uxr.h + pfm::screen::get_height();
			}

			ts->process();
			break;
		}

		case touch_sym_evt::TS_BACKWARD_SWIPE:
		{
			if (uxr.x < 0)
			{
				ts->process();
			}
			else if (tmap.find(touch_sym_evt::TS_BACKWARD_SWIPE) != tmap.end())
			{
				send(get_ux_page_parent(), tmap[touch_sym_evt::TS_BACKWARD_SWIPE]);
				ts->process();
			}

			break;
		}

		case touch_sym_evt::TS_FORWARD_SWIPE:
		{
			if (uxr.x > -uxr.w + pfm::screen::get_width())
			{
				ts->process();
			}
			else if (tmap.find(touch_sym_evt::TS_FORWARD_SWIPE) != tmap.end())
			{
				send(get_ux_page_parent(), tmap[touch_sym_evt::TS_FORWARD_SWIPE]);
				ts->process();
			}

			break;
		}

		case touch_sym_evt::TS_UPWARD_SWIPE:
		{
			if (uxr.y < 0)
			{
				ts->process();
			}
			else if (tmap.find(touch_sym_evt::TS_UPWARD_SWIPE) != tmap.end())
			{
				send(get_ux_page_parent(), tmap[touch_sym_evt::TS_UPWARD_SWIPE]);
				ts->process();
			}

			break;
		}

		case touch_sym_evt::TS_DOWNWARD_SWIPE:
		{
			if (uxr.y > -uxr.h + pfm::screen::get_height())
			{
				ts->process();
			}
			else if (tmap.find(touch_sym_evt::TS_DOWNWARD_SWIPE) != tmap.end())
			{
				send(get_ux_page_parent(), tmap[touch_sym_evt::TS_DOWNWARD_SWIPE]);
				ts->process();
			}

			break;
		}
		}
	}
}

void ux_page::update_state()
{
	point2d p = ks.update();

	uxr.x += p.x;
	uxr.y += p.y;

	BOOST_FOREACH(shared_ptr<ux> b, mlist)
	{
		uxr.w = std::max(uxr.w, b->get_pos().w);
		uxr.h = std::max(uxr.h, b->get_pos().h);
	}

	if (uxr.x > 0)
	{
		uxr.x = 0;
	}
	else if (uxr.x < -uxr.w + pfm::screen::get_width())
	{
		uxr.x = -uxr.w + pfm::screen::get_width();
	}

	if (uxr.y > 0)
	{
		uxr.y = 0;
	}
	else if (uxr.y < -uxr.h + pfm::screen::get_height())
	{
		uxr.y = -uxr.h + pfm::screen::get_height();
	}

	BOOST_FOREACH(shared_ptr<ux> b, mlist)
	{
		b->update_state();
	}
}

void ux_page::update_view(shared_ptr<ux_camera> g)
{
	BOOST_FOREACH(shared_ptr<ux> b, mlist)
	{
		b->update_view(g);
	}
}

shared_ptr<ux> ux_page::get_ux_at(int idx)
{
	return mlist[idx];
}

void ux_page::on_resize()
{
	shared_ptr<ux_page_tab> parent = get_ux_page_parent();

	uxr.x = 0;
	uxr.y = 0;
	uxr.w = parent->uxr.w;
	uxr.h = parent->uxr.h;
}

shared_ptr<ux_page> ux_page::new_standalone_instance()
{
	return shared_ptr<ux_page>(new ux_page());
}

void ux_page::add(shared_ptr<ux_page_item> b)
{
	if (contains_ux(b))
	{
		throw ia_exception();//trs("uxpageitem with id [%1%] already exists") % b->get_id());
	}

	mlist.push_back(b);
}


ux_page_item::ux_page_item(shared_ptr<ux_page> iparent) : ux(iparent)
{
}

shared_ptr<ux_page> ux_page_item::get_ux_page_item_parent()
{
	return static_pointer_cast<ux_page>(get_parent());
}

void ux_page_item::add_to_page()
{
	shared_ptr<ux_page> page = static_pointer_cast<ux_page>(get_parent());
	shared_ptr<ux_page_item> inst = static_pointer_cast<ux_page_item>(get_instance());

	page->add(inst);
}
