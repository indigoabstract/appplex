#include "stdafx.hxx"

#include "mws-vkb.hxx"
#include "vrn/vrn-diag.hxx"
#include "vrn/vrn-visual.hxx"
#include "mws-mod-ctrl.hxx"
#include "mws/mws-camera.hxx"
#include "mws/text-vxo.hxx"
#include "mws/mws-font.hxx"
#include "mws-mod.hxx"
#include "kxmd/kxmd.hxx"
#include <numeric>


std::vector<vkb_file_info> mws_vkb_file_store_impl::get_vkb_list()
{
   if (vkb_info_vect.empty())
   {
      // search all files in resources
      auto& file_list = *pfm::filesystem::get_res_file_list();

      // store all the found vkb files in a list
      for (auto file_it : file_list)
      {
         mws_sp<pfm_file> file = file_it.second;
         const std::string& name = file->get_file_name();
         vkb_info vkb_i = mws_vkb_impl::get_vkb_info(name);

         if (vkb_i.aspect_ratio != 0.f)
         {
            vkb_file_info vkb_i_t = { vkb_i, file };

            vkb_info_vect.push_back(vkb_i_t);
         }
      }
   }

   return vkb_info_vect;
}

bool mws_vkb_file_store_impl::file_exists(const std::string& i_vkb_filename)
{
   std::vector<vkb_file_info> list = get_vkb_list();

   for (auto& e : list)
   {
      if (e.file->get_file_name() == i_vkb_filename)
      {
         return true;
      }
   }

   return false;
}

void mws_vkb_file_store_impl::save_vkb(const std::string& i_vkb_filename, const std::string& i_data)
{
   std::vector<vkb_file_info> flist = get_vkb_list();
   std::string dir = flist[0].file->get_root_directory();
   mws_sp<pfm_file> map_file_save = pfm_file::get_inst(i_vkb_filename, dir);

   map_file_save->io.open("wt");
   map_file_save->io.write((const uint8*)i_data.c_str(), i_data.length());
   map_file_save->io.close();
}

mws_sp<std::string> mws_vkb_file_store_impl::load_vkb(const std::string& i_vkb_filename)
{
   if (file_exists(i_vkb_filename))
   {
      mws_sp<std::string> str = pfm::filesystem::load_res_as_string(i_vkb_filename);
      return str;
   }

   return nullptr;
}


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
   uint32 gcd = mws_util::math::gcd(w, h);
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
      {VKB_ALT, "alt"},
      {VKB_SHIFT, "shift"},
      {VKB_ESCAPE, "escape"},
      {VKB_DONE, "done"},
      {VKB_HIDE_KB, "hide"},
      {VKB_DELETE, "delete"},
      {KEY_SPACE, ""/*"space"*/},
   };
   float dpcm = pfm_main::gi()->get_screen_dpcm();
   set_font(mws_font::nwi(72.f));
   {
      vk = mws_vrn_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
      vk->toggle_voronoi_object(obj_type_mask);
      vk->init();
      vk->vgeom->position = glm::vec3(0.f, 0.f, -1.f);
      attach(vk->vgeom);
      build_cell_border_tex();
   }
   // finish setup
   {
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;

      vk_keys = text_vxo::nwi();
      attach(vk_keys);
      vk_keys->camera_id_list.clear();
      vk_keys->camera_id_list.push_back(mws_cam.lock()->camera_id);
      held_keys_st.resize(2);
   }
}

mws_sp<mws_ptr_evt> mws_vkb_impl::on_receive(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   mws_sp<mws_ptr_evt> forwarded_ptr;
   mws_sp<mws_ptr_evt> used_ptr = i_pe;
   base_key_state_types hide_vkb_state = base_key_state_types::key_free;
   int hide_vkb_idx = -1;
   bool vkb_hidden = is_mod_key_held(mod_key_types::hide_vkb, &hide_vkb_state, &hide_vkb_idx);

   if (vkb_hidden)
   {
      mws_sp<mws_ptr_evt> pe = mws_ptr_evt::nwi();
      std::vector<mws_ptr_evt::touch_point> forwarded_touch_vect;

      *pe = *i_pe;
      pe->touch_count = 0;

      for (uint32 k = 0; k < i_pe->touch_count; k++)
      {
         auto& pt = i_pe->points[k];
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

         if (ret.idx == hide_vkb_idx)
         {
            pe->points[pe->touch_count] = pt;
            pe->touch_count++;
         }
         else
         {
            forwarded_touch_vect.push_back(pt);
         }
      }

      if (!forwarded_touch_vect.empty())
      {
         // modify i_pe
         i_pe->touch_count = forwarded_touch_vect.size();

         for (uint32 k = 0; k < i_pe->touch_count; k++)
         {
            i_pe->points[k] = forwarded_touch_vect[k];
         }

         forwarded_ptr = i_pe;
         used_ptr = pe;
      }
   }

   if (used_ptr->touch_count > 0)
   {
      bool dbl_tap_detected = dbl_tap_det.detect_helper(used_ptr);

      switch (used_ptr->type)
      {
      case mws_ptr_evt::touch_began:
         touch_began(used_ptr, i_ta);
         break;

      case mws_ptr_evt::touch_moved:
         touch_moved(used_ptr, i_ta);
         break;

      case mws_ptr_evt::touch_ended:
         touch_ended(used_ptr, i_ta);
         break;

      case mws_ptr_evt::touch_cancelled:
         touch_cancelled(used_ptr, i_ta);
         break;
      }

      if (dbl_tap_detected)
      {
         glm::vec2 press_0 = dbl_tap_det.get_first_press_pos();
         glm::vec2 press_1 = dbl_tap_det.get_second_press_pos();
         auto idx_0 = vk->get_kernel_idx_at(press_0.x, press_0.y);
         auto idx_1 = vk->get_kernel_idx_at(press_1.x, press_1.y);

         if (idx_0.idx == idx_1.idx)
         {
            key_types active_key = get_key_at(idx_0.idx);

            if (is_mod_key(active_key))
            {
               const base_key_state& st = base_key_st[idx_0.idx];
               base_key_state_types new_state = (st.state == base_key_state_types::key_free) ? base_key_state_types::key_locked : base_key_state_types::key_free;

               set_key_state(idx_0.idx, new_state);
            }
         }
      }
   }

   if (!forwarded_ptr)
   {
      process(i_pe);
   }

   return forwarded_ptr;
}

void mws_vkb_impl::on_update_state()
{
   if (!keys_visible)
   {
      return;
   }

   uint32 crt_time = pfm::time::get_time_millis();
   auto root = get_mws_root();
   auto vd = vk->get_diag_data();
   auto& kp_vect = vd->geom.kernel_points;
   uint32 size = get_key_vect_size();

   // show keys
   vk_keys->clear_text();

   for (uint32 k = 0; k < size; k++)
   {
      mws_sp<mws_font> font = key_font;
      mws_sp<mws_font> font_base = key_font_base;

      if (k == current_key_idx)
      {
         font = selected_key_font;
      }

      key_types key_id = get_key_at(k);
      std::string key = get_key_name(key_id);
      auto& kp = kp_vect[k];
      glm::vec2 dim = font->get_text_dim(key);
      glm::vec2 pos(kp.position.x, kp.position.y);
      glm::vec2 dim_base = font_base->get_text_dim(key);
      glm::vec2 pos_base = pos - dim_base / 2.f;
      float offset = 3.f;

      pos -= dim / 2.f;
      vk_keys->add_text(key, pos_base + glm::vec2(-offset, -offset), font_base);
      vk_keys->add_text(key, pos_base + glm::vec2(offset, -offset), font_base);
      vk_keys->add_text(key, pos_base + glm::vec2(offset, offset), font_base);
      vk_keys->add_text(key, pos_base + glm::vec2(-offset, offset), font_base);
      vk_keys->add_text(key, pos, font);
   }

   // key lights
   {
      for (auto it = highlight_vect.begin(); it != highlight_vect.end();)
      {
         mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->cell_borders_mesh_vect[it->key_idx];
         float delta_seconds = (crt_time - it->release_time) / 1000.f;

         if (delta_seconds < key_lights_off_seconds)
         {
            float alpha = 1.f - delta_seconds / key_lights_off_seconds;
            (*mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, alpha);
            //trx("idx {} alpha {}", it->key_idx, alpha);
            ++it;
         }
         else
         {
            (*mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, 1.f);
            mesh->visible = false;
            it = highlight_vect.erase(it);
         }
      }
   }
}

void mws_vkb_impl::on_resize(uint32 i_width, uint32 i_height)
{
   vk->resize(i_width, i_height);
   vkb_file_info vkb_fi = get_closest_match(i_width, i_height);
   mws_sp<pfm_file> file = vkb_fi.file;

   load_map(file->get_file_name());
}

vkb_file_info mws_vkb_impl::get_closest_match(uint32 i_width, uint32 i_height)
{
   mws_assert(file_store != nullptr);
   float screen_aspect_ratio = float(i_width) / i_height;
   std::vector<vkb_file_info> vkb_info_vect = file_store->get_vkb_list();


   if (vkb_info_vect.empty())
   {
      mws_throw mws_exception("no vkb files available");
      return vkb_file_info();
   }

   // sort the vkb files by aspect ratio
   auto cmp_aspect_ratio = [](const vkb_file_info& i_a, const vkb_file_info& i_b) { return i_a.info.aspect_ratio > i_b.info.aspect_ratio; };
   std::sort(vkb_info_vect.begin(), vkb_info_vect.end(), cmp_aspect_ratio);

   // find the vkb file with the closest match to the screen's aspect ratio
   auto closest_val = [&cmp_aspect_ratio](const std::vector<vkb_file_info>& i_vect, float i_value) -> const vkb_file_info &
   {
      vkb_file_info vkb_i_t;
      vkb_i_t.info.aspect_ratio = i_value;
      auto const it = std::lower_bound(i_vect.begin(), i_vect.end(), vkb_i_t, cmp_aspect_ratio);

      if (it == i_vect.end())
      {
         return i_vect.back();
      }

      return *it;
   };

   // found it
   const vkb_file_info& vkb_i_t = closest_val(vkb_info_vect, screen_aspect_ratio);

   return vkb_i_t;
}

mws_sp<mws_font> mws_vkb_impl::get_font() const
{
   return key_font;
}

void mws_vkb_impl::set_font(mws_sp<mws_font> i_fnt)
{
   key_font = mws_font::nwi(i_fnt);
   key_font->set_color(gfx_color::colors::yellow);
   key_font_base = mws_font::nwi(i_fnt, i_fnt->get_size());
   key_font_base->set_color(gfx_color::colors::black);
   selected_key_font = mws_font::nwi(i_fnt);
   selected_key_font->set_color(gfx_color::colors::red);
}

void mws_vkb_impl::done()
{
   auto it = highlight_vect.begin();

   while (it != highlight_vect.end())
   {
      mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->cell_borders_mesh_vect[it->key_idx];
      (*mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, 1.f);
      mesh->visible = false;
      ++it;
   }

   highlight_vect.clear();
   release_all_keys();
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
   if (!file_store->file_exists(i_filename))
   {
      trx("file [ {} ] doesn't exist", i_filename);
      return;
   }

   mws_sp<std::string> data = file_store->load_vkb(i_filename);
   auto db = kxmd::nwi(data->c_str(), data->length());
   kv_ref main = db->main();
   uint32 point_count = 0;
   glm::vec2 diag_dim(0.f);
   glm::vec2 screen_dim(pfm::screen::get_width(), pfm::screen::get_height());
   glm::vec2 resize_fact(0.f);

   // size
   {
      kv_ref size = main["size"];
      diag_dim = glm::vec2(mws_to<float>(size[0].key()), mws_to<float>(size[1].key()));
      resize_fact = screen_dim / diag_dim;
   }
   // pages
   {
      auto invalid_key_check = [](uint32 i_key_id, const std::string& i_filename)
      {
         if (i_key_id == KEY_INVALID) { trx("warning[ invalid key in loaded vkb map [ {} ] ]", i_filename); }
         mws_assert(i_key_id < KEY_COUNT);
      };
      kv_ref pages = main["pages"];
      kv_ref pg_0 = pages["0"];
      // key-mods
      {
         kv_ref key_mods = pg_0["key-mods"];
         // list of key-mods
         {
            kv_ref keys_mod_none = key_mods["mod-none"];
            kv_ref keys_mod_alt = key_mods["mod-alt"];
            kv_ref keys_mod_shift = key_mods["mod-shift"];
            uint32 size = keys_mod_none.size();

            if (keys_mod_alt)
            {
               mws_assert(keys_mod_alt.size() == size);
            }

            if (keys_mod_shift)
            {
               mws_assert(keys_mod_shift.size() == size);
            }

            set_key_vect_size(size);
            point_count = size;

            // load key mods
            {
               // mod-none must always be present
               for (uint32 k = 0; k < size; k++)
               {
                  const std::string& key = keys_mod_none[k].key();
                  uint32 key_id = mws_to<uint32>(key);

                  invalid_key_check(key_id, i_filename);
                  key_mod_vect[(uint32)key_mod_types::mod_none][k] = key_types(key_id);
               }

               // mod-alt can be omitted
               if (keys_mod_alt)
               {
                  for (uint32 k = 0; k < size; k++)
                  {
                     const std::string& key = keys_mod_alt[k].key();
                     uint32 key_id = mws_to<uint32>(key);

                     invalid_key_check(key_id, i_filename);
                     key_mod_vect[(uint32)key_mod_types::mod_alt][k] = key_types(key_id);
                  }
               }

               // mod-shift can be omitted
               if (keys_mod_shift)
               {
                  for (uint32 k = 0; k < size; k++)
                  {
                     const std::string& key = keys_mod_shift[k].key();
                     uint32 key_id = mws_to<uint32>(key);

                     invalid_key_check(key_id, i_filename);
                     key_mod_vect[(uint32)key_mod_types::mod_shift][k] = key_types(key_id);
                  }
               }
            }
         }
      }
      // key coord
      {
         kv_ref key_coord = pg_0["key-coord"];
         std::vector<glm::vec2> key_coord_pos;
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

   rebuild_key_state();
   loaded_filename = i_filename;
   trx("finished loading keyboard from [ {} ]", loaded_filename);
}

std::vector<key_types>& mws_vkb_impl::get_key_vect()
{
   mws_assert(key_mod < key_mod_types::count);
   return key_mod_vect[(uint32)key_mod];
}

uint32 mws_vkb_impl::get_key_vect_size()
{
   return key_mod_vect[(uint32)key_mod_types::mod_none].size();
}

key_types mws_vkb_impl::get_key_at(int i_idx)
{
   return get_key_vect()[i_idx];
}

key_types mws_vkb_impl::get_mod_key_at(key_mod_types i_key_mod, int i_idx)
{
   mws_assert(i_key_mod < key_mod_types::count);
   return key_mod_vect[(uint32)i_key_mod][i_idx];
}

void mws_vkb_impl::next_page()
{
   mws_println("next_page");
}

void mws_vkb_impl::prev_page()
{
   mws_println("prev_page");
}

void mws_vkb_impl::set_on_top()
{
   vk->vgeom->position = glm::vec3(0.f, 0.f, 1.f);
}

void mws_vkb_impl::build_cell_border_tex()
{
   std::string tex_id = "cell-border-tex";
   cell_border_tex = gi()->tex.get_texture_by_name(tex_id);

   if (!cell_border_tex)
   {
      gfx_color_mixer pal;
      gfx_tex_params prm;

      pal.set_color_at(gfx_color(0, 0, 0), 0.f);
      pal.set_color_at(gfx_color(85, 0, 0), 0.5f);
      pal.set_color_at(gfx_color(225, 0, 0), 0.87f);
      pal.set_color_at(gfx_color(255, 100, 0), 0.92f);
      pal.set_color_at(gfx_color(255, 240, 120), 0.97f);
      pal.set_color_at(gfx_color(255, 255, 150), 1.f);

      prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_repeat;
      prm.max_anisotropy = 0.f;
      prm.gen_mipmaps = true;
      prm.min_filter = gfx_tex_params::e_tf_linear_mipmap_linear;
      prm.mag_filter = gfx_tex_params::e_tf_linear;
      //prm.gen_mipmaps = false;
      //prm.min_filter = prm.mag_filter = gfx_tex_params::e_tf_nearest;

      cell_border_tex = gi()->tex.nwi(tex_id, 512, 1, &prm);

      uint32 width = cell_border_tex->get_width();
      uint32 height = cell_border_tex->get_height();
      std::vector<gfx_color> pixel_data(width * height);
      uint32 hw = width / 2;

      for (uint32 k = 0; k < height; k++)
      {
         for (uint32 i = 0; i < hw; i++)
         {
            float alpha = float(i) / (hw - 1);
            gfx_color color = pal.get_color_at(alpha);
            uint32 il = k * width + i;
            uint32 ir = k * width + width - 1 - i;

            pixel_data[il] = color;
            pixel_data[ir] = color;
         }
      }

      cell_border_tex->update(0, (char*)pixel_data.data());
   }

   vk->vgeom->cell_borders->tex = cell_border_tex;
}

bool mws_vkb_impl::is_mod_key(key_types i_key_id)
{
   switch (i_key_id)
   {
   case VKB_ALT:
   case VKB_SHIFT:
   case VKB_HIDE_KB:
      return true;
   }

   return false;
}

bool mws_vkb_impl::is_mod_key_held(mod_key_types i_mod_key, base_key_state_types* i_state, int* i_key_idx) const
{
   for (auto it = mod_keys_st.begin(); it != mod_keys_st.end(); ++it)
   {
      const base_key_state& st = base_key_st[it->first];

      if (st.key_id == static_cast<key_types>(i_mod_key))
      {
         if (st.state == base_key_state_types::key_held || st.state == base_key_state_types::key_locked)
         {
            if (i_state != nullptr)
            {
               *i_state = st.state;
            }

            if (i_key_idx != nullptr)
            {
               *i_key_idx = it->first;
            }

            return true;
         }
      }
   }

   return false;
}

void mws_vkb_impl::set_mod_key_lock(mod_key_types i_mod_key, bool i_set_lock)
{
   int mod_key_idx = -1;

   for (auto it = mod_keys_st.begin(); it != mod_keys_st.end(); ++it)
   {
      const base_key_state& st = base_key_st[it->first];

      if (st.key_id == static_cast<key_types>(i_mod_key))
      {
         if (mod_key_idx == -1)
         {
            mod_key_idx = it->first;
         }

         set_key_state(it->first, base_key_state_types::key_free);
      }
   }

   if (mod_key_idx != -1)
   {
      base_key_state_types state = (i_set_lock) ? base_key_state_types::key_locked : base_key_state_types::key_free;
      set_key_state(mod_key_idx, base_key_state_types::key_free);
   }
}

void mws_vkb_impl::clear_mod_key_locks()
{
   for (auto it = mod_keys_st.begin(); it != mod_keys_st.end(); ++it)
   {
      set_key_state(it->first, base_key_state_types::key_free);
   }
}

void mws_vkb_impl::set_key_vect_size(uint32 i_size)
{
   for (uint32 k = 0; k < (uint32)key_mod_types::count; k++)
   {
      key_mod_vect[k].resize(i_size);
   }
}

void mws_vkb_impl::set_key_at(int i_idx, key_types i_key_id)
{
   get_key_vect()[i_idx] = i_key_id;

   if (is_mod_key(i_key_id))
   {
      rebuild_key_state();
   }
}

void mws_vkb_impl::erase_key_at(int i_idx)
{
   for (uint32 k = 0; k < (uint32)key_mod_types::count; k++)
   {
      key_mod_vect[k].erase(key_mod_vect[k].begin() + i_idx);
   }

   rebuild_key_state();
}

void mws_vkb_impl::push_back_key(key_types i_key_id)
{
   for (uint32 k = 0; k < (uint32)key_mod_types::count; k++)
   {
      key_mod_vect[k].push_back(i_key_id);
   }

   rebuild_key_state();
}

bool mws_vkb_impl::touch_began(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   for (uint32 k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];
      auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
      key_types key_id = get_key_at(ret.idx);
      //trx("pressed i_idx {}", (char)key_id);

      held_keys_st[crt_keys_st_idx][ret.idx] = pt.is_changed;
   }

   handle_ptr_evt(i_pe);

   return true;
}

bool mws_vkb_impl::touch_moved(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   if (held_keys_st[prev_keys_st_idx].empty())
   {
      return false;
   }

   for (uint32 k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];
      auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
      key_types key_id = get_key_at(ret.idx);
      //trx("highlight i_idx {}", ret.idx);

      held_keys_st[crt_keys_st_idx][ret.idx] = pt.is_changed;
   }

   handle_ptr_evt(i_pe);

   return true;
}

bool mws_vkb_impl::touch_ended(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   // discard the event that comes immediately after opening the keyboard
   if (held_keys_st[prev_keys_st_idx].empty())
   {
      return false;
   }

   for (uint32 k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];

      if (pt.is_changed)
      {
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

         held_keys_st[crt_keys_st_idx][ret.idx] = pt.is_changed;
      }
   }

   handle_ptr_evt(i_pe);

   return true;
}

bool mws_vkb_impl::touch_cancelled(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   handle_ptr_evt(i_pe);

   if (i_pe->touch_count == 1)
   {
      get_mod()->key_ctrl_inst->clear_keys();
   }

   return false;
}

void mws_vkb_impl::highlight_key_at(int i_idx, bool i_light_on)
{
   mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->cell_borders_mesh_vect[i_idx];
   mesh->visible = i_light_on;

   for (auto it = highlight_vect.begin(); it != highlight_vect.end(); ++it)
   {
      if (it->key_idx == i_idx)
      {
         (*mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, 1.f);
         highlight_vect.erase(it);
         break;
      }
   }
}

void mws_vkb_impl::fade_key_at(int i_idx)
{
   for (auto& kh : highlight_vect)
   {
      if (kh.key_idx == i_idx)
      {
         kh.release_time = pfm::time::get_time_millis();
         mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->cell_borders_mesh_vect[i_idx];
         return;
      }
   }

   highlight_vect.push_back(key_highlight{ i_idx, pfm::time::get_time_millis() });
}

void mws_vkb_impl::handle_ptr_evt(mws_sp<mws_ptr_evt> i_pe)
{
   auto& kc = get_mod()->key_ctrl_inst;
   auto& crt_st = held_keys_st[crt_keys_st_idx];
   auto& prev_st = held_keys_st[prev_keys_st_idx];

   switch (i_pe->type)
   {
   case mws_ptr_evt::touch_began:
   case mws_ptr_evt::touch_moved:
   {
      // this will become true when keyboard becomes hidden
      bool keys_iter_invalidated = false;

      // for every key that's present in the current ht, if it's changed, call key_pressed() and highlight it
      for (auto it = crt_st.begin(); it != crt_st.end(); ++it)
      {
         bool& is_changed = it->second;

         if (is_changed)
         {
            int pos_idx = it->first;

            kc->key_pressed(key_types(pos_idx));
            trx("kp {}", pos_idx);
            keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_held);

            if (keys_iter_invalidated)
            {
               break;
            }
         }
      }

      if (!keys_iter_invalidated)
      {
         // for every key that's not present in the current ht, call key_released() and fade it out
         for (auto it = prev_st.begin(); it != prev_st.end(); ++it)
         {
            if (crt_st.find(it->first) == crt_st.end())
            {
               int pos_idx = it->first;

               kc->key_released(key_types(pos_idx));
               trx("kr0 {}", pos_idx);
               keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_free);

               if (keys_iter_invalidated)
               {
                  break;
               }
            }
         }
      }
      break;
   }

   case mws_ptr_evt::touch_ended:
   {
      // this will become true when keyboard becomes hidden
      bool keys_iter_invalidated = false;

      // for every key that's present in the current ht, call key_released() and fade it out
      for (auto it = crt_st.begin(); it != crt_st.end(); ++it)
      {
         int pos_idx = it->first;

         kc->key_released(key_types(pos_idx));
         trx("kr1 {}", pos_idx);
         keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_free);

         if (keys_iter_invalidated)
         {
            break;
         }
      }

      if (!keys_iter_invalidated && i_pe->touch_count == 1)
      {
         uint32 size = base_key_st.size();

         get_mod()->key_ctrl_inst->clear_keys();

         for (uint32 k = 0; k < size; ++k)
         {
            base_key_state& st = base_key_st[k];

            if (st.state == base_key_state_types::key_held)
            {
               set_key_state(k, base_key_state_types::key_free);
            }
         }
      }

      crt_st.clear();
      break;
   }

   case mws_ptr_evt::touch_cancelled:
      break;
   }

   // clear previous state and then mark the current state as the previous one
   prev_st.clear();
   std::swap(crt_keys_st_idx, prev_keys_st_idx);

   for (auto p : crt_st)
   {
      trx("key-held {}", p.first);
   }
}

void mws_vkb_impl::release_all_keys()
{
   uint32 size = base_key_st.size();

   get_mod()->key_ctrl_inst->clear_keys();
   held_keys_st[crt_keys_st_idx].clear();
   held_keys_st[prev_keys_st_idx].clear();

   for (uint32 k = 0; k < size; ++k)
   {
      highlight_key_at(k, false);
      base_key_st[k].state = base_key_state_types::key_free;
   }
}

bool mws_vkb_impl::set_key_state(int i_key_idx, base_key_state_types i_state)
{
   base_key_state& st = base_key_st[i_key_idx];
   // save the initial state
   base_key_state_types init_state = st.state;

   if (init_state == i_state)
   {
      return false;
   }

   // update state now, so we can query is_mod_key_held()
   st.state = i_state;

   key_types key_id = get_mod_key_at(key_mod_types::mod_none, i_key_idx);
   auto update_lights = [this](int i_key_idx, base_key_state_types i_state)
   {
      if (i_state == base_key_state_types::key_free)
      {
         fade_key_at(i_key_idx);
      }
      else
      {
         highlight_key_at(i_key_idx);
      }
   };

   if (is_mod_key(key_id))
   {
      switch (key_id)
      {
      case VKB_ALT:
      {
         if (is_mod_key_held(mod_key_types::alt))
         {
            if (is_mod_key_held(mod_key_types::shift))
            {
               key_mod = key_mod_types::mod_shift;
            }
            else
            {
               key_mod = key_mod_types::mod_alt;
            }
         }
         else
         {
            if (is_mod_key_held(mod_key_types::shift))
            {
               key_mod = key_mod_types::mod_shift;
            }
            else
            {
               key_mod = key_mod_types::mod_none;
            }
         }

         trx("alt key_mod {}", (int)key_mod);
         update_lights(i_key_idx, i_state);
         break;
      }

      case VKB_SHIFT:
      {
         if (is_mod_key_held(mod_key_types::shift))
         {
            key_mod = key_mod_types::mod_shift;
         }
         else
         {
            if (is_mod_key_held(mod_key_types::alt))
            {
               key_mod = key_mod_types::mod_alt;
            }
            else
            {
               key_mod = key_mod_types::mod_none;
            }
         }

         trx("shift key_mod {} state {}", (int)key_mod, (int)i_state);
         update_lights(i_key_idx, i_state);
         break;
      }

      case VKB_HIDE_KB:
      {
         uint32 size = base_key_st.size();
         bool show_vkb = (i_state == base_key_state_types::key_free);
         vk->vgeom->nexus_pairs_mesh->visible = show_vkb;
         vk_keys->visible = show_vkb;

         // if we need to hide the keyboard
         if (show_vkb)
         {
            fade_key_at(i_key_idx);
         }
         else
         {
            bool is_changed = held_keys_st[crt_keys_st_idx][i_key_idx];

            get_mod()->key_ctrl_inst->clear_keys();
            held_keys_st[crt_keys_st_idx].clear();
            held_keys_st[prev_keys_st_idx].clear();
            held_keys_st[crt_keys_st_idx][i_key_idx] = is_changed;
            highlight_key_at(i_key_idx);

            for (uint32 k = 0; k < size; ++k)
            {
               if (k != i_key_idx)
               {
                  highlight_key_at(k, false);
                  base_key_st[k].state = base_key_state_types::key_free;
               }
            }

            return true;
         }
         break;
      }
      }
   }
   else
   {
      update_lights(i_key_idx, i_state);
   }

   return false;
}

void mws_vkb_impl::rebuild_key_state()
{
   std::vector<key_types>& key_mod_default = key_mod_vect[(uint32)key_mod_types::mod_none];
   uint32 size = key_mod_default.size();

   mod_keys_st.clear();
   base_key_st.resize(size);

   // set the base key state and also find and store the mod keys in a hashtable
   for (uint32 k = 0; k < size; k++)
   {
      key_types key_id = key_mod_default[k];

      if (is_mod_key(key_id))
      {
         mod_keys_st[k] = get_mod_key_at(key_mod_types::mod_none, k);
      }

      base_key_st[k] = base_key_state{ key_id, base_key_state_types::key_free };
   }
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

key_types mws_vkb::apply_key_modifiers(key_types i_key_id) const
{
   uint32 idx = i_key_id;

   if (idx < impl->get_key_vect().size())
   {
      key_types key_id = impl->get_key_at(i_key_id);

      return key_id;
   }

   return KEY_INVALID;
}

void mws_vkb::receive(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<mws_ptr_evt> pe = mws_ptr_evt::as_pointer_evt(i_dp);
      mws_sp<mws_ptr_evt> forwarded_ptr = get_impl()->on_receive(pe, ta);

      // check if we need to close the keyboard
      if (!forwarded_ptr && pe->type == mws_ptr_evt::touch_ended)
      {
         for (uint32 k = 0; k < pe->touch_count; k++)
         {
            auto& pt = pe->points[k];

            if (pt.is_changed)
            {
               auto ret = get_impl()->vk->get_kernel_idx_at(pt.x, pt.y);
               key_types key_id = get_impl()->get_key_at(ret.idx);

               if (ta->is_action_key(key_id))
               {
                  done();
               }
            }
         }
      }
   }
   else if (i_dp->is_type(mws_key_evt::KEYEVT_EVT_TYPE))
   {
      ta->receive(i_dp);
   }
}

void mws_vkb::update_state()
{
   get_impl()->on_update_state();
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

mws_sp<mws_font> mws_vkb::get_font()
{
   return get_impl()->get_font();
}

void mws_vkb::set_font(mws_sp<mws_font> i_fnt)
{
   get_impl()->set_font(i_fnt);
}

mws_sp<mws_vkb_file_store> mws_vkb::get_file_store() const
{
   return file_store;
}

void mws_vkb::set_file_store(mws_sp<mws_vkb_file_store> i_store)
{
   file_store = i_store;
}

mws_sp<gfx_tex> mws_vkb::get_cell_border_tex()
{
   return get_impl()->cell_border_tex;
}


void mws_vkb::setup()
{
   mws_virtual_keyboard::setup();
   position = glm::vec3(position().x, position().y, 1.f);
   on_resize();
}

void mws_vkb::done()
{
   get_impl()->done();
   visible = false;
   ta->do_action();
   ta = nullptr;
}

mws_sp<mws_vkb_impl> mws_vkb::get_impl()
{
   if (!impl)
   {
      uint32 w = pfm::screen::get_width();
      uint32 h = pfm::screen::get_height();

      impl = std::make_shared<mws_vkb_impl>(mws_vrn_obj_types::nexus_pairs | mws_vrn_obj_types::cell_borders);// | mws_vrn_obj_types::cells);
      impl->file_store = get_file_store();
      attach(impl);
      impl->setup();
      impl->set_on_top();
      impl->on_resize(w, h);
   }

   return impl;
}
