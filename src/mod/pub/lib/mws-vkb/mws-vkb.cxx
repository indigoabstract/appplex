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

vkb_info mws_vkb_impl::get_vkb_info(const std::string& i_filename)
{
   vkb_info vkb_i;

   if (mws_str::starts_with(i_filename, VKB_PREFIX) && mws_str::ends_with(i_filename, VKB_EXT))
   {
      size_t nr_0_start_idx = VKB_PREFIX.length();
      size_t nr_0_end_idx = i_filename.find('x', nr_0_start_idx);
      std::string nr_0 = i_filename.substr(nr_0_start_idx, nr_0_end_idx - nr_0_start_idx);
      size_t nr_1_start_idx = nr_0_end_idx + 1;
      size_t nr_1_end_idx = i_filename.find('-', nr_1_start_idx);
      std::string nr_1 = i_filename.substr(nr_1_start_idx, nr_1_end_idx - nr_1_start_idx);
      size_t nr_2_start_idx = nr_1_end_idx + 1;
      size_t nr_2_end_idx = i_filename.find(VKB_EXT, nr_1_start_idx);
      std::string nr_2 = i_filename.substr(nr_2_start_idx, nr_2_end_idx - nr_2_start_idx);

      vkb_i.width = mws_to<uint32>(nr_0);
      vkb_i.height = mws_to<uint32>(nr_1);
      vkb_i.index = mws_to<uint32>(nr_2);
      vkb_i.aspect_ratio = float(vkb_i.width) / vkb_i.height;
   }

   return vkb_i;
}

std::string mws_vkb_impl::get_vkb_filename(uint32 i_map_idx)
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
   float dpcm = pfm_main::gi()->get_screen_dpcm();
   {
      vk = mws_vkb_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
      vk->toggle_voronoi_object(obj_type_mask);
      vk->init();
      vk->vgeom->position = glm::vec3(0.f, 0.f, -1.f);
      attach(vk->vgeom);
      key_font = mws_font::nwi(72.f);
      key_font->set_color(gfx_color::colors::white);
      selected_key_font = mws_font::nwi(48.f);
      selected_key_font->set_color(gfx_color::colors::red);
   }
   // finish setup
   {
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;

      key_font = mws_font::nwi(48.f);
      key_font->set_color(gfx_color::colors::white);
      selected_key_font = mws_font::nwi(72.f);
      selected_key_font->set_color(gfx_color::colors::red);
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
   vk->resize(i_width, i_height);

   struct vkb_info_t
   {
      vkb_info info;
      uint32 file_vect_idx = 0;
   };

   // search all files in resources
   std::vector<mws_sp<pfm_file>> file_vect;
   const umf_list file_list = pfm::filesystem::get_res_file_list();
   float screen_aspect_ratio = float(i_width) / i_height;
   std::vector<vkb_info_t> vkb_info_vect;
   uint32 k = 0;

   // store all the found vkb files in a list
   for (auto file_it : *file_list)
   {
      mws_sp<pfm_file> file = file_it.second;
      const std::string& name = file->get_file_name();
      vkb_info vkb_i = mws_vkb_impl::get_vkb_info(name);

      if (vkb_i.aspect_ratio != 0.f)
      {
         vkb_info_t vkb_i_t = { vkb_i, k };

         file_vect.push_back(file);
         vkb_info_vect.push_back(vkb_i_t);
         k++;
      }
   }

   if (vkb_info_vect.empty())
   {
      mws_throw mws_exception("no vkb files available");
      return;
   }

   // sort the vkb files by aspect ratio
   auto cmp_aspect_ratio = [](const vkb_info_t & i_a, const vkb_info_t & i_b) { return i_a.info.aspect_ratio > i_b.info.aspect_ratio; };
   std::sort(vkb_info_vect.begin(), vkb_info_vect.end(), cmp_aspect_ratio);

   // find the vkb file with the closest match to the screen's aspect ratio
   auto closest_val = [&cmp_aspect_ratio](const std::vector<vkb_info_t> & i_vect, float i_value) -> const vkb_info_t &
   {
      vkb_info_t vkb_i_t;
      vkb_i_t.info.aspect_ratio = i_value;
      auto const it = std::lower_bound(i_vect.begin(), i_vect.end(), vkb_i_t, cmp_aspect_ratio);

      if (it == i_vect.end())
      {
         return i_vect.back();
      }

      return *it;
   };
   // found it
   const vkb_info_t& vkb_i_t = closest_val(vkb_info_vect, screen_aspect_ratio);
   mws_sp<pfm_file> file = file_vect[vkb_i_t.file_vect_idx];

   load_map(file->get_file_name());
}

void mws_vkb_impl::set_font(mws_sp<mws_font> i_fnt)
{
   key_font = mws_font::nwi(i_fnt);
   key_font->set_color(gfx_color::colors::white);
   selected_key_font = mws_font::nwi(i_fnt);
   selected_key_font->set_color(gfx_color::colors::red);
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

void mws_vkb_impl::load_map(std::string i_filename)
{
   auto db = kxmd::nwi_from_file(i_filename);
   kxmd_ref main = db->main();
   uint32 point_count = 0;
   glm::vec2 diag_dim(0.f);
   glm::vec2 screen_dim(pfm::screen::get_width(), pfm::screen::get_height());
   glm::vec2 resize_fact(0.f);

   // size
   {
      kxmd_ref size = main["size"];
      diag_dim = glm::vec2(mws_to<float>(size[0].key()), mws_to<float>(size[1].key()));
      resize_fact = screen_dim / diag_dim;
   }
   // pages
   {
      kxmd_ref pages = main["pages"];
      kxmd_ref pg_0 = pages["0"];
      // key_mods
      {
         kxmd_ref key_mods = pg_0["key-mods"];
         // mod-none & mod-shift
         {
            kxmd_ref keys_mod_none = key_mods["mod-none"];
            kxmd_ref keys_mod_shift = key_mods["mod-shift"];
            uint32 size = keys_mod_none.size();
            mws_assert(keys_mod_shift.size() == size);
            set_key_vect_size(size);
            point_count = size;
            {
               for (uint32 k = 0; k < size; k++)
               {
                  // mod-none
                  {
                     const std::string& key = keys_mod_none[k].key();
                     uint32 key_id = mws_to<uint32>(key);

                     mws_assert(key_id > KEY_INVALID && key_id < KEY_COUNT);
                     key_mod_none_vect[k] = key_types(key_id);
                  }
                  // mod-shift
                  {
                     const std::string& key = keys_mod_shift[k].key();
                     uint32 key_id = mws_to<uint32>(key);

                     mws_assert(key_id > KEY_INVALID && key_id < KEY_COUNT);
                     key_mod_shift_vect[k] = key_types(key_id);
                  }
               }
            }
         }
      }
      // key coord
      {
         kxmd_ref key_coord = pg_0["key-coord"];
         std::vector<glm::vec2> key_coord_pos;
         uint32 size = key_coord.size();
         mws_assert(key_coord.size() == (2 * point_count));

         for (uint32 k = 0; k < point_count; k++)
         {
            float pos_0 = mws_to<float>(key_coord[2 * k + 0].key());
            float pos_1 = mws_to<float>(key_coord[2 * k + 1].key());
            glm::vec2 dim = glm::vec2(pos_0, pos_1) * resize_fact;

            key_coord_pos.push_back(dim);
         }

         vk->set_kernel_points(key_coord_pos);
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

         if (!ta->is_action_key(key_id))
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

            if (ta->is_action_key(key_id))
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
   impl = get_impl();
}

void mws_vkb::set_font(mws_sp<mws_font> i_fnt)
{
   get_impl()->set_font(i_fnt);
}

void mws_vkb::setup()
{
   mws_virtual_keyboard::setup();
   position = glm::vec3(position().x, position().y, 1.f);
   on_resize();
}

void mws_vkb::done()
{
   ta->do_action();
   visible = false;
   ta = nullptr;
}

mws_sp<mws_vkb_impl> mws_vkb::get_impl()
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

   return impl;
}
