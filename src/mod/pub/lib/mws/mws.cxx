#include "stdafx.hxx"

#include "appplex-conf.hxx"

#include "mws.hxx"
#include "mws-com.hxx"
#include "mws-camera.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-text-vxo.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "gfx-vxo.hxx"
#include "mws-mod.hxx"
#include "input/transitions.hxx"
#include "mws-vkb/mws-vkb.hxx"
#include "mod-list.hxx"
#include <algorithm>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


mws_sp<mws_model> mws_model::get_instance()
{
   return shared_from_this();
}

void mws_model::receive(mws_sp<mws_dp> i_dp)
{
}

void mws_model::set_view(mws_sp<mws_obj> iview)
{
   view = iview;
   notify_update();
}

void mws_model::notify_update()
{
   if (get_view())
   {
      send(get_view(), mws_dp::nwi(MWS_EVT_MODEL_UPDATE));
   }
}

mws_sp<mws_obj> mws_model::get_view()
{
   return view.lock();
}

mws_sp<mws_sender> mws_model::sender_inst()
{
   return get_instance();
}


mws_sp<mws_obj> mws_obj::nwi()
{
   auto inst = mws_sp<mws_obj>(new mws_obj());
   inst->setup();
   return inst;
}

mws_obj::mws_obj(mws_sp<gfx> i_gi) : gfx_node(i_gi)
// for rootless / parentless mws inst
{
   visible = true;
}

mws_sp<mws_obj> mws_obj::get_instance()
{
   return std::static_pointer_cast<mws_obj>(get_inst());
}

gfx_obj::e_gfx_obj_type mws_obj::get_type()const
{
   return gfx_obj::e_mws;
}

void mws_obj::add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   if (visible)
   {
      auto cam = mws_cam.lock();

      if (cam->camera_id() == i_camera_id)
      {
         for (auto it = children.begin(); it != children.end(); ++it)
         {
            if ((*it)->visible)
            {
               (*it)->add_to_draw_list(i_camera_id, i_opaque, i_translucent);
            }
         }
      }
   }
}

void mws_obj::attach(mws_sp<gfx_node> i_node)
{
   if (i_node->get_type() == gfx_obj::e_mws)
   {
      auto mws_ref = mws_dynamic_pointer_cast<mws_obj>(i_node);

      if (mws_ref)
      {
         mws_ref->mwsroot = mwsroot;
         mws_ref->mws_cam = mwsroot.lock()->get_mod()->mws_cam;
      }
   }

   i_node->position = i_node->position + glm::vec3(0.f, 0.f, 0.0001f);
   gfx_node::attach(i_node);
}

void mws_obj::list_mws_children(std::vector<mws_sp<mws_obj> >& i_mws_subobj_list)
{
   for (auto c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         i_mws_subobj_list.push_back(w);
         w->list_mws_children(i_mws_subobj_list);
      }
   }
}

void mws_obj::set_enabled(bool i_is_enabled)
{
   enabled = i_is_enabled;
}

bool mws_obj::is_enabled() const
{
   return enabled;
}

void mws_obj::set_visible(bool iis_visible)
{
   visible = iis_visible;
}

bool mws_obj::is_visible()const
{
   return visible;
}

void mws_obj::set_id(std::string i_id)
{
   id = i_id;
}

const std::string& mws_obj::get_id()
{
   return id;
}

mws_sp<mws_obj> mws_obj::find_by_id(const std::string& i_id)
{
   return mwsroot.lock()->contains_id(i_id);
}

mws_sp<mws_obj> mws_obj::contains_id(const std::string& i_id)
{
   if (i_id == id)
   {
      return get_instance();
   }
   else
   {
      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            auto w = mws_dynamic_pointer_cast<mws_obj>(c);

            if (w->id == i_id)
            {
               return w;
            }

            auto w2 = w->contains_id(i_id);

            if (w2)
            {
               return w2;
            }
         }
      }

      return nullptr;
   }
}

bool mws_obj::contains_mws(const mws_sp<mws_obj> i_mws)
{
   if (i_mws == get_instance())
   {
      return true;
   }
   else
   {
      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            auto w = mws_dynamic_pointer_cast<mws_obj>(c);

            if (w == i_mws || w->contains_mws(i_mws))
            {
               return true;
            }
         }
      }

      return false;
   }
}

mws_sp<mws_obj> mws_obj::get_mws_parent() const
{
   return mws_dynamic_pointer_cast<mws_obj>(get_parent());
}

mws_sp<mws_page_tab> mws_obj::get_mws_root() const
{
   return mwsroot.lock();
}

mws_sp<mws_mod> mws_obj::get_mod() const
{
   return std::static_pointer_cast<mws_mod>(mwsroot.lock()->get_mod());
}

void mws_obj::process(mws_sp<mws_dp> i_dp) { i_dp->process(get_instance()); }

void mws_obj::receive(mws_sp<mws_dp> i_dp)
{
   if (receive_handler)
   {
      receive_handler(get_instance(), i_dp);
   }
   else
   {
      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            auto w = mws_dynamic_pointer_cast<mws_obj>(c);

            if (w && w->visible)
            {
               send(w, i_dp);

               if (i_dp->is_processed())
               {
                  break;
               }
            }
         }
      }
   }
}

void mws_obj::update_state()
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         if (w && w->visible)
         {
            w->update_state();
         }
      }
   }
}

void mws_obj::update_view(mws_sp<mws_camera> g)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         if (w && w->visible)
         {
            w->update_view(g);
         }
      }
   }
}

const mws_size& mws_obj::get_best_size() const
{
   return best_size;
}

void mws_obj::set_best_size(const mws_size& i_size)
{
   best_size = i_size;
}

mws_rect mws_obj::get_pos()
{
   return mws_r;
}

float mws_obj::get_z()
{
   return position().z;
}

void mws_obj::set_z(float i_z_position)
{
   position = glm::vec3(position().x, position().y, i_z_position);
}


mws_sp<mws_sender> mws_obj::sender_inst()
{
   return get_instance();
}


mws_page_tab::mws_page_tab(mws_sp<mws_mod> i_mod)
{
   if (!i_mod)
   {
      mws_throw mws_exception("mod cannot be null");
   }

   u = i_mod;
   mws_r.set(0, 0, (float)i_mod->get_width(), (float)i_mod->get_height());
}

mws_sp<mws_page_tab> mws_page_tab::nwi(mws_sp<mws_mod> i_mod)
{
   mws_sp<mws_page_tab> pt(new mws_page_tab(i_mod));
   pt->new_instance_helper();
   return pt;
}

void mws_page_tab::add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   mws_obj::add_to_draw_list(i_camera_id, i_opaque, i_translucent);
}

void mws_page_tab::init()
{
   mwsroot = get_mws_page_tab_instance();
   mws_cam = mwsroot.lock()->get_mod()->mws_cam;
}

void mws_page_tab::init_subobj()
{
   auto z_sort = [](mws_sp<mws_obj> a, mws_sp<mws_obj> b)
   {
      return (a->get_z() > b->get_z());
   };

   for (auto p : page_tab)
   {
      p->init();
      p->mws_subobj_list.clear();
      p->list_mws_children(p->mws_subobj_list);
      std::sort(p->mws_subobj_list.begin(), p->mws_subobj_list.end(), z_sort);
   }

   if (page_nav && !(page_nav->get_main_page_id().empty()))
   {
      page_nav->reset_pages();
   }
}

void mws_page_tab::on_destroy()
{
   for (auto p : page_tab)
   {
      p->on_destroy();
   }
}

mws_sp<mws_obj> mws_page_tab::contains_id(const std::string& i_id)
{
   if (i_id.length() > 0)
   {
      if (i_id == get_id())
      {
         return get_instance();
      }

      for (auto p : page_tab)
      {
         mws_sp<mws_obj> u = p->contains_id(i_id);

         if (u)
         {
            return u;
         }
      }
   }

   return mws_sp<mws_obj>();
}

bool mws_page_tab::contains_mws(const mws_sp<mws_obj> i_mws)
{
   for (auto p : page_tab)
   {
      if (i_mws == p || p->contains_mws(i_mws))
      {
         return true;
      }
   }

   return false;
}

mws_sp<mws_page_tab> mws_page_tab::get_mws_page_tab_instance()
{
   return static_pointer_cast<mws_page_tab>(get_instance());
}

mws_sp<mws_mod> mws_page_tab::get_mod()
{
   return static_pointer_cast<mws_mod>(u.lock());
}

bool mws_page_tab::is_empty()
{
   return page_tab.empty();
}

mws_sp<mws_text_vxo> mws_page_tab::get_text_vxo() const
{
   return tab_text_vxo;
}

void mws_page_tab::receive(mws_sp<mws_dp> i_dp)
{
   if (vkb && vkb->visible)
   {
      vkb->receive(i_dp);
   }

   if (!i_dp->is_processed())
   {
      if (receive_handler)
      {
         receive_handler(get_instance(), i_dp);
      }
      else
      {
         if (i_dp->is_processed())
         {
            return;
         }

         if (!is_empty())
         {
            send(page_tab.back(), i_dp);
         }
      }
   }
}

void mws_page_tab::update_state()
{
   if (vkb && vkb->visible)
   {
      vkb->update_state();
   }

   if (tab_text_vxo)
   {
      tab_text_vxo->clear_text();
   }

   for (mws_sp<mws_page> p : page_tab)
   {
      if (p->visible)
      {
         p->update_state();
      }
   }
}

void mws_page_tab::update_view(mws_sp<mws_camera> g)
{
   if (vkb && vkb->visible)
   {
      vkb->update_view(g);
   }

   for (mws_sp<mws_page> p : page_tab)
   {
      if (p->visible)
      {
         p->update_view(g);
      }
   }
}

void mws_page_tab::on_resize()
{
   mws_r.w = (float)u.lock()->get_width();
   mws_r.h = (float)u.lock()->get_height();

   if (tab_text_vxo)
   {
      tab_text_vxo->clear_text();
   }

   for (mws_sp<mws_page> p : page_tab)
   {
      if (p->visible)
      {
         p->on_resize();
      }
   }

   if (vkb)
   {
      vkb->on_resize();
   }
}

void mws_page_tab::add_page(mws_sp<mws_page> i_page)
{
   attach(i_page);
   add(i_page);
}

void mws_page_tab::add(mws_sp<mws_page> p)
{
   if (contains_mws(p))
   {
      mws_throw mws_exception();//trs("page with id [%1%] already exists") % p->get_id());
   }

   page_tab.push_back(p);
}

int mws_page_tab::get_page_index(mws_sp<mws_page> ipage)
{
   int k = 0;

   for (auto p : page_tab)
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
   mws_sp<mws_page_tab> mws_root = get_mws_page_tab_instance();
   mwsroot = mws_root;
   mws_cam = mwsroot.lock()->get_mod()->mws_cam;
   page_nav = mws_stack_page_nav::nwi(mws_root);

#if defined MOD_VECTOR_FONTS
   {
      auto tab_text_vxo = mws_text_vxo::nwi();

      mws_root->tab_text_vxo = tab_text_vxo;
      mws_root->attach(tab_text_vxo);
      tab_text_vxo->camera_id_list.clear();
      tab_text_vxo->camera_id_list.push_back("mws_cam");
   }
#endif
}

bool mws_page_tab::handle_back_evt()
{
   if (vkb && vkb->visible)
   {
      vkb->visible = false;
      vkb->set_target(nullptr);

      return true;
   }

   return false;
}

mws_sp<mws_virtual_keyboard> mws_page_tab::get_keyboard()
{
   if (mod_mws_vkb_on)
   {
      if (!vkb)
      {
         vkb = mws_vkb::gi();
         vkb->set_file_store(get_file_store());
         attach(vkb);
         vkb->visible = false;
      }

      return vkb;
   }

   return nullptr;
}

void mws_page_tab::show_keyboard(mws_sp<mws_text_area> i_tbx)
{
   if (mod_mws_vkb_on)
   {
      auto kb = get_keyboard();

      kb->visible = true;
      kb->set_target(i_tbx);
   }
}

mws_sp<mws_vkb_file_store> mws_page_tab::get_file_store()
{
#if MOD_MWS_VKB
   if (!vkb_store)
   {
      set_file_store(std::make_shared<mws_vkb_file_store_impl>(get_mod()));
   }
#endif

   return vkb_store;
}


mws_page::mws_page()
{
   on_visibility_changed = [this](bool i_visible)
   {
      if (i_visible)
      {
         mws_sp<mws_page_tab> parent = get_mws_page_parent();

         if (mws_r.w != parent->mws_r.w || mws_r.h != parent->mws_r.h)
         {
            on_resize();
         }
      }
   };
}

mws_sp<mws_page> mws_page::nwi(mws_sp<mws_page_tab> i_parent)
{
   mws_sp<mws_page> u(new mws_page());
   i_parent->attach(u);
   i_parent->add(u);
   return u;
}

void mws_page::init() {}

void mws_page::on_destroy()
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         w->on_destroy();
      }
   }
}

mws_sp<mws_obj> mws_page::contains_id(const std::string& i_id)
{
   if (i_id.length() > 0)
   {
      if (i_id == get_id())
      {
         return get_instance();
      }

      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            auto w = mws_dynamic_pointer_cast<mws_obj>(c);

            mws_sp<mws_obj> u = w->contains_id(i_id);

            if (u)
            {
               return u;
            }
         }
      }
   }

   return nullptr;
}

bool mws_page::contains_mws(const mws_sp<mws_obj> i_mws)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         if (w == i_mws || w->contains_mws(i_mws))
         {
            return true;
         }
      }
   }

   return false;
}

mws_sp<mws_page> mws_page::get_mws_page_instance()
{
   return static_pointer_cast<mws_page>(get_instance());
}

mws_sp<mws_page_tab> mws_page::get_mws_page_parent() const
{
   return static_pointer_cast<mws_page_tab>(get_mws_parent());
}

void mws_page::on_show_transition(const mws_sp<linear_transition> i_transition) {}

void mws_page::on_hide_transition(const mws_sp<linear_transition> i_transition) {}

void mws_page::receive(mws_sp<mws_dp> i_dp)
{
   if (receive_handler)
   {
      receive_handler(get_instance(), i_dp);
   }
   else
   {
      update_input_sub_mws(i_dp);
      //update_input_std_behaviour(i_dp);
   }
}

void mws_page::update_input_sub_mws(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_processed())
   {
      return;
   }

   if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
   {
      mws_sp<mws_obj> new_selected_item;
      mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);
      static auto z_sort = [](mws_sp<gfx_node> a, mws_sp<gfx_node> b)
      {
         auto& pos_0 = gfx_util::get_pos_from_tf_mx(a->get_global_tf_mx());
         auto& pos_1 = gfx_util::get_pos_from_tf_mx(b->get_global_tf_mx());

         return (pos_0.z > pos_1.z);
      };

      std::sort(children.begin(), children.end(), z_sort);

      for (auto& c : children)
      {
         if (c->get_type() == gfx_obj::e_mws)
         {
            mws_sp<mws_obj> w = mws_dynamic_pointer_cast<mws_obj>(c);

            if (w && w->visible)
            {
               send(w, i_dp);

               if (i_dp->is_processed())
               {
                  new_selected_item = mws_dynamic_pointer_cast<mws_obj>(i_dp->destination());
                  break;
               }
            }
         }
      }

      if (selected_item != new_selected_item)
      {
         if (selected_item)
         {
            selected_item->on_focus_changed(false);
         }

         selected_item = new_selected_item;

         if (selected_item)
         {
            selected_item->on_focus_changed(true);
         }
      }
   }
   else if (i_dp->is_type(mws_key_evt::key_evt_type))
   {
      if (selected_item)
      {
         selected_item->receive(i_dp);
      }
   }

}

void mws_page::update_input_std_behaviour(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_processed())
   {
      return;
   }

   if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
   {
      mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);

      //switch (ts->get_type())
      //{
      //case touch_sym_evt::TS_PRESSED:
      //{
      //   float x = ts->pressed.te->points[0].x;
      //   float y = ts->pressed.te->points[0].y;

      //   if (ts->tap_count == 1)
      //   {
      //      ks.grab(x, y);
      //      ts->process();
      //   }

      //   break;
      //}

      //case touch_sym_evt::TS_PRESS_AND_DRAG:
      //{
      //   float x = ts->points[0].x;
      //   float y = ts->points[0].y;

      //   switch (ts->tap_count)
      //   {
      //   case 1:
      //   {
      //      if (ts->is_finished)
      //      {
      //         ks.start_slowdown();
      //      }
      //      else
      //      {
      //         ks.begin(ts->points[0].x, ts->points[0].y);
      //      }

      //      mws_r.x += ts->points[0].x - ts->prev_state.te->points[0].x;
      //      mws_r.y += ts->points[0].y - ts->prev_state.te->points[0].y;

      //      if (mws_r.x > 0)
      //      {
      //         mws_r.x = 0;
      //      }
      //      else if (mws_r.x < -mws_r.w + mws::screen::get_width())
      //      {
      //         mws_r.x = -mws_r.w + mws::screen::get_width();
      //      }

      //      if (mws_r.y > 0)
      //      {
      //         mws_r.y = 0;
      //      }
      //      else if (mws_r.y < -mws_r.h + mws::screen::get_height())
      //      {
      //         mws_r.y = -mws_r.h + mws::screen::get_height();
      //      }

      //      ts->process();
      //   }
      //   }

      //   break;
      //}

      //case touch_sym_evt::TS_MOUSE_WHEEL:
      //{
      //   mws_sp<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

      //   mws_r.y += float(mw->wheel_delta) * 50.f;

      //   if (mws_r.y > 0)
      //   {
      //      mws_r.y = 0;
      //   }
      //   else if (mws_r.y < -mws_r.h + mws::screen::get_height())
      //   {
      //      mws_r.y = -mws_r.h + mws::screen::get_height();
      //   }

      //   ts->process();
      //   break;
      //}
      //}
   }
}

void mws_page::update_state()
{
   //glm::vec2 p = ks.update();

   //mws_r.x += p.x;
   //mws_r.y += p.y;

   //for (auto b : mlist)
   //{
   //   mws_r.w = std::max(mws_r.w, b->get_pos().w);
   //   mws_r.h = std::max(mws_r.h, b->get_pos().h);
   //}

   //if (mws_r.x > 0)
   //{
   //   mws_r.x = 0;
   //}
   //else if (mws_r.x < -mws_r.w + mws::screen::get_width())
   //{
   //   mws_r.x = -mws_r.w + mws::screen::get_width();
   //}

   //if (mws_r.y > 0)
   //{
   //   mws_r.y = 0;
   //}
   //else if (mws_r.y < -mws_r.h + mws::screen::get_height())
   //{
   //   mws_r.y = -mws_r.h + mws::screen::get_height();
   //}

   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         if (w && w->visible)
         {
            w->update_state();
         }
      }
   }
}

void mws_page::update_view(mws_sp<mws_camera> g)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws_obj>(c);

         if (w && w->visible)
         {
            w->update_view(g);
         }
      }
   }
}

glm::vec2 mws_page::get_dim() const
{
   return glm::vec2(mws_r.w - mws_r.x, mws_r.h - mws_r.y);
}

mws_sp<mws_obj> mws_page::get_mws_at(uint32 i_idx)
{
   uint32 k = 0;

   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         if (k == i_idx)
         {
            auto w = mws_dynamic_pointer_cast<mws_obj>(c);
            return w;
         }

         k++;
      }
   }

   return nullptr;
}

bool mws_page::is_selected(mws_sp<mws_obj> i_item)
{
   return i_item == selected_item;
}

void mws_page::set_focus(mws_sp<mws_obj> i_item, bool i_set_focus)
{
   if (contains_mws(i_item))
   {
      if (i_set_focus && (i_item != selected_item))
      {
         if (selected_item)
         {
            selected_item->on_focus_changed(false);
         }

         selected_item = i_item;
         selected_item->on_focus_changed(true);
      }
      else
      {
         if (i_item == selected_item)
         {
            selected_item->on_focus_changed(false);
            selected_item = nullptr;
         }
         else
         {
            mws_println("mws_page::set_focus[ i_item was not focused ]");
         }
      }
   }
   else
   {
      mws_println("mws_page::set_focus[ i_item is not in mws_page ]");
   }
}

void mws_page::on_resize()
{
   mws_sp<mws_page_tab> parent = get_mws_page_parent();

   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = parent->mws_r.w;
   mws_r.h = parent->mws_r.h;
}


void mws_page_item::set_rect(const mws_rect& i_rect)
{
   set_position(glm::vec2(i_rect.x, i_rect.y));
   set_size(glm::vec2(i_rect.w, i_rect.h));
}

void mws_page_item::set_position(const glm::vec2& i_position)
{
   mws_r.x = i_position.x;
   mws_r.y = i_position.y;
}

void mws_page_item::set_size(const glm::vec2& i_size)
{
   mws_r.w = i_size.x;
   mws_r.h = i_size.y;
}

mws_sp<mws_page> mws_page_item::get_page()
{
   mws_sp<gfx_node> parent = get_parent();

   while (parent)
   {
      if (parent->get_type() == gfx_obj::e_mws)
      {
         mws_sp<mws_page> page = mws_dynamic_pointer_cast<mws_page>(parent);

         if (page)
         {
            return page;
         }
      }

      parent = parent->get_parent();
   }

   return nullptr;
}

bool mws_page_item::has_focus()
{
   mws_sp<mws_page> parent = get_page();
   return parent->is_selected(get_instance());
}

void mws_page_item::set_focus(bool i_set_focus)
{
   get_page()->set_focus(get_instance(), i_set_focus);
}

mws_page_item::mws_page_item() {}

void mws_page_item::setup()
{
   mws_obj::setup();
}
