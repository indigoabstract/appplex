#include "stdafx.hxx"

#include "mws-vkb.hxx"
#include "mws-vkb-visual.hxx"
#include "jcv/vrn-diag.hxx"
#include "mws-mod-ctrl.hxx"
#include "mws/mws-camera.hxx"
#include "mws/text-vxo.hxx"
#include "mws/mws-font.hxx"
#include "mws-mod.hxx"
#include "kxmd/kxmd.hxx"
#include <numeric>


mws_vkb_impl::mws_vkb_impl(uint32 i_obj_type_mask)
{
   obj_type_mask = i_obj_type_mask;
}

std::string mws_vkb_impl::get_map_filename(uint32 i_map_idx)
{
   uint32 w = pfm::screen::get_width();
   uint32 h = pfm::screen::get_height();
   uint32 gcd = std::gcd(w, h);
   uint32 wd = w / gcd;
   uint32 hd = h / gcd;

   return trs("{0}{1}x{2}-{3}{4}", VKB_PREFIX, wd, hd, i_map_idx, VKB_EXT);
}

void mws_vkb_impl::setup()
{
   key_map =
   {
      {KEY_BACKSPACE, "backsp"},
      {VKB_ENTER, "enter"},
      {VKB_SHIFT, "shift"},
      {VKB_ESCAPE, "escape"},
      {VKB_DONE, "done"},
      {VKB_HIDE_KB, "hide"},
      {VKB_DELETE, "delete"},
      {KEY_SPACE, "space"},
   };
   vk = mws_vkb_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
   vk->toggle_voronoi_object(obj_type_mask);
   //vk->toggle_voronoi_object(mws_vrn_obj_types::kernel_points | mws_vrn_obj_types::nexus_points | mws_vrn_obj_types::nexus_pairs | mws_vrn_obj_types::cells);
   vk->init();
   vk->vgeom->position = glm::vec3(0.f, 0.f, -1.f);
   //vk->vgen->random_points();
   //vk->update_diag();
   //load_map(get_map_filename());
   attach(vk->vgeom);
   key_font = mws_font::nwi(48.f);
   //key_font->set_color(gfx_color::colors::slate_gray);
   key_font->set_color(gfx_color::colors::white);
   selected_key_font = mws_font::nwi(48.f);
   selected_key_font->set_color(gfx_color::colors::red);
   //set_random_keys();
   // finish setup
   {
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;

      key_font = mws_font::nwi(48.f);
      //key_font->set_color(gfx_color::colors::slate_gray);
      key_font->set_color(gfx_color::colors::white);
      selected_key_font = mws_font::nwi(48.f);
      selected_key_font->set_color(gfx_color::colors::red);

      //for (uint32 k = 0; k < key_vect.size(); k++)
      //{
      //   mws_sp<mws_font> font = key_font;

      //   if (k == current_key_idx)
      //   {
      //      font = selected_key_font;
      //   }

      //   key_types key_id = key_vect[k];
      //   std::string key = get_key_name(key_id);
      //   auto& kp = kp_vect[k];
      //   glm::vec2 dim = font->get_text_dim(key);
      //   glm::vec2 pos(kp.position.x, kp.position.y);

      //   pos -= dim / 2.f;
      //   vk_keys->add_text(key, pos, font);
      //}
      vk_keys = text_vxo::nwi();
      attach(vk_keys);
      vk_keys->camera_id_list.clear();
      vk_keys->camera_id_list.push_back(mws_cam.lock()->camera_id);
   }
}

void mws_vkb_impl::update_state()
{
   if (keys_visible)
   {
      auto root = get_mws_root();
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;
      uint32 size = get_key_vect_size();
      bool shift_held = get_mod()->key_ctrl_inst->key_is_held(KEY_SHIFT);
      key_mod = (shift_held) ? key_mod_types::mod_shift : key_mod_types::mod_none;
      vk_keys->clear_text();

      for (uint32 k = 0; k < size; k++)
      {
         mws_sp<mws_font> font = key_font;

         if (k == current_key_idx)
         {
            font = selected_key_font;
         }

         key_types key_id = get_key_at(k);
         std::string key = get_key_name(key_id);
         auto& kp = kp_vect[k];
         glm::vec2 dim = font->get_text_dim(key);
         glm::vec2 pos(kp.position.x, kp.position.y);

         pos -= dim / 2.f;
         vk_keys->add_text(key, pos, font);
      }
   }
}

void mws_vkb_impl::on_resize(uint32 i_width, uint32 i_height)
{
   std::vector<mws_sp<pfm_file>> file_vect;
   const umf_list file_list = pfm::filesystem::get_res_file_list();

   for (auto file_it : *file_list)
   {
      mws_sp<pfm_file> file = file_it.second;
      const std::string& name = file->get_file_name();

      if (mws_str::starts_with(name, VKB_PREFIX) && mws_str::ends_with(name, VKB_EXT))
      {
         file_vect.push_back(file);
      }
   }

   bool is_landscape = (i_width > i_height);
   int x = 3;
}

std::string mws_vkb_impl::get_key_name(key_types i_key_id) const
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

key_types mws_vkb_impl::get_key_type(const std::string& i_key_name) const
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

void mws_vkb_impl::load_map(mws_sp<mws_mod> i_mod, std::string i_filename)
{
   mws_sp<std::vector<uint8> > res = i_mod->storage.load_mod_byte_vect(i_filename);
   mws_sp<std::string> src(new std::string((const char*)begin_ptr(res), res->size()));
   mws_sp<kxmd_elem> kxmdi = kxmd::parse(src);
   uint32 point_count = 0;

   // pages
   {
      auto pages = kxmd::elem_at(kxmdi, "pages");
      auto pg_0 = kxmd::elem_at(pages, "0");
      // key_mods
      {
         auto key_mods = kxmd::elem_at(pg_0, "key-mods");
         // mod-none & mod-shift
         {
            auto keys_mod_none = kxmd::elem_at(key_mods, "mod-none");
            auto keys_mod_shift = kxmd::elem_at(key_mods, "mod-shift");
            uint32 size = keys_mod_none->vect.size();
            mws_assert(keys_mod_shift->vect.size() == size);
            set_key_vect_size(size);
            point_count = size;
            {
               for (uint32 k = 0; k < size; k++)
               {
                  // mod-none
                  {
                     const std::string& val = keys_mod_none->vect[k]->val;
                     uint32 key_id = mws_to<uint32>(val);

                     mws_assert(key_id > KEY_INVALID && key_id < KEY_COUNT);
                     key_mod_none_vect[k] = key_types(key_id);
                  }
                  // mod-shift
                  {
                     const std::string& val = keys_mod_shift->vect[k]->val;
                     uint32 key_id = mws_to<uint32>(val);

                     mws_assert(key_id > KEY_INVALID && key_id < KEY_COUNT);
                     key_mod_shift_vect[k] = key_types(key_id);
                  }
               }
            }
         }
      }
      // key coord
      {
         auto key_coord = kxmd::elem_at(pg_0, "key-coord");
         std::vector<glm::vec2> key_coord_pos;
         uint32 size = key_coord->vect.size();
         mws_assert(key_coord->vect.size() == (2 * point_count));

         for (uint32 k = 0; k < point_count; k++)
         {
            float pos_0 = mws_to<float>(key_coord->vect[2 * k + 0]->val);
            float pos_1 = mws_to<float>(key_coord->vect[2 * k + 1]->val);
            key_coord_pos.push_back(glm::vec2(pos_0, pos_1));
         }

         vk->set_kernel_points(key_coord_pos);
         //vk->update_geometry();
      }
   }

   trx("finished loading keyboard from [ {} ]", i_filename);
}

std::vector<key_types>& mws_vkb_impl::get_key_vect()
{
   switch (key_mod)
   {
   case key_mod_types::mod_none: return key_mod_none_vect;
   case key_mod_types::mod_shift: return key_mod_shift_vect;
   }

   mws_assert(false);
   return key_mod_none_vect;
}

uint32 mws_vkb_impl::get_key_vect_size()
{
   return get_key_vect().size();
}

void mws_vkb_impl::set_key_vect_size(uint32 i_size)
{
   key_mod_none_vect.resize(i_size);
   key_mod_shift_vect.resize(i_size);
}

key_types mws_vkb_impl::get_key_at(int i_idx)
{
   return get_key_vect()[i_idx];
}

void mws_vkb_impl::set_key_at(int i_idx, key_types i_key_id)
{
   get_key_vect()[i_idx] = i_key_id;
}

void mws_vkb_impl::erase_key_at(int i_idx)
{
   key_mod_none_vect.erase(key_mod_none_vect.begin() + i_idx);
   key_mod_shift_vect.erase(key_mod_shift_vect.begin() + i_idx);
}

void mws_vkb_impl::push_back_key(key_types i_key_id)
{
   key_mod_none_vect.push_back(i_key_id);
   key_mod_shift_vect.push_back(i_key_id);
}

void mws_vkb_impl::next_page()
{
   mws_println("next_page");
}

void mws_vkb_impl::prev_page()
{
   mws_println("prev_page");
}


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
         auto ret = impl->vk->get_kernel_idx_at(pt.x, pt.y);
         key_types key_id = impl->get_key_at(ret.idx);

         if (key_id != VKB_DONE)
         {
            mws_mod_ctrl::inst()->key_action(KEY_PRESS, key_id);
         }
         break;
      }

      case mws_ptr_evt::touch_ended:
      {
         auto& pt = pe->points[0];
         auto ret = impl->vk->get_kernel_idx_at(pt.x, pt.y);

         impl->current_key_idx = impl->selected_kernel_idx = ret.idx;
         {
            auto vd = impl->vk->get_diag_data();
            int id = vd->geom.kernel_points[impl->selected_kernel_idx].id;
            key_types key_id = impl->get_key_at(impl->selected_kernel_idx);
            std::string key_name = impl->get_key_name(key_id);
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
   impl->update_state();
}

void mws_vkb::update_view(mws_sp<mws_camera> g)
{
}

void mws_vkb::on_resize()
{
   uint32 w = pfm::screen::get_width();
   uint32 h = pfm::screen::get_height();

   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = (float)w;
   mws_r.h = (float)h;

   if (impl)
   {
      impl->on_resize(w, h);
   }
}

void mws_vkb::set_target(mws_sp<mws_text_area> i_ta)
{
   ta = i_ta;

   if (vkb_filename.empty())
   {
      load(mws_vkb_impl::get_map_filename(0));
   }
}

void mws_vkb::load(std::string i_filename)
{
   if (!impl)
   {
      uint32 w = pfm::screen::get_width();
      uint32 h = pfm::screen::get_height();

      impl = std::make_shared<mws_vkb_impl>(mws_vrn_obj_types::nexus_pairs | mws_vrn_obj_types::cells);
      //vk->vgeom->position = glm::vec3(0.f, 0.f, 1.f);
      attach(impl);
      impl->setup();
      impl->on_resize(w, h);
   }

   vkb_filename = i_filename;
   impl->load_map(get_mod(), vkb_filename);
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
