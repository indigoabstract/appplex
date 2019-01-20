#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "mws-vkb.hpp"
#include "mws-vkb-diagram.hpp"
#include "mws-vkb-visual.hpp"
#include "../mws-camera.hpp"
#include "com/mws/text-vxo.hpp"
#include "com/mws/mws-font.hpp"
#include "unit.hpp"
#include <kxmd/kxmd.hpp>


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

void mws_vkb::receive(mws_sp<iadp> i_dp)
{
   if (i_dp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<pointer_evt> pe = pointer_evt::as_pointer_evt(i_dp);

      switch (pe->type)
      {
      case pointer_evt::touch_began:
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
         }
         break;
      }

      case pointer_evt::touch_ended:
      {
         break;
      }
      }

      pe->process();
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

void mws_vkb::set_target(mws_sp<mws_text_box> i_tbx)
{
   tbx = i_tbx;

   if (vkb_filename.empty())
   {
      load("vk-00.kxmd");
   }
}

void mws_vkb::load(std::string i_filename)
{
   if (!vk)
   {
      vk = vkb_voronoi_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
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
         { KEY_EXCLAMATION, "!" },
         { KEY_DOUBLE_QUOTE, "\"" },
         { KEY_NUMBER_SIGN, "#" },
         { KEY_DOLLAR_SIGN, "$" },
         { KEY_PERCENT_SIGN, "%" },
         { KEY_AMPERSAND, "&" },
         { KEY_SINGLE_QUOTE, "'" },
         { KEY_LEFT_PARENTHESIS, "(" },
         { KEY_RIGHT_PARENTHESIS, ")" },
         { KEY_ASTERISK, "*" },
         { KEY_PLUS_SIGN, "+" },
         { KEY_COMMA, "," },
         { KEY_MINUS_SIGN, "-" },
         { KEY_PERIOD, "." },
         { KEY_SLASH, "/" },
         { KEY_0, "0" },
         { KEY_1, "1" },
         { KEY_2, "2" },
         { KEY_3, "3" },
         { KEY_4, "4" },
         { KEY_5, "5" },
         { KEY_6, "6" },
         { KEY_7, "7" },
         { KEY_8, "8" },
         { KEY_9, "9" },
         { KEY_COLON, ":" },
         { KEY_SEMICOLON, ";" },
         { KEY_LESS_THAN_SIGN, "<" },
         { KEY_EQUAL_SIGN, "=" },
         { KEY_GREATER_THAN_SIGN, ">" },
         { KEY_QUESTION_MARK, "?" },
         { KEY_AT_SYMBOL, "@" },
         { KEY_A, "A" },
         { KEY_B, "B" },
         { KEY_C, "C" },
         { KEY_D, "D" },
         { KEY_E, "E" },
         { KEY_F, "F" },
         { KEY_G, "G" },
         { KEY_H, "H" },
         { KEY_I, "I" },
         { KEY_J, "J" },
         { KEY_K, "K" },
         { KEY_L, "L" },
         { KEY_M, "M" },
         { KEY_N, "N" },
         { KEY_O, "O" },
         { KEY_P, "P" },
         { KEY_Q, "Q" },
         { KEY_R, "R" },
         { KEY_S, "S" },
         { KEY_T, "T" },
         { KEY_U, "U" },
         { KEY_V, "V" },
         { KEY_W, "W" },
         { KEY_X, "X" },
         { KEY_Y, "Y" },
         { KEY_Z, "Z" },
         { KEY_LEFT_BRACKET, "[" },
         { KEY_BACKSLASH, "\\" },
         { KEY_RIGHT_BRACKET, "]" },
         { KEY_CIRCUMFLEX, "^" },
         { KEY_UNDERSCORE, "_" },
         { KEY_GRAVE_ACCENT, "`" },
         { KEY_LEFT_BRACE, "{" },
         { KEY_VERTICAL_BAR, "|" },
         { KEY_RIGHT_BRACE, "}" },
         { KEY_TILDE_SIGN, "~" },
      };
   }

   vkb_filename = i_filename;
   mws_sp<std::vector<uint8> > res = get_unit()->storage.load_unit_byte_vect(vkb_filename);
   mws_sp<std::string> src(new std::string((const char*)begin_ptr(res), res->size()));
   kxmd_parser parser;

   mws_sp<kxmd_elem> kxmdi = parser.parse(src);
   mws_sp<vkb_voronoi_data> diag_data = vk->diag_data;
   vkb_voronoi_data::vkb_voronoi_geometry_data& gd = diag_data->geom;

   {
      auto kernel_point_keys = kxmd::get_elem("kernel-point-keys", kxmdi);

      key_vect.resize(kernel_point_keys->elem_count());

      for (int k = 0; k < kernel_point_keys->elem_count(); k++)
      {
         const std::string& key_name = kernel_point_keys->vect[k]->val;
         key_vect[k] = get_key_type(key_name);
      }
   }

   // kernel-points
   {
      auto kernel_points = kxmd::get_elem("kernel-points", kxmdi);
      auto& kpv = gd.kernel_points;
      int size = kernel_points->elem_count();

      kpv.resize(size);
      kpv._first_idx = 0;
      kpv._last_idx = size - 1;

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = kernel_points->vect[k];
         auto& kp = kpv[k];

         kp.id = k;
         kp.position.x = std::stof(pos_pair->vect[0]->val);
         kp.position.y = std::stof(pos_pair->vect[1]->val);
      }
   }

   // nexus-points
   {
      auto nexus_points = kxmd::get_elem("nexus-points", kxmdi);
      auto& npv = gd.nexus_points;
      int size = nexus_points->elem_count();

      npv.resize(size);
      npv._first_idx = gd.kernel_points._last_idx + 1;
      npv._last_idx = npv._first_idx + size;

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = nexus_points->vect[k];
         auto& np = npv[k];

         np.id = npv._first_idx + k;
         np.position.x = std::stof(pos_pair->vect[0]->val);
         np.position.y = std::stof(pos_pair->vect[1]->val);
      }
   }

   // nexus-pairs
   {
      auto nexus_pairs = kxmd::get_elem("nexus-pairs", kxmdi);
      auto& npp = gd.nexus_pairs;
      int size = nexus_pairs->elem_count();
      npp.resize(size);

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = nexus_pairs->vect[k];
         auto& np = npp[k];

         np.nexus0_id = std::stoi(pos_pair->vect[0]->val);
         np.nexus1_id = std::stoi(pos_pair->vect[1]->val);
      }
   }

   // cell-indices
   {
      auto cell_indices = kxmd::get_elem("cell-indices", kxmdi);
      auto& cpi = gd.cell_points_ids;
      auto& cpc = gd.cell_point_count;
      int cell_indices_size = cell_indices->elem_count();
      int cell_points_count = 0;

      cpc.resize(cell_indices_size);

      for (int k = 0; k < cell_indices_size; k++)
      {
         auto& triangle_indices = cell_indices->vect[k];
         int vx_count = triangle_indices->vect.size();

         cell_points_count += vx_count;
         cpc[k] = vx_count;
      }

      cpi.resize(cell_points_count);

      for (int k = 0, l = 0; k < cell_indices_size; k++)
      {
         auto& triangle_indices = cell_indices->vect[k];
         size_t triangle_indices_count = triangle_indices->vect.size();

         for (size_t i = 0; i < triangle_indices_count; i++, l++)
         {
            int idx = std::stoi(triangle_indices->vect[i]->val);
            cpi[l].point_id = idx;
         }
      }
   }

   vk->update_geometry();

   {
      auto vd = vk->get_diag_data();
      auto& kp_vect = vd->geom.kernel_points;

      key_font = mws_font::nwi(48.f);
      //key_font->set_color(gfx_color::colors::slate_gray);
      key_font->set_color(gfx_color::colors::white);
      selected_key_font = mws_font::nwi(48.f);
      selected_key_font->set_color(gfx_color::colors::red);

      for (size_t k = 0; k < key_vect.size(); k++)
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
   tbx = nullptr;
}

std::string mws_vkb::get_key_name(key_types i_key_id) const
{
   auto it = key_map.find(i_key_id);

   if (it != key_map.end())
   {
      return it->second;
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

   return KEY_INVALID;
}

#endif
