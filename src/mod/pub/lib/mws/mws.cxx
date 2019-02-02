#include "stdafx.hxx"

#include "appplex-conf.hxx"

#if defined MOD_MWS

#include "mws.hxx"
#include "mws-camera.hxx"
#include "mws-font.hxx"
#include "text-vxo.hxx"
#include "vkb/mws-vkb.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "gfx-vxo.hxx"
#include "mws-mod.hxx"
#include "input/transitions.hxx"
#include <algorithm>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


mws_rect::mws_rect() {}
mws_rect::mws_rect(float i_x, float i_y, float i_w, float i_h)
{
   set(i_x, i_y, i_w, i_h);
}

void mws_rect::set(float i_x, float i_y, float i_w, float i_h)
{
   x = i_x;
   y = i_y;
   w = i_w;
   h = i_h;
}


mws_sp<mws_model> mws_model::get_instance()
{
   return shared_from_this();
}

void mws_model::receive(mws_sp<mws_dp> idp)
{
}

void mws_model::set_view(mws_sp<mws> iview)
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

mws_sp<mws> mws_model::get_view()
{
   return view.lock();
}

mws_sp<mws_sender> mws_model::sender_inst()
{
   return get_instance();
}


mws_sp<mws> mws::nwi()
{
   auto inst = mws_sp<mws>(new mws());
   inst->setup();
   return inst;
}

mws::mws(mws_sp<gfx> i_gi) : gfx_node(i_gi)
// for rootless / parentless mws inst
{
   visible = true;
}

mws_sp<mws> mws::get_instance()
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

void mws::attach(mws_sp<gfx_node> i_node)
{
   gfx_node::attach(i_node);

   if (i_node->get_type() == gfx_obj::e_mws)
   {
      auto mws_ref = mws_dynamic_pointer_cast<mws>(i_node);

      if (mws_ref)
      {
         mws_ref->mwsroot = mwsroot;
         mws_ref->mws_cam = mwsroot.lock()->get_mod()->mws_cam;
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

void mws::set_enabled(bool i_is_enabled)
{
   enabled = i_is_enabled;
}

bool mws::is_enabled() const
{
   return enabled;
}

void mws::set_visible(bool iis_visible)
{
   visible = iis_visible;
}

bool mws::is_visible()const
{
   return visible;
}

void mws::set_id(std::string iid)
{
   id = iid;
}

const std::string& mws::get_id()
{
   return id;
}

mws_sp<mws> mws::find_by_id(const std::string& iid)
{
   return mwsroot.lock()->contains_id(iid);
}

mws_sp<mws> mws::contains_id(const std::string& iid)
{
   if (iid == id)
   {
      return get_instance();
   }

   return nullptr;
}

bool mws::contains_mws(const mws_sp<mws> i_mws)
{
   return i_mws == get_instance();
}

mws_sp<mws> mws::get_mws_parent()
{
   return mws_dynamic_pointer_cast<mws>(get_parent());
}

mws_sp<mws_page_tab> mws::get_mws_root()
{
   return mwsroot.lock();
}

mws_sp<mws_mod> mws::get_mod()
{
   return std::static_pointer_cast<mws_mod>(mwsroot.lock()->get_mod());
}

void mws::receive(mws_sp<mws_dp> idp)
{
   if (receive_handler)
   {
      receive_handler(get_instance(), idp);
   }
   else
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
}

void mws::set_receive_handler(std::function<void(mws_sp<mws> i_mws, mws_sp<mws_dp> i_idp)> i_receive_handler)
{
   receive_handler = i_receive_handler;
}

void mws::update_state()
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

         if (w && w->visible)
         {
            w->update_state();
         }
      }
   }
}

void mws::update_view(mws_sp<mws_camera> g)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

         if (w && w->visible)
         {
            w->update_view(g);
         }
      }
   }
}

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


mws_sp<mws_sender> mws::sender_inst()
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
   mws::add_to_draw_list(i_camera_id, i_opaque, i_translucent);
}

void mws_page_tab::init()
{
   mwsroot = get_mws_page_tab_instance();
   mws_cam = mwsroot.lock()->get_mod()->mws_cam;
}

void mws_page_tab::init_subobj()
{
   auto z_sort = [](mws_sp<mws> a, mws_sp<mws> b)
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
}

void mws_page_tab::on_destroy()
{
   for (auto p : page_tab)
   {
      p->on_destroy();
   }
}

mws_sp<mws> mws_page_tab::contains_id(const std::string& iid)
{
   if (iid.length() > 0)
   {
      if (iid == get_id())
      {
         return get_instance();
      }

      for (auto p : page_tab)
      {
         mws_sp<mws> u = p->contains_id(iid);

         if (u)
         {
            return u;
         }
      }
   }

   return mws_sp<mws>();
}

bool mws_page_tab::contains_mws(const mws_sp<mws> i_mws)
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

mws_sp<text_vxo> mws_page_tab::get_text_vxo() const
{
   return tab_text_vxo;
}

void mws_page_tab::receive(mws_sp<mws_dp> idp)
{
   if (vkb && vkb->visible)
   {
      vkb->receive(idp);
   }
   else
   {
      if (receive_handler)
      {
         receive_handler(get_instance(), idp);
      }
      else
      {
         if (idp->is_processed())
         {
            return;
         }

         if (!is_empty())
         {
            send(page_tab.back(), idp);
         }
      }
   }
}

void mws_page_tab::update_state()
{
   if (vkb)
   {
      vkb->update_state();
   }

   if (tab_text_vxo)
   {
      tab_text_vxo->clear_text();
   }

   for (auto p : page_tab)
   {
      if (p->visible)
      {
         p->update_state();
      }
   }
}

void mws_page_tab::update_view(mws_sp<mws_camera> g)
{
   if (vkb)
   {
      vkb->update_view(g);
   }

   for (auto p : page_tab)
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

   for (auto p : page_tab)
   {
      p->on_resize();
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

#if defined MOD_VECTOR_FONTS
   {
      auto tab_text_vxo = text_vxo::nwi();

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

void mws_page_tab::show_keyboard(mws_sp<mws_text_area> i_tbx)
{
#if defined MOD_VKB

   mws_println("mws_page_tab::show_keyboard");

   if (!vkb)
   {
      vkb = mws_vkb::gi();
      attach(vkb);
   }

   vkb->visible = true;
   vkb->set_target(i_tbx);

#endif
}


mws_page::mws_page()
{
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
   for (auto p : mlist)
   {
      p->on_destroy();
   }
}

mws_sp<mws> mws_page::contains_id(const std::string& iid)
{
   if (iid.length() > 0)
   {
      if (iid == get_id())
      {
         return get_instance();
      }

      for (auto p : mlist)
      {
         mws_sp<mws> u = p->contains_id(iid);

         if (u)
         {
            return u;
         }
      }
   }

   return mws_sp<mws>();
}

bool mws_page::contains_mws(const mws_sp<mws> i_mws)
{
   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

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

mws_sp<mws_page_tab> mws_page::get_mws_page_parent()
{
   return static_pointer_cast<mws_page_tab>(get_mws_parent());
}

void mws_page::on_visibility_changed(bool iis_visible) {}
void mws_page::on_show_transition(const mws_sp<linear_transition> itransition) {}
void mws_page::on_hide_transition(const mws_sp<linear_transition> itransition) {}

void mws_page::receive(mws_sp<mws_dp> idp)
{
   if (receive_handler)
   {
      receive_handler(get_instance(), idp);
   }
   else
   {
      update_input_sub_mws(idp);
      //update_input_std_behaviour(idp);
   }
}

void mws_page::update_input_sub_mws(mws_sp<mws_dp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);
      static auto z_sort = [](mws_sp<gfx_node> a, mws_sp<gfx_node> b)
      {
         auto& pos_0 = gfx_util::get_pos_from_tf_mx(a->get_global_tf_mx());
         auto& pos_1 = gfx_util::get_pos_from_tf_mx(b->get_global_tf_mx());

         return (pos_0.z > pos_1.z);
      };

      std::sort(children.begin(), children.end(), z_sort);
      selected_item = nullptr;

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
                  selected_item = w;
                  break;
               }
            }
         }
      }
   }
   else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      if (selected_item)
      {
         selected_item->receive(idp);
      }
   }

}

void mws_page::update_input_std_behaviour(mws_sp<mws_dp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);

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
      //      else if (mws_r.x < -mws_r.w + pfm::screen::get_width())
      //      {
      //         mws_r.x = -mws_r.w + pfm::screen::get_width();
      //      }

      //      if (mws_r.y > 0)
      //      {
      //         mws_r.y = 0;
      //      }
      //      else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
      //      {
      //         mws_r.y = -mws_r.h + pfm::screen::get_height();
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
      //   else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
      //   {
      //      mws_r.y = -mws_r.h + pfm::screen::get_height();
      //   }

      //   ts->process();
      //   break;
      //}
      //}
   }
}

void mws_page::update_state()
{
   //point2d p = ks.update();

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
   //else if (mws_r.x < -mws_r.w + pfm::screen::get_width())
   //{
   //   mws_r.x = -mws_r.w + pfm::screen::get_width();
   //}

   //if (mws_r.y > 0)
   //{
   //   mws_r.y = 0;
   //}
   //else if (mws_r.y < -mws_r.h + pfm::screen::get_height())
   //{
   //   mws_r.y = -mws_r.h + pfm::screen::get_height();
   //}

   for (auto& c : children)
   {
      if (c->get_type() == gfx_obj::e_mws)
      {
         auto w = mws_dynamic_pointer_cast<mws>(c);

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
         auto w = mws_dynamic_pointer_cast<mws>(c);

         if (w && w->visible)
         {
            w->update_view(g);
         }
      }
   }
}

mws_sp<mws> mws_page::get_mws_at(int idx)
{
   return mlist[idx];
}

void mws_page::select(mws_sp<mws> i_item)
{
   if (contains_mws(i_item))
   {
      selected_item = i_item;
   }
   else
   {
      mws_println("mws_page::select[ i_item is not in mws_page ]");
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

mws_sp<mws_page> mws_page::new_standalone_instance()
{
   return mws_sp<mws_page>(new mws_page());
}

void mws_page::add(mws_sp<mws_page_item> b)
{
   if (contains_mws(b))
   {
      mws_throw mws_exception();//trs("mwspageitem with id [%1%] already exists") % b->get_id());
   }

   mlist.push_back(b);
}


void mws_page_item::set_rect(const mws_rect& i_rect)
{
   mws_r = i_rect;
}

void mws_page_item::set_size(float i_width, float i_height)
{
   mws_r.w = i_width;
   mws_r.h = i_height;
}

mws_sp<mws_page> mws_page_item::get_mws_page_item_parent()
{
   return static_pointer_cast<mws_page>(get_mws_parent());
}

void mws_page_item::select()
{
   get_mws_page_item_parent()->select(get_instance());
}

mws_page_item::mws_page_item() {}

void mws_page_item::setup()
{
   mws::setup();
}

void mws_page_item::add_to_page()
{
   mws_sp<mws_page> page = static_pointer_cast<mws_page>(get_mws_parent());
   mws_sp<mws_page_item> inst = static_pointer_cast<mws_page_item>(get_instance());

   page->add(inst);
}

#endif
