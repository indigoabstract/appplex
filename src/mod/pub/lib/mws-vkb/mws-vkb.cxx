#include "stdafx.hxx"

#include "mws-vkb.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "vrn/vrn-diag.hxx"
#include "vrn/vrn-visual.hxx"
#include "mws/mws-camera.hxx"
#include "fonts/mws-text-vxo.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"
#include "kawase-bloom/kawase-bloom.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "gfx-tex.hxx"
#include "gfx-pbo.hxx"
#include "tex-atlas/mws-tex-atlas.hxx"
#include "kxmd/kxmd.hxx"
#include <array>
#include <numeric>


using namespace ns_kxmd;


mws_vkb_file_store_impl::mws_vkb_file_store_impl(mws_sp<mws_mod> i_mod) : mod(i_mod) {}

std::vector<vkb_file_info> mws_vkb_file_store_impl::get_vkb_list()
{
   if (vkb_info_vect.empty())
   {
      // search all files in resources
      const mws_file_map& file_list = mod.lock()->storage.get_res_file_list();

      // store all the found vkb files in a list
      for (auto file_it : file_list)
      {
         mws_sp<mws_file> file = file_it.second;
         const std::string& name = file->filename();
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
      if (e.file->filename() == i_vkb_filename)
      {
         return true;
      }
   }

   return false;
}

void mws_vkb_file_store_impl::save_vkb(const std::string& i_vkb_filename, const std::string& i_data)
{
   std::vector<vkb_file_info> flist = get_vkb_list();
   mws_path dir = flist[0].file->path().parent_path();
   mws_sp<mws_file> map_file_save = mws_file::get_inst(dir / i_vkb_filename);

   map_file_save->io.open("wt");
   map_file_save->io.write(reinterpret_cast<const std::byte*>(i_data.c_str()), i_data.length());
   map_file_save->io.close();
}

std::string mws_vkb_file_store_impl::load_vkb(const std::string& i_vkb_filename)
{
   std::string vkb;

   if (file_exists(i_vkb_filename))
   {
      vkb = mod.lock()->storage.load_as_string(i_vkb_filename);
   }

   return vkb;
}

mws_vkb_impl::mws_vkb_impl(uint32_t i_obj_type_mask)
{
   obj_type_mask = i_obj_type_mask;

   if (resolution_params.empty())
   {
      resolution_params =
      {
         // sd
         { 720, { 0.9505f, 0.95925f, 0.9025f, 0.9025f, 1.f, 1.f, 1.f, 1.f, } },
         // custom
         { 1040, { 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
         // hd
         { 1280, { 0.505f, 0.5925f, 0.025f, 0.025f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
         // full hd
         { 1920, { 0.45f, 0.35f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.5f, 0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
         // 2094
         { 2094, { 0.55f, 0.25f, 0.125f, 0.125f, 0.25f, 0.25f, 0.25f, 0.5f, 0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f  } },
         // wqhd
         { 2560, { 0.45f, 0.35f, 0.25f, 0.25f, 0.25f, 0.25f, 0.25f, 0.5f, 0.5f, 0.5f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
         // note 9 fullres
         { 2960, { 0.25f, 0.125f, 0.125f, 0.25f, 0.25f, 0.25f, 0.25f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
         // 4k
         { 4096, { 0.25f, 0.125f, 0.125f, 0.25f, 0.25f, 0.25f, 0.25f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f, 1.f } },
      };
   }
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

      mws_to<uint32_t>(nr_0, vkb_i.width);
      mws_to<uint32_t>(nr_1, vkb_i.height);
      mws_to<uint32_t>(nr_2, vkb_i.index);
      vkb_i.aspect_ratio = float(vkb_i.width) / vkb_i.height;
   }

   return vkb_i;
}

std::string mws_vkb_impl::get_vkb_filename(uint32_t i_map_idx)
{
   uint32_t w = mws::screen::get_width();
   uint32_t h = mws::screen::get_height();
   uint32_t gcd = std::gcd(w, h);
   uint32_t wd = w / gcd;
   uint32_t hd = h / gcd;

   return trs("{0}{1}x{2}-{3}{4}", VKB_PREFIX, wd, hd, i_map_idx, VKB_EXT);
}

void mws_vkb_impl::setup()
{
   key_map =
   {
      {mws_key_backspace, "Backsp"},
      {VKB_ENTER, "Enter"},
      {VKB_ALT, "Alt"},
      {VKB_SHIFT, "Shift"},
      {VKB_ESCAPE, "Esc"},
      {VKB_DONE, "Done"},
      {VKB_HIDE_KB, "Hide"},
      {VKB_DELETE, "Del"},
      {mws_key_space, ""/*"space"*/},
   };
   {
      setup_font_dimensions();
      vk = mws_vrn_main::nwi(mws::screen::get_width(), mws::screen::get_height(), mws_cam.lock());
      vk->toggle_voronoi_object(obj_type_mask);
      vk->init();
      vk->vgeom->position = glm::vec3(0.f, 0.f, -1.f);
      attach(vk->vgeom);
   }
   // finish setup
   {
      // mark this as a double tap if the touches fall within the same key
      dbl_tap_det.dist_eval = [vk = vk](const glm::vec2& i_first_press, const glm::vec2& i_second_press)
      {
         auto idx_0 = vk->get_kernel_idx_at(i_first_press.x, i_first_press.y);
         auto idx_1 = vk->get_kernel_idx_at(i_second_press.x, i_second_press.y);

         return (idx_0.idx == idx_1.idx);
      };
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
      bool forward_touch = true;

      // check if the hide key is locked
      if (hide_vkb_state != base_key_state_types::key_locked)
      {
         const mws_ptr_evt::touch_point* vkb_hide_finger = i_pe->find_point(pressed_vkb_hide_finger_id);

         // it's not, so check if we can find the finger that was used to hide the keyboard
         if (vkb_hide_finger)
         {
            auto ret = vk->get_kernel_idx_at(vkb_hide_finger->x, vkb_hide_finger->y);

            // if the indices differ, it means the finger moved outside the hide button,
            // so release the button and show the keyboard
            if (ret.idx != hide_vkb_idx)
            {
               set_key_state(hide_vkb_idx, base_key_state_types::key_free);
               pressed_vkb_hide_finger_id = -1;
               forward_touch = false;
            }
         }
      }

      // if keyboard is hidden, forward the touch to the underlying text area
      if (forward_touch)
      {
         mws_sp<mws_ptr_evt> pe = mws_ptr_evt::nwi();
         std::vector<mws_ptr_evt::touch_point> forwarded_touch_vect;

         *pe = *i_pe;
         pe->touch_count = 0;

         for (uint32_t k = 0; k < i_pe->touch_count; k++)
         {
            auto& pt = i_pe->points[k];
            auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

            if (ret.idx == hide_vkb_idx)
            {
               if (pt.is_changed)
               {
                  pe->points[pe->touch_count] = pt;
                  pe->touch_count++;
               }
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

            for (uint32_t k = 0; k < i_pe->touch_count; k++)
            {
               i_pe->points[k] = forwarded_touch_vect[k];
            }

            forwarded_ptr = i_pe;
            used_ptr = pe;
         }
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
         auto idx_0 = vk->get_kernel_idx_at(press_0.x, press_0.y);
         mws_key_types active_key = get_key_at(idx_0.idx);

         if ((!vkb_hidden && is_mod_key(active_key)) || (vkb_hidden && (idx_0.idx == hide_vkb_idx)))
         {
            const base_key_state& st = base_key_st[idx_0.idx];
            base_key_state_types new_state = (st.state == base_key_state_types::key_free) ? base_key_state_types::key_locked : base_key_state_types::key_free;

            set_key_state(idx_0.idx, new_state);
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
   if (fade_slider.is_enabled())
   {
      fade_slider.update();
      float alpha = fade_slider.get_value();
      if (fade_type == fade_types::e_hide_vkb) { alpha = 1.f - alpha; }
      set_key_transparency(alpha);
      if (!fade_slider.is_enabled()) { fade_type = fade_types::e_none; }
   }

   // key lights
   if(build_textures)
   {
      uint32_t crt_time = mws::time::get_time_millis();

      for (auto it = highlight_vect.begin(); it != highlight_vect.end();)
      {
         mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->get_cell_borders_mesh_at(it->key_idx);
         float delta_seconds = (crt_time - it->release_time) / 1000.f;

         if (delta_seconds < key_lights_off_seconds)
         {
            float alpha = 1.f - delta_seconds / key_lights_off_seconds;
            (*mesh)["u_v1_transparency"] = alpha;
            ++it;
         }
         else
         {
            (*mesh)["u_v1_transparency"] = 1.f;
            mesh->visible = false;
            it = highlight_vect.erase(it);
         }
      }

      if (pressed_key->visible && pressed_key->is_fading(crt_time))
      {
         float delta_seconds = (crt_time - pressed_key->light_turnoff_start) / 1000.f - pressed_key_lights_hold_seconds;
         float alpha = 1.f - delta_seconds / pressed_key_lights_off_seconds;
         pressed_key->set_fade_gradient(alpha);

         if (alpha <= 0.f)
         {
            pressed_key->visible = false;
         }
      }
   }
}

void mws_vkb_impl::on_resize(uint32_t i_width, uint32_t i_height)
{
   glm::ivec2 scr_dim(i_width, i_height);

   if (vkb_dim != scr_dim)
   {
      vkb_file_info vkb_fi = get_closest_vkb_match(i_width, i_height);
      mws_sp<mws_file> file = vkb_fi.file;
      std::string new_filename = file->filename();
      vkb_dim = scr_dim;

      // check if 'loaded_filename' is the best fit for current resolution and load 'new_filename' if not
      if (loaded_filename != new_filename)
      {
         load_map(new_filename);
      }
      else
      {
         vk->resize(i_width, i_height);
      }

      if (build_textures)
      {
         build_keys_tex();
      }
   }
}

vkb_file_info mws_vkb_impl::get_closest_vkb_match(uint32_t i_width, uint32_t i_height)
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
   auto cmp_aspect_ratio = [](const vkb_file_info& i_a, const vkb_file_info& i_b) { return i_a.info.aspect_ratio < i_b.info.aspect_ratio; };
   std::sort(vkb_info_vect.begin(), vkb_info_vect.end(), cmp_aspect_ratio);

   // find the vkb file with the closest match to the screen's aspect ratio
   auto closest_val = [&cmp_aspect_ratio](const std::vector<vkb_file_info>& i_vect, float i_value) -> const vkb_file_info&
   {
      vkb_file_info vkb_i_t;
      vkb_i_t.info.aspect_ratio = i_value;
      auto const it = std::lower_bound(i_vect.begin(), i_vect.end(), vkb_i_t, cmp_aspect_ratio);

      // if we have an exact match, return 'it'
      if (it == i_vect.end())
      {
         return i_vect.back();
      }

      // we don't have an exact match. return the closest value
      float upper_diff = it->info.aspect_ratio - i_value;

      // first check if i_value is less than last aspect ration and that we have a lower aspect ratio
      if (upper_diff > 0.f && it > i_vect.begin())
      {
         auto const it_lower = it - 1;
         float lower_diff = i_value - it_lower->info.aspect_ratio;

         // if i_value is closer to the lower aspect ratio, return 'it_lower'
         if (lower_diff < upper_diff)
         {
            return *it_lower;
         }
         // i_value is closer to the upper/last aspect ratio, so return 'it'
         else
         {
            return *it;
         }
      }
      // if we don't, then 'it' is already the closest available aspect ratio. return 'it'
      else
      {
         return *it;
      }
   };

   // found it
   const vkb_file_info& vkb_i_t = closest_val(vkb_info_vect, screen_aspect_ratio);

   return vkb_i_t;
}

mws_sp<mws_font> mws_vkb_impl::get_font() const
{
   return letter_font;
}

void mws_vkb_impl::set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt)
{
   letter_font = mws_font::nwi(i_letter_fnt);
   letter_font->set_color(gfx_color::colors::yellow);
   word_font = mws_font::nwi(i_word_fnt);
   word_font->set_color(gfx_color::colors::yellow);
}

void mws_vkb_impl::start_anim()
{
   bool valid_state = visible && keys_quad && key_border_quad && !is_mod_key_held(mod_key_types::hide_vkb);

   if (valid_state)
   {
      fade_slider.start(fade_duration_in_seconds);
      fade_type = fade_types::e_show_vkb;
      keys_bg_outline_quad->visible = keys_quad->visible = key_border_quad->visible = true;
   }
}

void mws_vkb_impl::done()
{
   auto it = highlight_vect.begin();

   while (it != highlight_vect.end())
   {
      mws_sp<gfx_vxo> mesh = vk->vgeom->cell_borders->get_cell_borders_mesh_at(it->key_idx);
      (*mesh)["u_v1_transparency"] = 1.f;
      mesh->visible = false;
      ++it;
   }

   highlight_vect.clear();
   release_all_keys();
}

glm::ivec2 mws_vkb_impl::get_dimensions() const
{
   return vkb_dim;
}

std::string mws_vkb_impl::get_key_name(mws_key_types i_key_id) const
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

mws_key_types mws_vkb_impl::get_key_type(const std::string& i_key_name) const
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
         return (mws_key_types)ch;
      }
   }

   return mws_key_invalid;
}

void mws_vkb_impl::load_map(std::string i_filename)
{
   if (!file_store->file_exists(i_filename))
   {
      mws_println("file [ %s ] doesn't exist", i_filename.c_str());
      return;
   }

   std::string data = file_store->load_vkb(i_filename);
   auto db = kxmd::nwi(data.c_str(), data.length());
   kv_ref main = db->main();
   uint32_t point_count = 0;
   glm::ivec2 screen_dim(mws::screen::get_width(), mws::screen::get_height());
   glm::vec2 resize_fact(0.f);

   // size
   {
      kv_ref size = main["size"];
      mws_to(size[0].key(), diag_original_dim.x);
      mws_to(size[1].key(), diag_original_dim.y);
      resize_fact = glm::vec2(screen_dim) / diag_original_dim;
   }
   // pages
   {
      auto invalid_key_check = [](uint32_t i_key_id, const std::string& i_filename)
      {
         if (i_key_id == mws_key_invalid) { mws_println("warning[ invalid key in loaded vkb map [ %s ] ]", i_filename.c_str()); }
         mws_assert(i_key_id < mws_key_count);
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
            uint32_t size = keys_mod_none.size();

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
               for (uint32_t k = 0; k < size; k++)
               {
                  const std::string& key = keys_mod_none[k].key();
                  uint32_t key_id = 0;

                  mws_to<uint32_t>(key, key_id);
                  invalid_key_check(key_id, i_filename);
                  key_mod_vect[(uint32_t)key_mod_types::mod_none][k] = mws_key_types(key_id);
               }

               // mod-alt can be omitted
               if (keys_mod_alt)
               {
                  for (uint32_t k = 0; k < size; k++)
                  {
                     const std::string& key = keys_mod_alt[k].key();
                     uint32_t key_id = 0;

                     mws_to<uint32_t>(key, key_id);
                     invalid_key_check(key_id, i_filename);
                     key_mod_vect[(uint32_t)key_mod_types::mod_alt][k] = mws_key_types(key_id);
                  }
               }

               // mod-shift can be omitted
               if (keys_mod_shift)
               {
                  for (uint32_t k = 0; k < size; k++)
                  {
                     const std::string& key = keys_mod_shift[k].key();
                     uint32_t key_id = 0;

                     mws_to<uint32_t>(key, key_id);
                     invalid_key_check(key_id, i_filename);
                     key_mod_vect[(uint32_t)key_mod_types::mod_shift][k] = mws_key_types(key_id);
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

         for (uint32_t k = 0; k < point_count; k++)
         {
            glm::vec2 dim;
            mws_to(key_coord[2 * k + 0].key(), dim.x);
            mws_to(key_coord[2 * k + 1].key(), dim.y);
            dim *= resize_fact;

            key_coord_pos.push_back(dim);
         }

         vk->diag_data->info.original_diag_width = screen_dim.x;
         vk->diag_data->info.original_diag_height = screen_dim.y;
         vk->diag_data->info.diag_width = screen_dim.x;
         vk->diag_data->info.diag_height = screen_dim.y;
         vk->set_kernel_points(key_coord_pos);
      }
   }

   rebuild_key_state();
   loaded_filename = i_filename;
   mws_println("finished loading keyboard from [ %s ]", loaded_filename.c_str());
}

std::vector<mws_key_types>& mws_vkb_impl::get_key_vect()
{
   mws_assert(key_mod < key_mod_types::count);
   return key_mod_vect[(uint32_t)key_mod];
}

uint32_t mws_vkb_impl::get_key_vect_size()
{
   return key_mod_vect[(uint32_t)key_mod_types::mod_none].size();
}

mws_key_types mws_vkb_impl::get_key_at(int i_idx)
{
   return get_key_vect()[i_idx];
}

mws_key_types mws_vkb_impl::get_mod_key_at(key_mod_types i_key_mod, int i_idx)
{
   mws_assert(i_key_mod < key_mod_types::count);
   return key_mod_vect[(uint32_t)i_key_mod][i_idx];
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

std::vector<mws_sp<gfx_tex>> mws_vkb_impl::get_tex_list()
{
   std::vector<mws_sp<gfx_tex>> vect;

   vect.push_back(cell_border_tex);
   vect.push_back(key_border_tex.get_tex());

   for (auto& p : keys_tex)
   {
      vect.push_back(p.get_tex());
   }

   return vect;
}

void mws_vkb_impl::build_cell_border_tex()
{
   std::string tex_id = "mws-vkb-cell-border-tex";
   cell_border_tex = gi()->tex.get_texture_by_name(tex_id);

   if (!cell_border_tex)
   {
      gfx_color_mixer pal;
      gfx_tex_params prm;

      pal.set_color_at(gfx_color(0, 0, 0), 0.f);
      pal.set_color_at(gfx_color(75, 0, 0), 0.5f);
      pal.set_color_at(gfx_color(185, 0, 0), 0.85f);
      pal.set_color_at(gfx_color(255, 100, 0), 0.975f);
      pal.set_color_at(gfx_color(255, 240, 120), 0.98f);
      pal.set_color_at(gfx_color(255, 255, 150), 1.f);

      prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_repeat;
      prm.max_anisotropy = 0.f;
      prm.gen_mipmaps = true;
      prm.min_filter = gfx_tex_params::e_tf_linear_mipmap_linear;
      prm.mag_filter = gfx_tex_params::e_tf_linear;
      //prm.gen_mipmaps = false;
      //prm.min_filter = prm.mag_filter = gfx_tex_params::e_tf_nearest;

      cell_border_tex = gi()->tex.nwi(tex_id, 1024, 1, &prm);

      uint32_t width = cell_border_tex->get_width();
      uint32_t height = cell_border_tex->get_height();
      std::vector<gfx_color> pixel_data(width * height);
      uint32_t hw = width / 2;

      for (uint32_t k = 0; k < height; k++)
      {
         for (uint32_t i = 0; i < hw; i++)
         {
            float alpha = float(i) / (hw - 1);
            gfx_color color = pal.get_color_at(alpha);
            uint32_t il = k * width + i;
            uint32_t ir = k * width + width - 1 - i;

            pixel_data[il] = color;
            pixel_data[ir] = color;
         }
      }

      cell_border_tex->update(0, (char*)pixel_data.data());
   }

   vk->vgeom->cell_borders->set_cell_borders_tex(cell_border_tex);
}

void mws_vkb_impl::build_keys_tex()
{
   glm::ivec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());
   res_specific_params res_params = get_closest_resolution_match(glm::max(scr_dim.x, scr_dim.y));

   if (key_border_quad)
   {
      key_border_quad->detach();
      keys_bg_outline_quad->detach();
      keys_quad->detach();
   }

   if (!vkb_keys_fonts_shader)
   {
      init_shaders();
   }

   setup_font_dimensions();
   keys_tex.resize((uint32_t)key_mod_types::count);
   uint32_t key_map_size = keys_tex.size();
   mws_sp<mws_camera> g = get_mod()->mws_cam;
   glm::vec2 dim = letter_font->get_text_dim("M");
   std::string vkb_type = (diag_original_dim.x > diag_original_dim.y) ? "landscape" : "portrait";

   // key border
   {
      key_border_tex.init(mws_to_str_fmt("mws-vkb-keys-border-tex-%s-%d-%d", vkb_type.c_str(), scr_dim.x, scr_dim.y), scr_dim.x, scr_dim.y);
      key_border_quad = gfx_quad_2d::nwi();

      auto& rvxo = *key_border_quad;
      rvxo[MP_SHADER_NAME] = "mws-vkb-keys-fonts";
      rvxo[MP_BLENDING] = MV_ADD;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo["u_v1_transparency"] = 1.f;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = key_border_tex.get_tex();
      rvxo.name = "mws-vkb-keys-border-quad";
      rvxo.camera_id_list = { "mws_cam" };
      rvxo.set_scale((float)scr_dim.x, (float)scr_dim.y);
      rvxo.set_v_flip(true);
      // key_border_quad must be drawn before keys_quad
      rvxo.set_z(0.98f);
      attach(key_border_quad);

      {
         vk->vgeom->nexus_pairs_mesh->visible = true;
         gfx::i()->rt.set_current_render_target(key_border_tex.get_rt());
         gfx_rt::clear_buffers();
         //rvxo.draw_out_of_sync(g);
         float line_thickness = std::max(mws::screen::get_width(), mws::screen::get_height()) * 0.007f;
         (*vk->vgeom->nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness;
         (*vk->vgeom->nexus_pairs_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 1.f, 1.f);
         vk->vgeom->nexus_pairs_mesh->draw_out_of_sync(g);
         (*vk->vgeom->nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness / 5.f;
         (*vk->vgeom->nexus_pairs_mesh)["u_v4_color"] = glm::vec4(1.f, 1.f, 1.f, 1.f);
         vk->vgeom->nexus_pairs_mesh->draw_out_of_sync(g);
         gfx::i()->rt.set_current_render_target();

         mws_sp<mws_kawase_bloom> bloom = mws_kawase_bloom::nwi(key_border_tex.get_tex());
         auto& rvxo = *key_border_tex.get_quad();
         std::vector<float> weight_fact =
         {
            0.505f, 0.525f
         };
         bloom->set_iter_count(weight_fact.size(), weight_fact);
         bloom->update();

         rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
         rvxo[MP_BLENDING] = MV_NONE;
         rvxo["u_s2d_tex"][MP_TEXTURE_INST] = bloom->get_bloom_tex();
         rvxo.set_v_flip(true);

         gfx::i()->rt.set_current_render_target(key_border_tex.get_rt());
         rvxo.draw_out_of_sync(g);
         (*vk->vgeom->nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness / 8.f;
         (*vk->vgeom->nexus_pairs_mesh)["u_v4_color"] = glm::vec4(1.f, 1.f, 1.f, 1.f);
         //vk->vgeom->nexus_pairs_mesh->draw_out_of_sync(g);
         gfx::i()->rt.set_current_render_target();
         vk->vgeom->nexus_pairs_mesh->visible = false;
      }
   }

   for (uint32_t k = 0; k < key_map_size; k++)
   {
      mws_gfx_ppb& ppb = keys_tex[k];
      ppb.init(mws_to_str_fmt("keys-map-%s-%d-%d-%d", vkb_type.c_str(), k, scr_dim.x, scr_dim.y), scr_dim.x, scr_dim.y);
   }

   // keys
   {
      {
         keys_bg_outline_quad = gfx_quad_2d::nwi();
         auto& rvxo = *keys_bg_outline_quad;

         rvxo[MP_SHADER_NAME] = "mws-vkb-keys-outline";
         rvxo[MP_BLENDING] = MV_ALPHA;
         rvxo[MP_DEPTH_TEST] = false;
         rvxo[MP_DEPTH_WRITE] = false;
         rvxo["u_v1_outline_transparency"] = 0.85f;
         rvxo["u_v1_transparency"] = 1.f;
         rvxo["u_s2d_tex"][MP_TEXTURE_INST] = keys_tex[0].get_tex();
         rvxo.name = "mws-vkb-keys-quad-bg-outline";
         rvxo.camera_id_list = { "mws_cam" };
         rvxo.set_scale((float)scr_dim.x, (float)scr_dim.y);
         rvxo.set_v_flip(true);
         // keys_quad must be drawn after (on top of) key_border_quad
         rvxo.set_z(0.97f);
         attach(keys_bg_outline_quad);
      }
      {
         keys_quad = gfx_quad_2d::nwi();
         auto& rvxo = *keys_quad;

         rvxo[MP_SHADER_NAME] = "mws-vkb-keys-fonts";
         rvxo[MP_BLENDING] = MV_ADD_COLOR;
         rvxo[MP_DEPTH_TEST] = false;
         rvxo[MP_DEPTH_WRITE] = false;
         rvxo["u_v1_transparency"] = 1.f;
         rvxo["u_s2d_tex"][MP_TEXTURE_INST] = keys_tex[0].get_tex();
         rvxo.name = "mws-vkb-keys-quad";
         rvxo.camera_id_list = { "mws_cam" };
         rvxo.set_scale((float)scr_dim.x, (float)scr_dim.y);
         rvxo.set_v_flip(true);
         // keys_quad must be drawn after (on top of) key_border_quad
         rvxo.set_z(0.99f);
         attach(keys_quad);
      }
   }

   auto root = get_mws_root();
   auto vd = vk->get_diag_data();
   mws_vrn_kernel_pt_vect& kp_vect = vd->geom.kernel_points;
   std::array<key_mod_types, (uint32_t)key_mod_types::count> mod_vect = { key_mod_types::mod_none, key_mod_types::mod_alt, key_mod_types::mod_shift };

   // keys bg font outline
   {
      mws_sp<mws_font> letter_font_bg_outline;
      mws_sp<mws_font> word_font_bg_outline;
      float bg_outline_scale = 5.f;
      gfx_color bg_outline_color = gfx_color::colors::black;
      {
         mws_font_markup mk = { mws_font_rendermode::e_outline_positive, bg_outline_scale };
         letter_font_bg_outline = mws_font::nwi(letter_font, letter_font->get_size(), &mk);
         letter_font_bg_outline->set_color(bg_outline_color);
      }
      {
         mws_font_markup mk = { mws_font_rendermode::e_outline_positive, bg_outline_scale };
         word_font_bg_outline = mws_font::nwi(word_font, word_font->get_size(), &mk);
         word_font_bg_outline->set_color(bg_outline_color);
      }

      // draw keys bg outline
      for (uint32_t k = 0; k < key_map_size; k++)
      {
         gfx::i()->rt.set_current_render_target(keys_tex[k].get_rt());
         gfx_rt::clear_buffers(true, true, true, gfx_color::from_float(0.f, 0.f, 0.f, 0.35f));
         draw_keys(g, letter_font_bg_outline, word_font_bg_outline, mod_vect[k], kp_vect);
         g->update_camera_state();
      }

      gfx::i()->rt.set_current_render_target();
   }

   // disable writing to alpha channel
   mws_sp<mws_text_vxo> cam_txt_vxo = g->get_text_vxo();
   (*cam_txt_vxo)[MP_COLOR_WRITE] = glm::bvec4(true, true, true, false);
   g->set_text_blending(MV_ADD);

   // draw the key letters/words
   {
      mws_sp<mws_font> letter_font_bg = mws_font::nwi(letter_font);
      mws_sp<mws_font> word_font_bg = mws_font::nwi(word_font);
      gfx_color argb(16, 95, 223, 255);
      letter_font_bg->set_color(argb);
      word_font_bg->set_color(argb);

      // draw keys
      for (uint32_t k = 0; k < key_map_size; k++)
      {
         gfx::i()->rt.set_current_render_target(keys_tex[k].get_rt());
         draw_keys(g, letter_font_bg, word_font_bg, mod_vect[k], kp_vect);
         g->update_camera_state();
      }

      gfx::i()->rt.set_current_render_target();
   }

   // apply bloom
   {
      for (uint32_t k = 0; k < key_map_size; k++)
      {
         mws_sp<mws_kawase_bloom> bloom = mws_kawase_bloom::nwi(keys_tex[k].get_tex());
         const std::vector<float>& weight_fact = res_params.key_weight_fact;
         mws_sp<mws_font> letter_font_bg;
         mws_sp<mws_font> word_font_bg;

         letter_font_bg = mws_font::nwi(letter_font);
         word_font_bg = mws_font::nwi(word_font);
         letter_font_bg->set_color(gfx_color::colors::white);
         word_font_bg->set_color(gfx_color::colors::white);
         //{
         //   mws_font_markup mk = { mws_font_rendermode::e_outline_negative, 0.2f };
         //   letter_font_bg = mws_font::nwi(letter_font, letter_font->get_size(), &mk);
         //   letter_font_bg->set_color(gfx_color::colors::white);
         //}
         //{
         //   mws_font_markup mk = { mws_font_rendermode::e_outline_negative, 0.2f };
         //   word_font_bg = mws_font::nwi(word_font, word_font->get_size(), &mk);
         //   word_font_bg->set_color(gfx_color::colors::white);
         //}

         bloom->set_iter_count(weight_fact.size(), weight_fact);
         bloom->set_alpha_op_type(mws_kawase_bloom::e_set_alpha_to_original);
         bloom->update();

         auto& rvxo = *keys_tex[k].get_quad();
         rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
         rvxo[MP_BLENDING] = MV_NONE;
         rvxo["u_s2d_tex"][MP_TEXTURE_INST] = bloom->get_bloom_tex();
         rvxo.set_v_flip(true);

         // draw the white keys on top
         gfx::i()->rt.set_current_render_target(keys_tex[k].get_rt());
         rvxo.draw_out_of_sync(g);
         draw_keys(g, letter_font_bg, word_font_bg, mod_vect[k], kp_vect);
         g->update_camera_state();
      }

      gfx::i()->rt.set_current_render_target();
   }

   // reenable writing to alpha channel
   (*cam_txt_vxo)[MP_COLOR_WRITE] = glm::bvec4(true, true, true, true);
   g->set_text_blending(MV_ALPHA);
   set_key_transparency(0.f);

   if(!pressed_key)
   {
      pressed_key = std::make_shared<mws_vkb_pressed_key>();
      pressed_key->init(cell_border_tex);
      attach(pressed_key);
   }
}

void mws_vkb_impl::show_pressed_key(const mws_sp<mws_text_area> i_ta, uint32_t i_key_idx)
{
   mws_key_types key_id = get_mod_key_at(key_mod_types::mod_none, i_key_idx);

   if (!is_mod_key(key_id) && !i_ta->is_action_key(key_id))
   {
      pressed_key->show_pressed_key(i_ta, i_key_idx, vk, keys_tex[(uint32_t)key_mod].get_tex());
   }
}

bool mws_vkb_impl::is_mod_key(mws_key_types i_key_id)
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
      if (it->second == static_cast<mws_key_types>(i_mod_key))
      {
         const base_key_state& st = base_key_st[it->first];

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

      if (st.key_id == static_cast<mws_key_types>(i_mod_key))
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
      set_key_state(mod_key_idx, state);
   }
}

void mws_vkb_impl::clear_mod_key_locks()
{
   for (auto it = mod_keys_st.begin(); it != mod_keys_st.end(); ++it)
   {
      set_key_state(it->first, base_key_state_types::key_free);
   }
}

void mws_vkb_impl::release_all_keys(bool i_release_locked_keys)
{
   uint32_t size = base_key_st.size();

   for (uint32_t k = 0; k < size; ++k)
   {
      base_key_state& st = base_key_st[k];

      if (i_release_locked_keys || (!i_release_locked_keys && st.state != base_key_state_types::key_locked))
      {
         set_key_state(k, base_key_state_types::key_free);
      }
   }

   get_mod()->key_ctrl_inst->clear_keys();
   prev_ptr_evt = nullptr;

   if (pressed_key)
   {
      pressed_key->visible = false;
   }
}


mws_vkb_impl::mws_vkb_pressed_key::mws_vkb_pressed_key() : gfx_node(nullptr) {}

void mws_vkb_impl::mws_vkb_pressed_key::init(mws_sp<gfx_tex> i_cell_border_tex)
{
   {
      arrow = std::make_shared<gfx_vxo>(vx_info("a_v3_position, a_v2_tex_coord"));
      gfx_vxo& rvxo = *arrow;

      rvxo.camera_id_list = { "mws_cam" };
      rvxo.name = "mws-vkb-pressed-key-arrow";
      rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
      rvxo["u_v4_color"] = arrow_color;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo[MP_CULL_FRONT] = false;
      rvxo[MP_CULL_BACK] = false;
      rvxo[MP_BLENDING] = MV_ALPHA;
      // arrow must be drawn on top of the keyboard
      rvxo.position = glm::vec3(0.f, 0.f, 1.96f);
   }
   {
      label_bg = std::make_shared<gfx_vxo>(vx_info("a_v3_position, a_v2_tex_coord"));
      gfx_vxo& rvxo = *label_bg;

      rvxo.camera_id_list = { "mws_cam" };
      rvxo.name = "mws-vkb-pressed-key-label-bg";
      rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
      rvxo["u_v4_color"] = label_bg_color;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo[MP_CULL_FRONT] = false;
      rvxo[MP_CULL_BACK] = false;
      rvxo[MP_BLENDING] = MV_ALPHA;
      // label_bg must be drawn on top of the keyboard
      rvxo.position = glm::vec3(0.f, 0.f, 1.97f);
   }
   {
      label = std::make_shared<gfx_vxo>(vx_info("a_v3_position, a_v2_tex_coord"));
      gfx_vxo& rvxo = *label;

      rvxo.camera_id_list = { "mws_cam" };
      rvxo.name = "mws-vkb-pressed-key-label";
      rvxo[MP_SHADER_NAME] = "mws-vkb-hsv-shift";
      rvxo["u_v1_hue_val"] = 0.38f;
      //rvxo["u_v1_id"] = 1.f;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo[MP_CULL_FRONT] = false;
      rvxo[MP_CULL_BACK] = false;
      rvxo[MP_BLENDING] = MV_ADD_COLOR;
      // label must be drawn on top of the keyboard
      rvxo.position = glm::vec3(0.f, 0.f, 1.99f);
   }
   {
      border = mws_vrn_cell_vxo::nwi();
      gfx_vxo& rvxo = *border;

      rvxo.camera_id_list = { "mws_cam" };
      rvxo.name = "mws-vkb-pressed-key-border";
      rvxo[MP_SHADER_NAME] = "mws-vkb-hsv-shift";
      rvxo["u_v1_hue_val"] = 0.85f;
      //rvxo["u_v1_id"] = 2.f;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = i_cell_border_tex;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo[MP_CULL_FRONT] = false;
      rvxo[MP_CULL_BACK] = false;
      rvxo[MP_BLENDING] = MV_ADD_COLOR;
      // border must be drawn on top of the keyboard
      rvxo.position = glm::vec3(0.f, 0.f, 1.98f);
   }
   {
      attach(arrow);
      attach(label_bg);
      attach(label);
      attach(border);
      visible = false;
   }
}

void mws_vkb_impl::mws_vkb_pressed_key::show_pressed_key
(const mws_sp<mws_text_area> i_ta, uint32_t i_key_idx, mws_sp<mws_vrn_main> i_vk, mws_sp<gfx_tex> i_keys_tex)
{
   glm::vec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());
   mws_sp<mws_vrn_cell_vxo> cell_border = i_vk->vgeom->get_cell_borders()->get_cell_borders_mesh_at(i_key_idx);
   glm::vec4 bb = cell_border->bounding_box;
   glm::vec2 cell_dim(bb.z - bb.x, bb.w - bb.y);
   mws_rect cursor_rect = i_ta->get_cursor_rect(mws_text_area::e_middle_vertical_cursor);
   glm::vec2 cell_pos = glm::vec2(cursor_rect.x, cursor_rect.y) - glm::vec2(bb.x, bb.y);
   glm::vec2 cursor_arrow_base = glm::vec2(cursor_rect.x, cursor_rect.y);
   glm::vec2 cell_offset(std::min(scr_dim.x, scr_dim.y) * 0.05f);
   glm::vec2 quadrant(1.f);
   {
      const glm::vec2& kernel_pos = cell_border->kernel_pos;
      uint32_t triangle_count = cell_border->nexus_pos_vect.size();
      uint32_t vx_count = triangle_count + 1;
      uint32_t ix_count = triangle_count * 3;
      gfx_vxo& r_label = *label;
      gfx_vxo& r_label_bg = *label_bg;
      r_label.set_size(vx_count, ix_count);
      r_label_bg.set_size(vx_count, ix_count);
      std::vector<gfx_indices_type>& ix_label = r_label.get_ix_buffer();
      std::vector<gfx_indices_type>& ix_label_bg = r_label_bg.get_ix_buffer();
      std::vector<uint8_t>& vx_label = r_label.get_vx_buffer();
      std::vector<uint8_t>& vx_label_bg = r_label_bg.get_vx_buffer();
      mws_vrn_cell_borders::vx_fmt_3f_2f* vx_label_data = reinterpret_cast<mws_vrn_cell_borders::vx_fmt_3f_2f*>(vx_label.data());
      mws_vrn_cell_borders::vx_fmt_3f_2f* vx_label_bg_data = reinterpret_cast<mws_vrn_cell_borders::vx_fmt_3f_2f*>(vx_label_bg.data());
      vx_label_bg_data[0] = vx_label_data[0] = { glm::vec3(kernel_pos, 0.f), glm::vec2(kernel_pos.x, scr_dim.y - kernel_pos.y) / scr_dim };

      for (uint32_t k = 0, idx = 0; k < triangle_count; k++)
      {
         const glm::vec2& pos = cell_border->nexus_pos_vect[k];
         vx_label_bg_data[k + 1] = vx_label_data[k + 1] = { glm::vec3(pos, 0.f), glm::vec2(pos.x, scr_dim.y - pos.y) / scr_dim };
         ix_label_bg[idx] = ix_label[idx] = 0;
         idx++;
         ix_label_bg[idx] = ix_label[idx] = (k + 2 > triangle_count) ? 1 : k + 2;
         idx++;
         ix_label_bg[idx] = ix_label[idx] = k + 1;
         idx++;
      }

      r_label.update_data();
      r_label_bg.update_data();
      r_label["u_s2d_tex"][MP_TEXTURE_INST] = i_keys_tex;
   }
   {
      uint32_t vx_count = cell_border->get_vx_buffer().size() / sizeof(mws_vrn_cell_borders::vx_fmt_3f_2f);
      uint32_t ix_count = cell_border->get_ix_buffer().size();
      gfx_vxo& rvxo = *border;
      rvxo.set_size(vx_count, ix_count);
      std::vector<gfx_indices_type>& ks_indices_data = rvxo.get_ix_buffer();
      ks_indices_data = cell_border->get_ix_buffer();
      std::vector<uint8_t>& ks_vertices_data = rvxo.get_vx_buffer();
      ks_vertices_data = cell_border->get_vx_buffer();
      rvxo.update_data();
   }

   if (cursor_rect.x >= scr_dim.x / 2.f)
   {
      quadrant.x = -1.f;
      cell_pos.x -= cell_dim.x;
   }
   if (cursor_rect.y >= scr_dim.y / 2.f)
   {
      quadrant.y = -1.f;
      cell_pos.y -= cell_dim.y;
   }
   else
   {
      cell_pos.y += cursor_rect.h;
      cursor_arrow_base.y += cursor_rect.h;
   }

   cell_pos += quadrant * cell_offset;

   {
      uint32_t vx_count = 3;
      uint32_t ix_count = 3;
      gfx_vxo& rvxo = *arrow;
      rvxo.set_size(vx_count, ix_count);
      std::vector<gfx_indices_type>& ks_indices_data = rvxo.get_ix_buffer();
      std::vector<uint8_t>& ks_vertices_data = rvxo.get_vx_buffer();
      mws_vrn_cell_borders::vx_fmt_3f_2f* vx_data = reinterpret_cast<mws_vrn_cell_borders::vx_fmt_3f_2f*>(ks_vertices_data.data());
      const std::vector<glm::vec2>& nexus_vect = cell_border->nexus_pos_vect;
      uint32_t nexus_count = nexus_vect.size();
      std::vector<std::pair<uint32_t, float>> nexus_idx_dist_sq(nexus_count);

      for (uint32_t k = 0; k < nexus_count; k++)
      {
         glm::vec2 nexus = nexus_vect[k] + cell_pos;
         nexus_idx_dist_sq[k] = { k, glm::distance2(cursor_arrow_base, nexus) };
      }

      static auto sort_cmp = [](const std::pair<uint32_t, float>& i_d0, const std::pair<uint32_t, float>& i_d1)
      {
         return (i_d0.second < i_d1.second);
      };

      std::sort(nexus_idx_dist_sq.begin(), nexus_idx_dist_sq.end(), sort_cmp);

      vx_data[0] = { glm::vec3(cursor_arrow_base - cell_pos, 0.f), glm::vec2(0.f) };
      vx_data[1] = { glm::vec3(nexus_vect[nexus_idx_dist_sq[0].first], 0.f), glm::vec2(0.f) };
      vx_data[2] = { glm::vec3(nexus_vect[nexus_idx_dist_sq[1].first], 0.f), glm::vec2(0.f) };
      ks_indices_data = { 0, 2, 1 };
      rvxo.update_data();
   }

   visible = true;
   position = glm::vec3(cell_pos, 1.f);
   start_light_turnoff();
   set_fade_gradient(1.f);
}

void mws_vkb_impl::mws_vkb_pressed_key::start_light_turnoff()
{
   light_turnoff_start = mws::time::get_time_millis();
}

bool mws_vkb_impl::mws_vkb_pressed_key::is_fading(uint32_t i_crt_time)
{
   return (i_crt_time - light_turnoff_start) / 1000.f > pressed_key_lights_hold_seconds;
}

void mws_vkb_impl::mws_vkb_pressed_key::set_fade_gradient(float i_gradient)
{
   (*arrow)["u_v4_color"] = arrow_color * i_gradient;
   (*label)["u_v1_transparency"] = i_gradient;
   (*label_bg)["u_v4_color"] = label_bg_color * i_gradient;
   (*border)["u_v1_transparency"] = i_gradient;
}


mws_vkb_impl::res_specific_params mws_vkb_impl::get_closest_resolution_match(uint32_t i_resolution_px)
{
   if (i_resolution_px <= resolution_params.front().resolution_px)
   {
      return resolution_params.front();
   }
   else if (i_resolution_px >= resolution_params.back().resolution_px)
   {
      return resolution_params.back();
   }

   auto cmp_resolution = [](const res_specific_params& i_a, const res_specific_params& i_b) { return i_a.resolution_px < i_b.resolution_px; };
   auto mix_params = [](const res_specific_params& i_lower, const res_specific_params& i_upper, uint32_t i_resolution_px)
   {
      res_specific_params params;
      const uint32_t max_weight_size = glm::max(i_lower.key_weight_fact.size(), i_upper.key_weight_fact.size());
      float lerpf = float(i_resolution_px - i_lower.resolution_px) / float(i_upper.resolution_px - i_lower.resolution_px);
      std::vector<float> lower(max_weight_size, 0.f);
      std::vector<float> upper(max_weight_size, 0.f);

      std::copy(i_lower.key_weight_fact.begin(), i_lower.key_weight_fact.end(), lower.begin());
      std::copy(i_upper.key_weight_fact.begin(), i_upper.key_weight_fact.end(), upper.begin());
      params.resolution_px = (uint32_t)glm::mix(i_lower.resolution_px, i_upper.resolution_px, lerpf);
      params.key_weight_fact.resize(max_weight_size);

      for (uint32_t k = 0; k < max_weight_size; k++)
      {
         params.key_weight_fact[k] = glm::mix(lower[k], upper[k], lerpf);
      }

      //mws_println("res_specific_params[ res %d lerpf %f [ %d, %d ] ]", params.resolution_px, lerpf, i_lower.resolution_px, i_upper.resolution_px);
      return params;
   };

   res_specific_params t_params = { i_resolution_px, {} };
   auto const it = std::lower_bound(resolution_params.begin(), resolution_params.end(), t_params, cmp_resolution);

   // exact match
   if (i_resolution_px == it->resolution_px)
   {
      return *it;
   }
   // not an exact match. interpolate
   else
   {
      auto const it_lower = it - 1;
      return mix_params(*it_lower, *it, i_resolution_px);
   }
}

void mws_vkb_impl::setup_font_dimensions()
{
   uint32_t vkb_size = std::min(mws::screen::get_width(), mws::screen::get_height());
   mws_px letter_font_height(vkb_size / 5.f / 2.5f, mws_dim::e_vertical);
   mws_px word_font_height(vkb_size / 5.f / 4.5f, mws_dim::e_vertical);
   mws_sp<mws_font> font = mws_font_db::inst()->get_global_font();
   mws_sp<mws_font> letter_font = mws_font::nwi(font, letter_font_height);
   mws_sp<mws_font> word_font = mws_font::nwi(font, word_font_height);

   set_font(letter_font, word_font);
}

void mws_vkb_impl::init_shaders()
{
   // keys fonts shader
   vkb_keys_fonts_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_keys_fonts_sh, true);
   if (!vkb_keys_fonts_shader)
   {
      std::string vsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      layout (location = 0) in vec3 a_v3_position;
      layout (location = 1) in vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      smooth out vec2 v_v2_tex_coord;

      void main()
      {
	      v_v2_tex_coord = a_v2_tex_coord;
	      vec4 v4_position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
	
	      gl_Position = v4_position;
      }
      )");

      std::string fsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision lowp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform sampler2D u_s2d_tex;
      uniform float u_v1_transparency;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
	      vec3 v3_color = texture(u_s2d_tex, v_v2_tex_coord).rgb;
	      vec4 v4_color = vec4(v3_color, 1.) * u_v1_transparency;
	
	      v4_frag_color = v4_color;
      }
      )");

      vkb_keys_fonts_shader = gfx::i()->shader.new_program_from_src(vkb_keys_fonts_sh, vsh, fsh);
   }

   // keys outline shader
   vkb_keys_outline_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_keys_outline_sh, true);
   if (!vkb_keys_outline_shader)
   {
      std::string vsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      layout (location = 0) in vec3 a_v3_position;
      layout (location = 1) in vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      smooth out vec2 v_v2_tex_coord;

      void main()
      {
	      v_v2_tex_coord = a_v2_tex_coord;
	      vec4 v4_position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
	
	      gl_Position = v4_position;
      }
      )");

      std::string fsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision lowp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform sampler2D u_s2d_tex;
      uniform float u_v1_outline_transparency;
      uniform float u_v1_transparency;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
	      float v1_tex_alpha = texture(u_s2d_tex, v_v2_tex_coord).a;
	      float v1_alpha_val = v1_tex_alpha * u_v1_outline_transparency * u_v1_transparency;
	
	      v4_frag_color = vec4(vec3(0.), v1_alpha_val);
      }
      )");

      vkb_keys_outline_shader = gfx::i()->shader.new_program_from_src(vkb_keys_outline_sh, vsh, fsh);
   }
   // hsv shift
   vkb_hsv_shift_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_hsv_shift_sh, true);
   if (!vkb_hsv_shift_shader)
   {
      std::string vsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      layout (location = 0) in vec3 a_v3_position;
      layout (location = 1) in vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      smooth out vec2 v_v2_tex_coord;

      void main()
      {
	      v_v2_tex_coord = a_v2_tex_coord;
	      vec4 v4_position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
	
	      gl_Position = v4_position;
      }
      )");

      std::string fsh(R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision lowp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform float u_v1_hue_val;
      uniform float u_v1_transparency;
      uniform sampler2D u_s2d_tex;

      smooth in vec2 v_v2_tex_coord;

      vec3 rgb2hsv(vec3 c)
      {
            vec4 k = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
            vec4 p = mix(vec4(c.bg, k.wz), vec4(c.gb, k.xy), step(c.b, c.g));
            vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

            float d = q.x - min(q.w, q.y);
            float e = 1.0e-10;
            return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
      }

      vec3 hsv2rgb(vec3 c)
      {
            vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
            vec3 p = abs(fract(c.xxx + k.xyz) * 6.0 - k.www);
            return c.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), c.y);
      }

      void main()
      {
	      vec3 v3_diff_color = texture(u_s2d_tex, v_v2_tex_coord).rgb;
	      vec3 v3_hsv = rgb2hsv(v3_diff_color);
	      v3_hsv.r = u_v1_hue_val;
	      vec3 v3_rgb = hsv2rgb(v3_hsv);
	
	      v4_frag_color = vec4(v3_rgb, 1.) * u_v1_transparency;
      }
      )");

      vkb_hsv_shift_shader = gfx::i()->shader.new_program_from_src(vkb_hsv_shift_sh, vsh, fsh);
   }
}

void mws_vkb_impl::set_key_transparency(float i_alpha)
{
   (*key_border_quad)["u_v1_transparency"] = i_alpha;
   (*keys_bg_outline_quad)["u_v1_transparency"] = i_alpha;
   (*keys_quad)["u_v1_transparency"] = i_alpha;
}

void mws_vkb_impl::draw_keys(mws_sp<mws_draw_text> i_dt, mws_sp<mws_font> i_letter_font, mws_sp<mws_font> i_word_font,
   key_mod_types i_mod, mws_vrn_kernel_pt_vect& i_kp_vect)
{
   uint32_t size = get_key_vect_size();

   for (uint32_t k = 0; k < size; k++)
   {
      mws_key_types key_id = get_mod_key_at(i_mod, k);
      std::string key = get_key_name(key_id);

      // exclude space bar
      if (!key.empty())
      {
         mws_sp<mws_font> font = (key.length() > 1) ? i_word_font : i_letter_font;
         auto& kp = i_kp_vect[k];
         glm::vec2 text_dim = font->get_text_dim(key);
         glm::vec2 pos(kp.position.x, kp.position.y);
         pos -= text_dim / 2.f;

         i_dt->draw_text(key, pos.x, pos.y, font);
      }
   }
}

void mws_vkb_impl::set_key_vect_size(uint32_t i_size)
{
   for (uint32_t k = 0; k < (uint32_t)key_mod_types::count; k++)
   {
      key_mod_vect[k].resize(i_size);
   }
}

void mws_vkb_impl::set_key_at(int i_idx, mws_key_types i_key_id)
{
   get_key_vect()[i_idx] = i_key_id;

   if (is_mod_key(i_key_id))
   {
      rebuild_key_state();
   }
}

void mws_vkb_impl::erase_key_at(int i_idx)
{
   for (uint32_t k = 0; k < (uint32_t)key_mod_types::count; k++)
   {
      key_mod_vect[k].erase(key_mod_vect[k].begin() + i_idx);
   }

   rebuild_key_state();
}

void mws_vkb_impl::push_back_key(mws_key_types i_key_id)
{
   for (uint32_t k = 0; k < (uint32_t)key_mod_types::count; k++)
   {
      key_mod_vect[k].push_back(i_key_id);
   }

   rebuild_key_state();
}

bool mws_vkb_impl::touch_began(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   for (uint32_t k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];

      if (pt.is_changed)
      {
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
         int pos_idx = ret.idx;
         base_key_state& key_st = base_key_st[pos_idx];

         if (key_st.state == base_key_state_types::key_free)
         {
            auto& kc = get_mod()->key_ctrl_inst;
            kc->key_pressed(mws_key_types(pos_idx));

            if (i_ta)
            {
               pressed_key_ker_idx = pos_idx;
            }
         }

         if (mws_dbg::enabled(mws_dbg::app_touch))
         {
            mws_println("ptr-began[ app id [%d] key [%c] at [%d] ]", pt.identifier, (char)get_key_at(pos_idx), pos_idx);
         }

         if (key_st.key_id == VKB_HIDE_KB)
         {
            pressed_vkb_hide_finger_id = pt.identifier;
         }

         key_st.pressed_count++;
         bool keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_held);
      }
      else
      {
         if (mws_dbg::enabled(mws_dbg::app_touch))
         {
            auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

            mws_println("ptr-began[ app id [%d] key [%c] at [%d] ignored(changed == false) ]", pt.identifier, (char)get_key_at(ret.idx), ret.idx);
         }
      }
   }

   prev_ptr_evt = i_pe;

   return true;
}

bool mws_vkb_impl::touch_moved(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   if (!prev_ptr_evt)
   {
      prev_ptr_evt = i_pe;
   }

   for (uint32_t k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];

      if (pt.is_changed)
      {
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
         int pos_idx = ret.idx;
         base_key_state& key_st = base_key_st[pos_idx];
         const mws_ptr_evt::touch_point* same_id = prev_ptr_evt->find_point(pt.identifier);

         // found the same pointer id in the prev event
         if (same_id)
         {
            auto prev_ret = vk->get_kernel_idx_at(same_id->x, same_id->y);
            int prev_pos_idx = prev_ret.idx;

            // if changed keys from previous event, release the previous key, else, if they are on the same key, do nothing
            if (pos_idx != prev_pos_idx)
            {
               base_key_state& prev_key_st = base_key_st[prev_pos_idx];

               if (prev_key_st.pressed_count > 0)
               {
                  prev_key_st.pressed_count--;

                  if (prev_key_st.pressed_count == 0)
                  {
                     auto& kc = get_mod()->key_ctrl_inst;
                     kc->key_released(mws_key_types(prev_pos_idx));
                     bool keys_iter_invalidated = set_key_state(prev_pos_idx, base_key_state_types::key_free);

                     if (mws_dbg::enabled(mws_dbg::app_touch))
                     {
                        mws_println("ptr-moved[ app id [%d] released key [%c] at [%d] ]", pt.identifier, (char)get_key_at(prev_pos_idx), prev_pos_idx);
                     }
                  }
                  else
                  {
                     if (mws_dbg::enabled(mws_dbg::app_touch))
                     {
                        mws_println("ptr-moved[ app id []%d key [%c] at [%d], dec pressed count to [%d] ]",
                           pt.identifier, (char)get_key_at(prev_pos_idx), prev_pos_idx, (int)prev_key_st.pressed_count);
                     }
                  }
               }
               else
               {
                  if (mws_dbg::enabled(mws_dbg::app_touch))
                  {
                     mws_println("ptr-moved[ app id [%d] key [%c] at [%d], !ERROR![ pressed count already zero, possible BUG! ] ]",
                        pt.identifier, (char)get_key_at(prev_pos_idx), prev_pos_idx);
                  }
               }

               if (key_st.state == base_key_state_types::key_free)
               {
                  auto& kc = get_mod()->key_ctrl_inst;
                  kc->key_pressed(mws_key_types(pos_idx));

                  if (i_ta)
                  {
                     pressed_key_ker_idx = pos_idx;
                  }
               }

               if (key_st.key_id == VKB_HIDE_KB)
               {
                  pressed_vkb_hide_finger_id = pt.identifier;
               }

               key_st.pressed_count++;
               bool keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_held);

               if (mws_dbg::enabled(mws_dbg::app_touch))
               {
                  mws_println("ptr-moved[ app id [%d] key [%c] at [%d]. inc pressed count to [%d] ]",
                     pt.identifier, (char)get_key_at(pos_idx), pos_idx, (int)key_st.pressed_count);
               }
            }
         }
         // id not found?
         else
         {
            if (key_st.state == base_key_state_types::key_free)
            {
               auto& kc = get_mod()->key_ctrl_inst;
               kc->key_pressed(mws_key_types(pos_idx));
            }

            key_st.pressed_count++;
            bool keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_held);

            if (mws_dbg::enabled(mws_dbg::app_touch))
            {
               mws_println("ptr-moved[ app id [%d] key [%c] at [%d], id not found, set pressed_count to [%d] ]",
                  pt.identifier, (char)get_key_at(pos_idx), pos_idx, key_st.pressed_count);
            }
         }
      }
      else
      {
         if (mws_dbg::enabled(mws_dbg::app_touch))
         {
            auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

            mws_println("ptr-moved[ app id [%d] key [%c] at [%d] ignored(changed == false) ]", pt.identifier, (char)get_key_at(ret.idx), ret.idx);
         }
      }
   }

   prev_ptr_evt = i_pe;

   return true;
}

bool mws_vkb_impl::touch_ended(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   bool is_processed = false;

   if (!prev_ptr_evt)
   {
      mws_println("ptr-ended[ app prev_ptr_evt is NULL ]");
   }

   for (uint32_t k = 0; k < i_pe->touch_count; k++)
   {
      auto& pt = i_pe->points[k];

      if (pt.is_changed)
      {
         auto ret = vk->get_kernel_idx_at(pt.x, pt.y);
         int pos_idx = ret.idx;
         base_key_state& key_st = base_key_st[pos_idx];

         if (key_st.pressed_count > 0)
         {
            key_st.pressed_count--;
            is_processed = true;

            if (key_st.pressed_count == 0)
            {
               auto& kc = get_mod()->key_ctrl_inst;
               kc->key_released(mws_key_types(pos_idx));
               bool keys_iter_invalidated = set_key_state(pos_idx, base_key_state_types::key_free);

               // last pointer released
               if (i_pe->touch_count == 1)
               {
                  prev_ptr_evt = nullptr;

                  if (mws_dbg::enabled(mws_dbg::app_touch))
                  {
                     mws_println("ptr-ended[ app id [%d] released key [%c] at [%d]. prev_ptr_evt set to nullptr ]",
                        pt.identifier, (char)get_key_at(pos_idx), pos_idx);
                  }
               }
               else
               {
                  if (mws_dbg::enabled(mws_dbg::app_touch))
                  {
                     mws_println("ptr-ended[ app id [%d] released key [%c] at [%d] ]", pt.identifier, (char)get_key_at(pos_idx), pos_idx);
                  }
               }
            }
            else
            {
               if (mws_dbg::enabled(mws_dbg::app_touch))
               {
                  mws_println("ptr-ended[ app id [%d] key [%c] at [%d], dec pressed count to [%d] ]",
                     pt.identifier, (char)get_key_at(pos_idx), pos_idx, (int)key_st.pressed_count);
               }
            }
         }
         else
         {
            if (mws_dbg::enabled(mws_dbg::app_touch))
            {
               mws_println("ptr-ended[ app id [%d] key [%c] at [%d], !ERROR![ pressed count already zero, possible BUG! ] ]",
                  pt.identifier, (char)get_key_at(pos_idx), pos_idx);
            }
         }
      }
      else
      {
         if (mws_dbg::enabled(mws_dbg::app_touch))
         {
            auto ret = vk->get_kernel_idx_at(pt.x, pt.y);

            mws_println("ptr-ended[ app id [%d] key [%c] at [%d] ignored(changed == false) ]", pt.identifier, (char)get_key_at(ret.idx), ret.idx);
         }
      }
   }

   if (prev_ptr_evt != nullptr)
   {
      prev_ptr_evt = i_pe;
   }

   // last pointer released. release all stuck keys
   if (i_pe->touch_count == 1 && i_pe->points[0].is_changed)
   {
      uint32_t size = base_key_st.size();

      get_mod()->key_ctrl_inst->clear_keys();
      prev_ptr_evt = nullptr;

      for (uint32_t k = 0; k < size; ++k)
      {
         base_key_state& st = base_key_st[k];

         if (st.pressed_count > 0)
         {
            st.state = base_key_state_types::key_free;
            st.pressed_count = 0;
            fade_key_at(k);
         }
      }
   }

   return is_processed;
}

bool mws_vkb_impl::touch_cancelled(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta)
{
   if (i_pe->touch_count == 1)
   {
      get_mod()->key_ctrl_inst->clear_keys();
   }

   return false;
}

void mws_vkb_impl::highlight_key_at(int i_idx, bool i_light_on)
{
   auto& cell_borders = vk->vgeom->cell_borders;

   if (cell_borders->get_cell_borders_mesh_size() > 0)
   {
      mws_sp<gfx_vxo> mesh = cell_borders->get_cell_borders_mesh_at(i_idx);
      mesh->visible = i_light_on;

      for (auto it = highlight_vect.begin(); it != highlight_vect.end(); ++it)
      {
         if (it->key_idx == i_idx)
         {
            (*mesh)["u_v1_transparency"] = 1.f;
            highlight_vect.erase(it);
            break;
         }
      }
   }
}

void mws_vkb_impl::fade_key_at(int i_idx)
{
   auto& cell_borders = vk->vgeom->cell_borders;

   if (cell_borders->get_cell_borders_mesh_size() > 0)
   {
      for (auto& kh : highlight_vect)
      {
         if (kh.key_idx == i_idx)
         {
            kh.release_time = mws::time::get_time_millis();
            return;
         }
      }

      highlight_vect.push_back(key_highlight{ i_idx, mws::time::get_time_millis() });
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

   mws_key_types key_id = get_mod_key_at(key_mod_types::mod_none, i_key_idx);
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

         if (mws_dbg::enabled(mws_dbg::app_touch)) { mws_println("alt key_mod %d", (int)key_mod); }

         if (keys_quad)
         {
            mws_sp<gfx_tex> tex = keys_tex[(uint32_t)key_mod].get_tex();

            (*keys_bg_outline_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            (*keys_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            update_lights(i_key_idx, i_state);
         }
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

         if (mws_dbg::enabled(mws_dbg::app_touch)) { mws_println("shift key_mod %d state %d", (int)key_mod, (int)i_state); }

         if (keys_quad)
         {
            mws_sp<gfx_tex> tex = keys_tex[(uint32_t)key_mod].get_tex();

            (*keys_bg_outline_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            (*keys_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            update_lights(i_key_idx, i_state);
         }
         break;
      }

      case VKB_HIDE_KB:
      {
         if (keys_quad && key_border_quad)
         {
            bool show_vkb = (i_state == base_key_state_types::key_free);

            // if we need to show the keyboard
            if (show_vkb)
            {
               fade_slider.start(fade_duration_in_seconds);
               fade_type = fade_types::e_show_vkb;
               keys_bg_outline_quad->visible = keys_quad->visible = key_border_quad->visible = true;
               fade_key_at(i_key_idx);
            }
            // if we need to hide the keyboard
            else if (init_state != base_key_state_types::key_locked)
            {
               uint32_t size = base_key_st.size();

               fade_slider.start(fade_duration_in_seconds);
               fade_type = fade_types::e_hide_vkb;
               get_mod()->key_ctrl_inst->clear_keys();
               highlight_key_at(i_key_idx);

               for (uint32_t k = 0; k < size; ++k)
               {
                  if (k != i_key_idx)
                  {
                     highlight_key_at(k, false);
                     base_key_st[k].state = base_key_state_types::key_free;
                     base_key_st[k].pressed_count = 0;
                  }
               }

               key_mod = key_mod_types::mod_none;
               mws_sp<gfx_tex> tex = keys_tex[(uint32_t)key_mod].get_tex();
               (*keys_bg_outline_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
               (*keys_quad)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
               pressed_key->visible = false;

               return true;
            }
         }
         break;
      }
      }
   }
   else
   {
      if (key_border_quad)
      {
         update_lights(i_key_idx, i_state);
      }
   }

   return false;
}

void mws_vkb_impl::rebuild_key_state()
{
   std::vector<mws_key_types>& key_mod_default = key_mod_vect[(uint32_t)key_mod_types::mod_none];
   uint32_t size = key_mod_default.size();

   mod_keys_st.clear();
   base_key_st.resize(size);

   // set the base key state and also find and store the mod keys in a hashtable
   for (uint32_t k = 0; k < size; k++)
   {
      mws_key_types key_id = key_mod_default[k];

      if (is_mod_key(key_id))
      {
         mod_keys_st[k] = get_mod_key_at(key_mod_types::mod_none, k);
      }

      base_key_st[k] = base_key_state{ key_id, base_key_state_types::key_free, (uint8_t)0 };
   }
}


mws_sp<mws_vkb> mws_vkb::gi()
{
   if (!inst)
   {
      //if (mws_debug_enabled) { mws_dbg::set_flags(mws_dbg::app_touch); }
      inst = mws_sp<mws_vkb>(new mws_vkb());
      inst->setup();
   }

   return inst;
}

mws_key_types mws_vkb::apply_key_modifiers(mws_key_types i_key_id) const
{
   uint32_t idx = i_key_id;

   if (active_vkb && idx < active_vkb->get_key_vect().size())
   {
      glm::ivec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());

      if (active_vkb->get_dimensions() == scr_dim)
      {
         mws_key_types key_id = active_vkb->get_key_at(i_key_id);

         return key_id;
      }
   }

   return mws_key_invalid;
}

void mws_vkb::receive(mws_sp<mws_dp> i_dp)
{
   glm::ivec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());
   bool valid_state = active_vkb && (active_vkb->get_dimensions() == scr_dim);

   if (i_dp->is_type(mws_ptr_evt::ptr_evt_type) && valid_state)
   {
      mws_sp<mws_ptr_evt> pe = mws_ptr_evt::as_pointer_evt(i_dp);
      mws_sp<mws_ptr_evt> forwarded_ptr = (ta) ? active_vkb->on_receive(pe, ta) : nullptr;

      // check if we need to close the keyboard
      if (!forwarded_ptr && pe->type == mws_ptr_evt::touch_ended)
      {
         for (uint32_t k = 0; k < pe->touch_count; k++)
         {
            auto& pt = pe->points[k];

            if (pt.is_changed)
            {
               auto ret = active_vkb->vk->get_kernel_idx_at(pt.x, pt.y);
               mws_key_types key_id = active_vkb->get_key_at(ret.idx);

               if (ta && ta->is_action_key(key_id))
               {
                  done();
               }
            }
         }
      }
   }
   else if (i_dp->is_type(mws_key_evt::key_evt_type) && ta)
   {
      ta->receive(i_dp);
   }
}

void mws_vkb::update_state()
{
   if (upcoming_loading_wait())
   {
      std::function<void()> op_load_vkb = [this]()
      {
         nwi_inex();
         active_vkb->start_anim();
         active_vkb->on_update_state();
      };

      get_mod()->enq_op_on_next_frame_start(op_load_vkb);
      get_mod()->enq_op_on_crt_frame_end(get_waiting_msg_op());
   }
   else
   {
      get_active_vkb()->on_update_state();
   }
}

void mws_vkb::on_resize()
{
   uint32_t w = mws::screen::get_width();
   uint32_t h = mws::screen::get_height();

   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = (float)w;
   mws_r.h = (float)h;
   size_changed = true;
}

void mws_vkb::set_target(mws_sp<mws_text_area> i_ta)
{
   ta = i_ta;

   if (upcoming_loading_wait())
   {
      std::function<void()> op_load_vkb = [this]()
      {
         nwi_inex();
         active_vkb->start_anim();
      };

      get_mod()->enq_op_on_next_frame_start(op_load_vkb);
      get_mod()->enq_op_on_crt_frame_end(get_waiting_msg_op());
   }
   else
   {
      get_active_vkb()->start_anim();
   }
}

mws_sp<mws_font> mws_vkb::get_font()
{
   return get_active_vkb()->get_font();
}

void mws_vkb::set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt)
{
   get_active_vkb()->set_font(i_letter_fnt, i_word_fnt);
}

mws_sp<mws_vkb_file_store> mws_vkb::get_file_store() const
{
   return file_store;
}

void mws_vkb::set_file_store(mws_sp<mws_vkb_file_store> i_store)
{
   file_store = i_store;
}

std::vector<mws_sp<gfx_tex>> mws_vkb::get_tex_list()
{
   return get_active_vkb()->get_tex_list();
}

mws_vkb::mws_vkb() { visible = false; }

void mws_vkb::setup()
{
   mws_virtual_keyboard::setup();
   position = glm::vec3(position().x, position().y, 1.f);
   on_resize();
}

void mws_vkb::update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx)
{
   // cursor pos for the pressed key is available only in the next frame after pressing the key in vkb
   // to avoid flickering, pressed key's position needs to be updated in gfx_scene::update()
   // it's too late to do it in mws_obj::update_state(), as it runs after gfx_scene::update(), so we do it here
   if (active_vkb && active_vkb->pressed_key_ker_idx >= 0)
   {
      active_vkb->show_pressed_key(ta, active_vkb->pressed_key_ker_idx);
      active_vkb->pressed_key_ker_idx = -1;
   }

   gfx_node::update_recursive(i_global_tf_mx, i_update_global_mx);
}

void mws_vkb::done()
{
   get_active_vkb()->done();
   visible = false;
   ta->do_action();
   ta = nullptr;
}

void mws_vkb::load(bool i_blocking_load)
{
   if (i_blocking_load)
   {
      nwi_inex();
      active_vkb->start_anim();
   }
   else
   {
      if (upcoming_loading_wait())
      {
         std::function<void()> op_load_vkb = [this]()
         {
            nwi_inex();
            active_vkb->start_anim();
         };

         get_mod()->enq_op_on_next_frame_start(op_load_vkb);
         get_mod()->enq_op_on_crt_frame_end(get_waiting_msg_op());
      }
   }
}

bool mws_vkb::upcoming_loading_wait()
{
   if (!active_vkb)
   {
      return true;
   }

   glm::ivec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());

   // landscape
   if (scr_dim.x > scr_dim.y)
   {
      if (!vkb_landscape)
      {
         return true;
      }

      glm::ivec2 vkb_dim = vkb_landscape->get_dimensions();

      if (vkb_dim != scr_dim)
      {
         return true;
      }
   }
   // portrait
   else
   {
      if (!vkb_portrait)
      {
         return true;
      }

      glm::ivec2 vkb_dim = vkb_portrait->get_dimensions();

      if (vkb_dim != scr_dim)
      {
         return true;
      }
   }

   return false;
}

std::function<void()> mws_vkb::get_waiting_msg_op()
{
   if (!show_waiting_msg_op)
   {
      mws_wp<mws_camera> cam = get_mod()->mws_cam;

      show_waiting_msg_op = [cam]()
      {
         mws_sp<mws_camera> camera = cam.lock();
         gfx_rt::clear_buffers();

         if (camera)
         {
            std::string text = "Loading keyboard\nPlease wait...";
            glm::vec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());
            float vert_size = std::min(scr_dim.x, scr_dim.y);
            mws_sp<mws_font> global_font = mws_font_db::inst()->get_global_font();
            mws_px font_height(vert_size / 20.f, mws_dim::e_vertical);
            mws_sp<mws_font> font = mws_font::nwi(global_font, font_height);
            font->set_color(gfx_color::colors::white);
            glm::vec2 text_dim = font->get_text_dim(text);
            glm::vec2 pos((scr_dim.x - text_dim.x) / 2.f, (scr_dim.y - text_dim.y) / 2.f);

            camera->drawText(text, pos.x, pos.y, font);
            camera->update_camera_state();
         }
      };
   }

   return show_waiting_msg_op;
}

mws_sp<mws_vkb_impl> mws_vkb::get_active_vkb()
{
   uint32_t w = mws::screen::get_width();
   uint32_t h = mws::screen::get_height();

   if (w > h)
   {
      if (active_vkb != vkb_landscape)
      {
         // hide current keyboard as we need to show a new one
         active_vkb->release_all_keys(true);
         active_vkb->visible = false;
         active_vkb = vkb_landscape;
         active_vkb->visible = true;
         active_vkb->start_anim();
      }
   }
   else
   {
      if (active_vkb != vkb_portrait)
      {
         // hide current keyboard as we need to show a new one
         active_vkb->release_all_keys(true);
         active_vkb->visible = false;
         active_vkb = vkb_portrait;
         active_vkb->visible = true;
         active_vkb->start_anim();
      }
   }

   return active_vkb;
}

void mws_vkb::nwi_inex()
{
   // if active_vkb is null, load a vkb into it and also assign it to the landscape/portrait references
   if (!active_vkb)
   {
      uint32_t w = mws::screen::get_width();
      uint32_t h = mws::screen::get_height();

      size_changed = false;
      vkb_file_store = active_vkb = nwi_vkb();

      (w > h) ? vkb_landscape = active_vkb : vkb_portrait = active_vkb;
   }
   // if current active_vkb is not null, but the size has changed, we need to do some (elaborate) checking..
   else if (size_changed)
   {
      uint32_t w = mws::screen::get_width();
      uint32_t h = mws::screen::get_height();
      mws_sp<mws_vkb_impl> inst;

      size_changed = false;
      inst = (w > h) ? vkb_landscape : vkb_portrait;

      // if inst is not null, then we already have the landscape or portrait instance we need, but we have to check if it needs to be modified
      if (inst)
      {
         inst->on_resize(w, h);

         if (inst != active_vkb)
         {
            // hide current keyboard as we need to show a new one
            active_vkb->release_all_keys(true);
            active_vkb->visible = false;
            inst->visible = true;
         }
      }
      // if inst is null, we need to load a vkb into it and also assign it to the landscape/portrait references
      else
      {
         inst = nwi_vkb();
         (w > h) ? vkb_landscape = inst : vkb_portrait = inst;
         // hide current keyboard as we need to show a new one
         active_vkb->release_all_keys(true);
         active_vkb->visible = false;
      }

      (w > h) ? vkb_landscape = inst : vkb_portrait = inst;
      active_vkb = inst;
   }
}

mws_sp<mws_vkb_impl> mws_vkb::nwi_vkb()
{
   uint32_t w = mws::screen::get_width();
   uint32_t h = mws::screen::get_height();
   mws_sp<mws_vkb_impl> inst = std::make_shared<mws_vkb_impl>(mws_vrn_obj_types::nexus_pairs | mws_vrn_obj_types::cell_borders);

   inst->file_store = get_file_store();
   attach(inst);
   inst->setup();
   inst->build_cell_border_tex();
   inst->set_on_top();
   inst->on_resize(w, h);

   return inst;
}
