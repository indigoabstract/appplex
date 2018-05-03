#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS

#include "mws.hpp"
#include "mws-camera.hpp"
#include "mws-font.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "gfx-vxo.hpp"
#include "unit.hpp"
#include "com/unit/transitions.hpp"
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

   mws_throw ia_exception("not a swipe type");
}


shared_ptr<mws_model> mws_model::get_instance()
{
   return shared_from_this();
}

void mws_model::receive(shared_ptr<iadp> idp)
{
}

void mws_model::set_view(shared_ptr<mws> iview)
{
   view = iview;
   notify_update();
}

void mws_model::notify_update()
{
   if (get_view())
   {
      send(get_view(), iadp::nwi(MWS_EVT_MODEL_UPDATE));
   }
}

shared_ptr<mws> mws_model::get_view()
{
   return view.lock();
}

shared_ptr<ia_sender> mws_model::sender_inst()
{
   return get_instance();
}


mws::mws(std::shared_ptr<gfx> i_gi) : gfx_node(i_gi)
// for rootless / parentless mws inst
{
   visible = true;
   is_opaque = true;
}

shared_ptr<mws> mws::get_instance()
{
   return std::static_pointer_cast<mws>(get_inst());
}

gfx_obj::e_gfx_obj_type mws::get_type()const
{
   return gfx_obj::e_mws;
}

void mws::add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   if (visible)
   {
      auto cam = mws_cam.lock();

      if (cam->camera_id() == i_camera_id)
      {
         for (auto it = children.begin(); it != children.end(); it++)
         {
            (*it)->add_to_draw_list(i_camera_id, i_opaque, i_translucent);
         }
      }
   }
}

void mws::attach(shared_ptr<gfx_node> i_node)
{
   gfx_node::attach(i_node);

   if (i_node->get_type() == gfx_obj::e_mws)
   {
      auto mws_ref = mws_dynamic_pointer_cast<mws>(i_node);

      if (mws_ref)
      {
         mws_ref->mwsroot = mwsroot;
         mws_ref->mws_cam = mwsroot.lock()->get_unit()->mws_cam;
      }
   }

   i_node->position = i_node->position() + glm::vec3(0.f, 0.f, 0.0001f);
}

void mws::list_mws_children(std::vector<mws_sp<mws> >& i_mws_subobj_list)
{
   for (auto c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

         i_mws_subobj_list.push_back(w);
         w->list_mws_children(i_mws_subobj_list);
      }
   }
}

void mws::set_visible(bool iis_visible)
{
   visible = iis_visible;
}

bool mws::is_visible()const
{
   return visible;
}

void mws::set_id(string iid)
{
   id = iid;
}

const string& mws::get_id()
{
   return id;
}

shared_ptr<mws> mws::find_by_id(const string& iid)
{
   return mwsroot.lock()->contains_id(iid);
}

shared_ptr<mws> mws::contains_id(const string& iid)
{
   if (iid == id)
   {
      return get_instance();
   }

   return shared_ptr<mws>();
}

bool mws::contains_mws(const shared_ptr<mws> i_mws)
{
   return i_mws == get_instance();
}

shared_ptr<mws> mws::get_mws_parent()
{
   return mws_dynamic_pointer_cast<mws>(get_parent());
}

shared_ptr<mws_page_tab> mws::get_mws_root()
{
   return mwsroot.lock();
}

shared_ptr<unit> mws::get_unit()
{
   return std::static_pointer_cast<unit>(mwsroot.lock()->get_unit());
}

void mws::receive(shared_ptr<iadp> idp)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

         if (w && w->visible)
         {
            send(w, idp);

            if (idp->is_processed())
            {
               break;
            }
         }
      }
   }
}

void mws::update_state() {}
void mws::update_view(shared_ptr<mws_camera> g) {}

mws_rect mws::get_pos()
{
   return mws_r;
}

float mws::get_z()
{
   return position().z;
}

void mws::set_z(float i_z_position)
{
   position = glm::vec3(position().x, position().y, i_z_position);
}


shared_ptr<ia_sender> mws::sender_inst()
{
   return get_instance();
}


shared_ptr<mws_page_transition> mws_page_transition::nwi(shared_ptr<mws_page> ipage)
{
   return shared_ptr<mws_page_transition>(new mws_page_transition(ipage));
}

shared_ptr<mws_page_transition> mws_page_transition::nwi(shared_ptr<mws_page_tab> imws_root, string iid)
{
   return shared_ptr<mws_page_transition>(new mws_page_transition(imws_root, iid));
}

mws_page_transition::mws_page_transition(shared_ptr<mws_page> ipage) : iadp(MWS_EVT_PAGE_TRANSITION)
{
   page = ipage;
   dir = slide_scrolling::SD_RIGHT_LEFT;
   pt_type = REPLACE_CURRENT_PAGE;
   pj_type = HISTORY_ADD_PAGE;
}

mws_page_transition::mws_page_transition(shared_ptr<mws_page_tab> imws_root, string iid) : iadp(MWS_EVT_PAGE_TRANSITION)
{
   mws_root = imws_root;
   id = iid;
   dir = slide_scrolling::SD_RIGHT_LEFT;
   pt_type = REPLACE_CURRENT_PAGE;
   pj_type = HISTORY_ADD_PAGE;
}

shared_ptr<mws_page> mws_page_transition::get_target_page()
{
   if (!page.expired())
   {
      return page.lock();
   }
   else
   {
      shared_ptr<mws> u = mws_root.lock()->contains_id(id);

      if (u)
      {
         return static_pointer_cast<mws_page>(u);
      }
   }

   mws_print("target page with id [%s] is not available\n", id.c_str());

   //return mws_page::PAGE_NONE;
   return nullptr;
}

slide_scrolling::scroll_dir mws_page_transition::get_scroll_dir()
{
   return dir;
}

mws_page_transition::page_transition_types mws_page_transition::get_transition_type()
{
   return pt_type;
}

mws_page_transition::page_jump_types mws_page_transition::get_jump_type()
{
   return pj_type;
}

shared_ptr<mws_page_transition> mws_page_transition::set_scroll_dir(slide_scrolling::scroll_dir idir)
{
   dir = idir;

   return get_instance();
}

shared_ptr<mws_page_transition> mws_page_transition::set_transition_type(page_transition_types iptType)
{
   pt_type = iptType;

   return get_instance();
}

shared_ptr<mws_page_transition> mws_page_transition::set_jump_type(page_jump_types ipjType)
{
   pj_type = ipjType;

   return get_instance();
}

shared_ptr<mws_page_transition> mws_page_transition::get_instance()
{
   return shared_from_this();
}


static shared_ptr<gfx_tex> keyboardImg;


class mwspagetab_vkeyboard_page : public mws_page
{
public:
   mwspagetab_vkeyboard_page(string iid)
   {
      set_id(iid);
      //tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = mws_page_transition::nwi(mws_page::PREV_PAGE)
      //   ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE))
      //   ->set_transition_type(mws_page_transition::POP_CURRENT_PAGE);
      tmap.erase(touch_sym_evt::TS_FORWARD_SWIPE);
   }

   virtual void receive(shared_ptr<iadp> idp)
   {
      if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
      {
         shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

         if (ts->get_type() == touch_sym_evt::TS_FIRST_TAP)
         {
            float x = ts->pressed.te->points[0].x;
            float y = ts->pressed.te->points[0].y;

            if (is_inside_box(x, y, mws_r.x, mws_r.h - 40, mws_r.w, mws_r.h))
            {
               //shared_ptr<mws_page_transition> upt = mws_page_transition::nwi(mws_page::PREV_PAGE)
               //   ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
               //   ->set_transition_type(mws_page_transition::POP_CURRENT_PAGE);

               //send(get_mws_parent(), upt);
            }
         }
      }

      mws_page::receive(idp);
   }

   virtual void update_view(shared_ptr<mws_camera> g)
   {
      mws_page::update_view(g);

      //if (keyboardImg)
      //{
      //	g->push_transform_state();
      //	g->scale((float)pfm::screen::get_width() / keyboardImg->get_width(), (float)pfm::screen::get_height() / keyboardImg->get_height());
      //	g->drawImage(keyboardImg, 0, 0);
      //	g->pop_transform_state();
      //}
      //else
      //{
      //	//keyboardImg = vg_image::load_image("voronoi/voronoi-vkb3.png");
      //}

      //g->drawText(get_id(), 10, 10);
   }
};


const string mws_page_tab::VKEYBOARD_MAIN_PAGE = "vkeyboard-main-page";
const string mws_page_tab::VKEYBOARD_UP_PAGE = "vkeyboard-up-page";
const string mws_page_tab::VKEYBOARD_RIGHT_PAGE = "vkeyboard-right-page";
const string mws_page_tab::VKEYBOARD_DOWN_PAGE = "vkeyboard-down-page";


mws_page_tab::mws_page_tab(shared_ptr<unit> iu) : mws(), ss(550)
{
   if (!iu)
   {
      mws_throw ia_exception("unit cannot be null");
   }

   u = iu;
   mws_r.set(0, 0, (float)iu->get_width(), (float)iu->get_height());
}

shared_ptr<mws_page_tab> mws_page_tab::nwi(shared_ptr<unit> iu)
{
   shared_ptr<mws_page_tab> pt(new mws_page_tab(iu));
   pt->new_instance_helper();
   return pt;
}

shared_ptr<mws_page_tab> mws_page_tab::new_shared_instance(mws_page_tab* inew_page_tab_class_instance)
{
   shared_ptr<mws_page_tab> pt(inew_page_tab_class_instance);
   pt->new_instance_helper();
   return pt;
}

void mws_page_tab::add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   mws::add_to_draw_list(i_camera_id, i_opaque, i_translucent);
}

void mws_page_tab::init()
{
   mwsroot = get_mws_page_tab_instance();
   mws_cam = mwsroot.lock()->get_unit()->mws_cam;
   ss.get_transition()->add_receiver(get_instance());
}
void mws_page_tab::init_subobj()
{
   if (!is_empty())
   {
      current_page = pages[4];
      page_history.push_back(current_page);
      page_stack.push_back(current_page);
   }

   static auto z_sort = [](mws_sp<mws> a, mws_sp<mws> b)
   {
      return (a->get_z() > b->get_z());
   };

   for (auto p : pages)
   {
      p->init();
      p->mws_subobj_list.clear();
      p->list_mws_children(p->mws_subobj_list);
      std::sort(p->mws_subobj_list.begin(), p->mws_subobj_list.end(), z_sort);
   }
}

void mws_page_tab::on_destroy()
{
   for (auto p : pages)
   {
      p->on_destroy();
   }
}

shared_ptr<mws> mws_page_tab::contains_id(const string& iid)
{
   if (iid.length() > 0)
   {
      if (iid == get_id())
      {
         return get_instance();
      }

      for (auto p : pages)
      {
         shared_ptr<mws> u = p->contains_id(iid);

         if (u)
         {
            return u;
         }
      }
   }

   return shared_ptr<mws>();
}

bool mws_page_tab::contains_mws(const shared_ptr<mws> i_mws)
{
   for (auto p : pages)
   {
      if (i_mws == p || p->contains_mws(i_mws))
      {
         return true;
      }
   }

   return false;
}

shared_ptr<mws_page_tab> mws_page_tab::get_mws_page_tab_instance()
{
   return static_pointer_cast<mws_page_tab>(get_instance());
}

shared_ptr<unit> mws_page_tab::get_unit()
{
   return static_pointer_cast<unit>(u.lock());
}

bool mws_page_tab::is_empty()
{
   return pages.size() <= 4;
}

void mws_page_tab::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);
      auto pa = ts->crt_state.te;
      //trx("_mt1 %1% tt %2%") % pa->is_multitouch() % pa->type;
   }
   if (idp->is_processed())
   {
      return;
   }

   shared_ptr<ia_sender> source = idp->source();
   shared_ptr<ms_linear_transition> mst = ss.get_transition();

   if (idp->is_type(MWS_EVT_PAGE_TRANSITION))
   {
      current_transition = static_pointer_cast<mws_page_transition>(idp);
      shared_ptr<mws_page> targetPage = current_transition->get_target_page();

      switch (current_transition->get_transition_type())
      {
      case mws_page_transition::REPLACE_CURRENT_PAGE:
         //current_page->on_visibility_changed(false);
         //current_page = current_transition->get_target_page();
         //page_stack.back() = current_page;
         break;

      case mws_page_transition::PUSH_CURRENT_PAGE:
         current_page->on_visibility_changed(false);
         current_page = current_transition->get_target_page();
         page_stack.push_back(current_page);
         break;

      case mws_page_transition::POP_CURRENT_PAGE:
         current_page->on_visibility_changed(false);
         page_stack.erase(page_stack.end() - 1);
         current_page = page_stack.back();
         break;

      case mws_page_transition::CLEAR_PAGE_STACK:
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
   //else if (idp->is_type(MWS_EVT_PAGE_TRANSITION))
   //{
   //	current_transition = static_pointer_cast<mws_page_transition>(idp);
   //	shared_ptr<mws_page> targetPage = current_transition->get_target_page();
   //	bool startTransition = false;

   //	if (targetPage == mws_page::PAGE_NONE)
   //	{
   //	}
   //	else if (targetPage == mws_page::PREV_PAGE)
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
   //	else if (targetPage == mws_page::NEXT_PAGE)
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
   //			mws_throw ia_exception("target page cannot be found");
   //		}

   //		last_page = current_page;
   //		current_page = pages[idx];

   //		switch (current_transition->get_jump_type())
   //		{
   //		case mws_page_transition::HISTORY_ADD_PAGE:
   //			page_history.push_back(current_page);
   //			break;

   //		case mws_page_transition::HISTORY_REWIND_TO_PAGE:
   //		{
   //			vector<shared_ptr<mws_page> >::reverse_iterator it = std::find(page_history.rbegin(), page_history.rend(), targetPage);

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

   //		case mws_page_transition::HISTORY_IGNORE_PAGE:
   //			break;
   //		}

   //		startTransition = true;
   //	}

   //	if (startTransition)
   //	{
   //		switch (current_transition->get_transition_type())
   //		{
   //		case mws_page_transition::REPLACE_CURRENT_PAGE:
   //			page_stack.back() = current_page;
   //			break;

   //		case mws_page_transition::PUSH_CURRENT_PAGE:
   //			page_stack.push_back(current_page);
   //			break;

   //		case mws_page_transition::POP_CURRENT_PAGE:
   //			page_stack.erase(page_stack.end() - 1);
   //			page_stack.back() = current_page;
   //			break;

   //		case mws_page_transition::CLEAR_PAGE_STACK:
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

void mws_page_tab::update_state()
{
   if (!is_empty())
   {
      if (!ss.is_finished())
      {
         ss.update();

         switch (current_transition->get_transition_type())
         {
         case mws_page_transition::REPLACE_CURRENT_PAGE:
            last_page->update_state();

            for (auto p : page_stack)
            {
               p->update_state();
            }

            break;

         case mws_page_transition::PUSH_CURRENT_PAGE:
            for (auto p : page_stack)
            {
               p->update_state();
            }

            break;

         case mws_page_transition::POP_CURRENT_PAGE:
            last_page->update_state();

            for (auto p : page_stack)
            {
               p->update_state();
            }

            break;

         case mws_page_transition::CLEAR_PAGE_STACK:
            current_page->update_state();

            for (auto p : page_stack)
            {
               p->update_state();
            }

            break;
         }

         if (ss.is_finished())
         {
            last_page->on_visibility_changed(false);

            if (current_transition->get_transition_type() == mws_page_transition::CLEAR_PAGE_STACK)
            {
               page_stack.clear();
               page_stack.push_back(current_page);
            }

            current_transition.reset();
         }
      }
      else
      {
         for (auto p : page_stack)
         {
            p->update_state();
         }
      }
   }
}

void mws_page_tab::update_view(shared_ptr<mws_camera> g)
{
   if (!is_empty())
   {
      int size = page_stack.size();
      int c = 0;

      for (int k = size - 1; k >= 0; k--)
      {
         if (page_stack[k]->is_opaque)
         {
            c = k;
            break;
         }
      }

      for (int k = c; k < size; k++)
      {
         shared_ptr<mws_page> p = page_stack[k];

         //g->push_transform_state();
         //g->translate(p->mws_r.x, p->mws_r.y);
         p->update_view(g);
         //g->pop_transform_state();
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
   //		case mws_page_transition::REPLACE_CURRENT_PAGE:
   //		{
   //			int size = page_stack.size() - 1;

   //			for (int k = 0; k < size; k++)
   //			{
   //				shared_ptr<mws_page> p = page_stack[k];

   //				g->push_transform_state();
   //				g->translate(p->mws_r.x, p->mws_r.y);

   //				p->update_view(g);

   //				g->pop_transform_state();
   //			}

   //			g->push_transform_state();
   //			g->translate(sx, sy);
   //			g->push_transform_state();
   //			g->translate(last_page->mws_r.x, last_page->mws_r.y);

   //			last_page->update_view(g);

   //			g->pop_transform_state();
   //			g->pop_transform_state();

   //			g->push_transform_state();
   //			g->translate(dx, dy);
   //			g->push_transform_state();
   //			g->translate(current_page->mws_r.x, current_page->mws_r.y);

   //			current_page->update_view(g);

   //			g->pop_transform_state();
   //			g->pop_transform_state();
   //			break;
   //		}

   //		case mws_page_transition::PUSH_CURRENT_PAGE:
   //		{
   //			int size = page_stack.size() - 1;

   //			for (int k = 0; k < size; k++)
   //			{
   //				shared_ptr<mws_page> p = page_stack[k];

   //				g->push_transform_state();
   //				g->translate(p->mws_r.x, p->mws_r.y);

   //				p->update_view(g);

   //				g->pop_transform_state();
   //			}

   //			g->push_transform_state();
   //			g->translate(dx, dy);
   //			g->push_transform_state();
   //			g->translate(current_page->mws_r.x, current_page->mws_r.y);

   //			current_page->update_view(g);

   //			g->pop_transform_state();
   //			g->pop_transform_state();
   //			break;
   //		}

   //		case mws_page_transition::POP_CURRENT_PAGE:
   //		{
   //			int size = page_stack.size();

   //			for (int k = 0; k < size; k++)
   //			{
   //				shared_ptr<mws_page> p = page_stack[k];

   //				g->push_transform_state();
   //				g->translate(p->mws_r.x, p->mws_r.y);

   //				p->update_view(g);

   //				g->pop_transform_state();
   //			}

   //			g->push_transform_state();
   //			g->translate(sx, sy);
   //			g->push_transform_state();
   //			g->translate(last_page->mws_r.x, last_page->mws_r.y);

   //			last_page->update_view(g);

   //			g->pop_transform_state();
   //			g->pop_transform_state();
   //			break;
   //		}

   //		case mws_page_transition::CLEAR_PAGE_STACK:
   //		{
   //			int size = page_stack.size();

   //			for (int k = 0; k < size; k++)
   //			{
   //				shared_ptr<mws_page> p = page_stack[k];

   //				g->push_transform_state();
   //				g->translate(sx, sy);
   //				g->push_transform_state();
   //				g->translate(p->mws_r.x, p->mws_r.y);

   //				p->update_view(g);

   //				g->pop_transform_state();
   //				g->pop_transform_state();
   //			}

   //			g->push_transform_state();
   //			g->translate(dx, dy);
   //			g->push_transform_state();
   //			g->translate(current_page->mws_r.x, current_page->mws_r.y);

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
   //			shared_ptr<mws_page> p = page_stack[k];

   //			g->push_transform_state();
   //			g->translate(p->mws_r.x, p->mws_r.y);
   //			p->update_view(g);
   //			g->pop_transform_state();
   //		}
   //	}
   //}
}

shared_ptr<mws_page> mws_page_tab::get_page_at(int idx)
{
   return pages[idx + 4];
}

void mws_page_tab::set_first_page(shared_ptr<mws_page> up)
{
   int idx = get_page_index(up);

   if (idx > 0)
   {
      shared_ptr<mws_page> swp = pages[idx];

      pages.erase(pages.begin() + idx);
      pages.insert(pages.begin() + 4, swp);
   }
   else if (idx == -1)
   {
      mws_throw ia_exception("page is not a member of this container");
   }
}

void mws_page_tab::show_vkeyboard()
{
   shared_ptr<mws_page_transition> upt = mws_page_transition::nwi(get_mws_page_tab_instance(), VKEYBOARD_MAIN_PAGE)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
      ->set_transition_type(mws_page_transition::PUSH_CURRENT_PAGE);

   send(get_instance(), upt);
}

void mws_page_tab::on_resize()
{
   mws_r.w = (float)u.lock()->get_width();
   mws_r.h = (float)u.lock()->get_height();

   for (auto p : pages)
   {
      p->on_resize();
   }
}

void mws_page_tab::add(shared_ptr<mws_page> p)
{
   if (contains_mws(p))
   {
      mws_throw ia_exception();//trs("page with id [%1%] already exists") % p->get_id());
   }

   pages.push_back(p);
}

int mws_page_tab::get_page_index(shared_ptr<mws_page> ipage)
{
   int k = 0;

   for (auto p : pages)
   {
      if (ipage == p)
      {
         return k;
      }

      k++;
   }

   return -1;
}

void mws_page_tab::new_instance_helper()
{
   shared_ptr<mws_page_tab> mws_root = get_mws_page_tab_instance();
   mwsroot = mws_root;
   mws_cam = mwsroot.lock()->get_unit()->mws_cam;
   shared_ptr<mws_page> vkmainpage = mws_page::new_shared_instance(mws_root, new mwspagetab_vkeyboard_page(VKEYBOARD_MAIN_PAGE));
   shared_ptr<mws_page> vkuppage = mws_page::new_shared_instance(mws_root, new mwspagetab_vkeyboard_page(VKEYBOARD_UP_PAGE));
   shared_ptr<mws_page> vkrightpage = mws_page::new_shared_instance(mws_root, new mwspagetab_vkeyboard_page(VKEYBOARD_RIGHT_PAGE));
   shared_ptr<mws_page> vkdownpage = mws_page::new_shared_instance(mws_root, new mwspagetab_vkeyboard_page(VKEYBOARD_DOWN_PAGE));

   vkmainpage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = mws_page_transition::nwi(vkdownpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);
   vkmainpage->tmap[touch_sym_evt::TS_FORWARD_SWIPE] = mws_page_transition::nwi(vkrightpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);
   vkmainpage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = mws_page_transition::nwi(vkuppage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);

   vkuppage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = mws_page_transition::nwi(vkdownpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);
   vkuppage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = mws_page_transition::nwi(vkmainpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);

   vkrightpage->tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = mws_page_transition::nwi(vkmainpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);

   vkdownpage->tmap[touch_sym_evt::TS_UPWARD_SWIPE] = mws_page_transition::nwi(vkuppage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_UPWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);
   vkdownpage->tmap[touch_sym_evt::TS_DOWNWARD_SWIPE] = mws_page_transition::nwi(vkmainpage)
      ->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_DOWNWARD_SWIPE))
      ->set_jump_type(mws_page_transition::HISTORY_IGNORE_PAGE);
}


//const shared_ptr<mws_page> mws_page::PAGE_NONE = mws_page::new_standalone_instance();
//const shared_ptr<mws_page> mws_page::PREV_PAGE = mws_page::new_standalone_instance();
//const shared_ptr<mws_page> mws_page::NEXT_PAGE = mws_page::new_standalone_instance();


mws_page::mws_page()
{
}

//mws_page::mws_page(shared_ptr<mws_page_tab> iparent)
//{
//	shared_ptr<unit> tu = iparent->get_unit();
//
//	mws_r.set(0, 0, (float)tu->get_width(), (float)tu->get_height());
//
//	tmap[touch_sym_evt::TS_BACKWARD_SWIPE] = mws_page_transition::nwi(PREV_PAGE)->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE));
//	tmap[touch_sym_evt::TS_FORWARD_SWIPE] = mws_page_transition::nwi(NEXT_PAGE)->set_scroll_dir(get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
//}

shared_ptr<mws_page> mws_page::nwi(shared_ptr<mws_page_tab> i_parent)
{
   shared_ptr<mws_page> u(new mws_page());
   i_parent->attach(u);
   i_parent->add(u);
   return u;
}

shared_ptr<mws_page> mws_page::new_shared_instance(shared_ptr<mws_page_tab> i_page_tab, mws_page* inew_page_class_instance)
{
   shared_ptr<mws_page> u(inew_page_class_instance);
   i_page_tab->attach(u);
   i_page_tab->add(u);
   return u;
}

void mws_page::init() {}

void mws_page::on_destroy()
{
   for (auto p : mlist)
   {
      p->on_destroy();
   }
}

shared_ptr<mws> mws_page::contains_id(const string& iid)
{
   if (iid.length() > 0)
   {
      if (iid == get_id())
      {
         return get_instance();
      }

      for (auto p : mlist)
      {
         shared_ptr<mws> u = p->contains_id(iid);

         if (u)
         {
            return u;
         }
      }
   }

   return shared_ptr<mws>();
}

bool mws_page::contains_mws(const shared_ptr<mws> i_mws)
{
   for (auto p : mlist)
   {
      if (i_mws == p)
      {
         return true;
      }
   }

   return false;
}

shared_ptr<mws_page> mws_page::get_mws_page_instance()
{
   return static_pointer_cast<mws_page>(get_instance());
}

shared_ptr<mws_page_tab> mws_page::get_mws_page_parent()
{
   return static_pointer_cast<mws_page_tab>(get_mws_parent());
}

void mws_page::on_visibility_changed(bool iis_visible) {}
void mws_page::on_show_transition(const shared_ptr<linear_transition> itransition) {}
void mws_page::on_hide_transition(const shared_ptr<linear_transition> itransition) {}

void mws_page::receive(shared_ptr<iadp> idp)
{
   update_input_sub_mws(idp);
   //update_input_std_behaviour(idp);
}

void mws_page::update_input_sub_mws(shared_ptr<iadp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            auto w = mws_dynamic_pointer_cast<mws>(c);

            if (w && w->visible)
            {
               send(w, idp);

               if (idp->is_processed())
               {
                  break;
               }
            }
         }
      }
   }
}

void mws_page::update_input_std_behaviour(shared_ptr<iadp> idp)
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
         float x = ts->pressed.te->points[0].x;
         float y = ts->pressed.te->points[0].y;

         if (ts->tap_count == 1)
         {
            ks.grab(x, y);
            ts->process();
         }

         break;
      }

      case touch_sym_evt::TS_PRESS_AND_DRAG:
      {
         float x = ts->crt_state.te->points[0].x;
         float y = ts->crt_state.te->points[0].y;

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
               ks.begin(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y);
            }

            mws_r.x += ts->crt_state.te->points[0].x - ts->prev_state.te->points[0].x;
            mws_r.y += ts->crt_state.te->points[0].y - ts->prev_state.te->points[0].y;

            if (mws_r.x > 0)
            {
               mws_r.x = 0;
            }
            else if (mws_r.x < -mws_r.w + pfm::screen::get_width())
            {
               mws_r.x = -mws_r.w + pfm::screen::get_width();
            }

            if (mws_r.y > 0)
            {
               mws_r.y = 0;
            }
            else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
            {
               mws_r.y = -mws_r.h + pfm::screen::get_height();
            }

            ts->process();
         }
         }

         break;
      }

      case touch_sym_evt::TS_MOUSE_WHEEL:
      {
         shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

         mws_r.y += float(mw->wheel_delta) * 50.f;

         if (mws_r.y > 0)
         {
            mws_r.y = 0;
         }
         else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
         {
            mws_r.y = -mws_r.h + pfm::screen::get_height();
         }

         ts->process();
         break;
      }

      case touch_sym_evt::TS_BACKWARD_SWIPE:
      {
         if (mws_r.x < 0)
         {
            ts->process();
         }
         else if (tmap.find(touch_sym_evt::TS_BACKWARD_SWIPE) != tmap.end())
         {
            send(get_mws_page_parent(), tmap[touch_sym_evt::TS_BACKWARD_SWIPE]);
            ts->process();
         }

         break;
      }

      case touch_sym_evt::TS_FORWARD_SWIPE:
      {
         if (mws_r.x > -mws_r.w + pfm::screen::get_width())
         {
            ts->process();
         }
         else if (tmap.find(touch_sym_evt::TS_FORWARD_SWIPE) != tmap.end())
         {
            send(get_mws_page_parent(), tmap[touch_sym_evt::TS_FORWARD_SWIPE]);
            ts->process();
         }

         break;
      }

      case touch_sym_evt::TS_UPWARD_SWIPE:
      {
         if (mws_r.y < 0)
         {
            ts->process();
         }
         else if (tmap.find(touch_sym_evt::TS_UPWARD_SWIPE) != tmap.end())
         {
            send(get_mws_page_parent(), tmap[touch_sym_evt::TS_UPWARD_SWIPE]);
            ts->process();
         }

         break;
      }

      case touch_sym_evt::TS_DOWNWARD_SWIPE:
      {
         if (mws_r.y > -mws_r.h + pfm::screen::get_height())
         {
            ts->process();
         }
         else if (tmap.find(touch_sym_evt::TS_DOWNWARD_SWIPE) != tmap.end())
         {
            send(get_mws_page_parent(), tmap[touch_sym_evt::TS_DOWNWARD_SWIPE]);
            ts->process();
         }

         break;
      }
      }
   }
}

void mws_page::update_state()
{
   point2d p = ks.update();

   mws_r.x += p.x;
   mws_r.y += p.y;

   for (auto b : mlist)
   {
      mws_r.w = std::max(mws_r.w, b->get_pos().w);
      mws_r.h = std::max(mws_r.h, b->get_pos().h);
   }

   if (mws_r.x > 0)
   {
      mws_r.x = 0;
   }
   else if (mws_r.x < -mws_r.w + pfm::screen::get_width())
   {
      mws_r.x = -mws_r.w + pfm::screen::get_width();
   }

   if (mws_r.y > 0)
   {
      mws_r.y = 0;
   }
   else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
   {
      mws_r.y = -mws_r.h + pfm::screen::get_height();
   }

   for (auto b : mlist)
   {
      b->update_state();
   }
}

void mws_page::update_view(shared_ptr<mws_camera> g)
{
   for (auto b : mlist)
   {
      b->update_view(g);
   }
}

shared_ptr<mws> mws_page::get_mws_at(int idx)
{
   return mlist[idx];
}

void mws_page::on_resize()
{
   shared_ptr<mws_page_tab> parent = get_mws_page_parent();

   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = parent->mws_r.w;
   mws_r.h = parent->mws_r.h;
}

shared_ptr<mws_page> mws_page::new_standalone_instance()
{
   return shared_ptr<mws_page>(new mws_page());
}

void mws_page::add(shared_ptr<mws_page_item> b)
{
   if (contains_mws(b))
   {
      mws_throw ia_exception();//trs("mwspageitem with id [%1%] already exists") % b->get_id());
   }

   mlist.push_back(b);
}


mws_page_item::mws_page_item() {}

void mws_page_item::set_rect(const mws_rect& i_rect)
{
   mws_r = i_rect;
}
void mws_page_item::set_size(float i_width, float i_height)
{
   mws_r.w = i_width;
   mws_r.h = i_height;
}

shared_ptr<mws_page> mws_page_item::get_mws_page_item_parent()
{
   return static_pointer_cast<mws_page>(get_mws_parent());
}

void mws_page_item::add_to_page()
{
   shared_ptr<mws_page> page = static_pointer_cast<mws_page>(get_mws_parent());
   shared_ptr<mws_page_item> inst = static_pointer_cast<mws_page_item>(get_instance());

   page->add(inst);
}

#endif
