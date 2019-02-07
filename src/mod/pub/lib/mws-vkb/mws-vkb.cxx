#include "stdafx.hxx"

#include "mws-vkb.hxx"
#include "jcv/vrn-diag.hxx"
#include "jcv/vrn-visual.hxx"
#include "mws-mod-ctrl.hxx"
#include "mws/mws-camera.hxx"
#include "mws/text-vxo.hxx"
#include "mws/mws-font.hxx"
#include "mws-mod.hxx"
#include "kxmd/kxmd.hxx"


mws_sp<mws_vkb> mws_vkb::inst;
mws_sp<mws_vkb> mws_vkb::gi()
{
   if (!inst)
   {
      inst = mws_sp<mws_vkb>(new mws_vkb());
      inst->setup();
   }

   return inst;
}

void mws_vkb::receive(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<mws_ptr_evt> pe = mws_ptr_evt::as_pointer_evt(i_dp);

      switch (pe->type)
      {
      case mws_ptr_evt::touch_began:
      {
         auto& pt = pe->points[0];
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
         key_types key_id = key_vect[ret.idx];

         if (key_id != VKB_DONE)
         {
            mws_mod_ctrl::inst()->key_action(KEY_PRESS, key_id);
         }
         break;
      }

      case mws_ptr_evt::touch_ended:
      {
         auto& pt = pe->points[0];
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

         current_key_idx = selected_kernel_idx = ret.idx;
         {
            auto vd = vk->get_diag_data();
            int id = vd->geom.kernel_points[selected_kernel_idx].id;
            key_types key_id = key_vect[selected_kernel_idx];
            std::string key_name = get_key_name(key_id);
            mws_println("selected idx [ %d] dist [ %f ] id [ %d ] name [ %s ]", ret.idx, ret.dist, id, key_name.c_str());

            if (key_id == VKB_DONE)
            {
               done();
            }
            else
            {
               mws_mod_ctrl::inst()->key_action(KEY_RELEASE, key_id);
            }
         }
         break;
      }
      }

      pe->process();
   }
   else if (i_dp->is_type(mws_key_evt::KEYEVT_EVT_TYPE))
   {
      ta->receive(i_dp);
   }
}

void mws_vkb::update_state()
{
}

void mws_vkb::update_view(mws_sp<mws_camera> g)
{
}

void mws_vkb::on_resize()
{
   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = (float)pfm::screen::get_width();
   mws_r.h = (float)pfm::screen::get_height();
}

void mws_vkb::set_target(mws_sp<mws_text_area> i_ta)
{
   ta = i_ta;

   if (vkb_filename.empty())
   {
      load("vk-00.kxmd");
   }
}

void mws_vkb::load(std::string i_filename)
{
   if (!vk)
   {
      vk = mws_vrn_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
      vk->toggle_voronoi_object(mws_vrn_obj_types::nexus_pairs | mws_vrn_obj_types::cells);
      vk->init();
      //vk->vgeom->position = glm::vec3(0.f, 0.f, 1.f);
      attach(vk->vgeom);
      vk_keys = text_vxo::nwi();
      attach(vk_keys);
      vk_keys->camera_id_list.clear();
      vk_keys->camera_id_list.push_back(mws_cam.lock()->camera_id);

      key_map =
      {
         {KEY_BACKSPACE, "backsp"},
         { KEY_ENTER, "enter" },
         { KEY_SHIFT, "shift" },
         { KEY_ESCAPE, "escape" },
         { KEY_END, "done" },
         { KEY_SPACE, "space" },
      };
   }

   vkb_filename = i_filename;
   mws_sp<std::vector<uint8> > res = get_mod()->storage.load_mod_byte_vect(vkb_filename);
   mws_sp<std::string> src(new std::string((const char*)begin_ptr(res), res->size()));
   mws_sp<kxmd_elem> kxmdi = kxmd::parse(src);

   // load key points
   {
      auto kernel_point_keys = kxmd::elem_at(kxmdi, "kernel-point-keys");
      key_vect.resize(kernel_point_keys->size());

      for (uint32 k = 0; k < kernel_point_keys->size(); k++)
      {
         const std::string& val = kernel_point_keys->vect[k]->val;
         uint32 key_id = mws_to<uint32>(val);

         mws_assert(key_id > KEY_INVALID && key_id < KEY_COUNT);
         key_vect[k] = key_types(key_id);
      }
   }
   // load kernel points
   {
      auto kernel_points = kxmd::elem_at(kxmdi, "kernel-points");
      std::vector<glm::vec2> kernel_points_pos;

      for (uint32 k = 0; k < kernel_points->size(); k++)
      {
         auto pos_pair = kernel_points->vect[k];
         float pos_0 = mws_to<float>(pos_pair->vect[0]->val);
         float pos_1 = mws_to<float>(pos_pair->vect[1]->val);
         kernel_points_pos.push_back(glm::vec2(pos_0, pos_1));
      }

      vk->set_kernel_points(kernel_points_pos);
      //vk->update_geometry();
   }
   // finish setup
   {
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;

      key_font = mws_font::nwi(48.f);
      //key_font->set_color(gfx_color::colors::slate_gray);
      key_font->set_color(gfx_color::colors::white);
      selected_key_font = mws_font::nwi(48.f);
      selected_key_font->set_color(gfx_color::colors::red);

      for (uint32 k = 0; k < key_vect.size(); k++)
      {
         mws_sp<mws_font> font = key_font;

         if (k == current_key_idx)
         {
            font = selected_key_font;
         }

         key_types key_id = key_vect[k];
         std::string key = get_key_name(key_id);
         auto& kp = kp_vect[k];
         glm::vec2 dim = font->get_text_dim(key);
         glm::vec2 pos(kp.position.x, kp.position.y);

         pos -= dim / 2.f;
         vk_keys->add_text(key, pos, font);
      }
   }

   mws_println("finished loading keyboard from [ %s ]", vkb_filename.c_str());
}

void mws_vkb::setup()
{
   mws_virtual_keyboard::setup();
   position = glm::vec3(position().x, position().y, 1.f);
   on_resize();
}

void mws_vkb::done()
{
   visible = false;
   ta = nullptr;
}

std::string mws_vkb::get_key_name(key_types i_key_id) const
{
   auto it = key_map.find(i_key_id);

   if (it != key_map.end())
   {
      return it->second;
   }

   if (mws_key_evt::is_ascii(i_key_id))
   {
      return std::string(1, i_key_id);
   }

   return "";
}

key_types mws_vkb::get_key_type(const std::string& i_key_name) const
{
   for (auto it = key_map.begin(); it != key_map.end(); ++it)
   {
      if (it->second == i_key_name)
      {
         return it->first;
      }
   }

   if (i_key_name.length() == 1)
   {
      char ch = i_key_name[0];

      if (mws_key_evt::is_ascii(ch))
      {
         return (key_types)ch;
      }
   }

   return KEY_INVALID;
}
