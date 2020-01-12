#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "mws-text-box.hxx"
#include "mws-mod.hxx"
#include "mws-camera.hxx"
#include "mws-com.hxx"
#include "mws-font.hxx"
#include "font-db.hxx"
#include "text-vxo.hxx"
#include "gfx-quad-2d.hxx"
#include "mws-vkb/mws-vkb.hxx"
#include "min.hxx"


#if defined MOD_VECTOR_FONTS

static const std::string nl_char_size = "n";

mws_sp<mws_text_box> mws_text_box::nwi()
{
   mws_sp<mws_text_box> inst(new mws_text_box());
   inst->setup();
   return inst;
}

void mws_text_box::setup()
{
   mws_page_item::setup();
   font = font_db::inst()->get_global_font();
   //font->set_color(gfx_color::colors::white);
   {
      gfx_cursor = std::make_shared<gfx_node>(nullptr);
      gfx_cursor->visible = false;
      attach(gfx_cursor);

      {
         gfx_cursor_bg = gfx_quad_2d::nwi();
         auto& rvxo = *gfx_cursor_bg;
         rvxo.camera_id_list = { "mws_cam" };
         rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
         rvxo[MP_DEPTH_TEST] = true;
         rvxo[MP_DEPTH_WRITE] = true;
         rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
         rvxo[MP_SCISSOR_ENABLED] = true;
         rvxo["u_v4_color"] = gfx_color::colors::blue.to_vec4();
         rvxo.set_dimensions(1, 1);
         gfx_cursor->attach(gfx_cursor_bg);
      }
      {
         gfx_cursor_middle = gfx_quad_2d::nwi();
         auto& rvxo = *gfx_cursor_middle;
         rvxo.camera_id_list = { "mws_cam" };
         rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
         rvxo[MP_BLENDING] = MV_ALPHA;
         rvxo[MP_DEPTH_TEST] = false;
         rvxo[MP_DEPTH_WRITE] = false;
         //rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
         rvxo[MP_SCISSOR_ENABLED] = true;
         rvxo["u_v4_color"] = gfx_color::colors::red.to_vec4();
         rvxo.set_dimensions(1, 1);
         rvxo.position += glm::vec3(0.f, 0.f, -0.1f);
         gfx_cursor->attach(gfx_cursor_middle);
      }
      //{
      //   gfx_cursor_left = gfx_quad_2d::nwi();
      //   auto& rvxo = *gfx_cursor_left;
      //   rvxo.camera_id_list = { "mws_cam" };
      //   rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
      //   rvxo[MP_DEPTH_TEST] = true;
      //   rvxo[MP_DEPTH_WRITE] = true;
      //   rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      //   rvxo["u_v4_color"] = gfx_color::colors::red.to_vec4();
      //   rvxo.set_dimensions(1, 1);
      //   gfx_cursor->attach(gfx_cursor_left);
      //}
      //{
      //   gfx_cursor_right = gfx_quad_2d::nwi();
      //   auto& rvxo = *gfx_cursor_right;
      //   rvxo.camera_id_list = { "mws_cam" };
      //   rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
      //   rvxo[MP_DEPTH_TEST] = true;
      //   rvxo[MP_DEPTH_WRITE] = true;
      //   rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      //   rvxo["u_v4_color"] = gfx_color::colors::red.to_vec4();
      //   rvxo.set_dimensions(1, 1);
      //   gfx_cursor->attach(gfx_cursor_right);
      //}
   }
   if (mws_dbg::enabled(mws_dbg::pfm_mws))
   {
      debug_bg = gfx_quad_2d::nwi();
      auto& rvxo = *debug_bg;
      rvxo.camera_id_list = { "mws_cam" };
      rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
      rvxo["u_v4_color"] = gfx_color::colors::gray.to_vec4();
      rvxo.set_dimensions(1, 1);
      rvxo.position = glm::vec3(0.f, 0.f, -5.f);
      attach(debug_bg);
   }
}

bool mws_text_box::is_action_key(key_types i_key) const
{
   if (i_key == VKB_DONE)
   {
      return true;
   }

   return false;
}

void mws_text_box::do_action()
{
   if (on_action)
   {
      on_action();
   }

   if (pfm::uses_touchscreen())
   {
      set_focus(false);
   }
}

bool mws_text_box::is_editable() const
{
   return editable;
}

void mws_text_box::set_editable(bool i_is_editable)
{
   if (editable != i_is_editable)
   {
      editable = i_is_editable;
      auto src = new_model();

      if (tx_src)
      {
         std::string text = tx_src->get_text();
         src->set_text(text);
      }

      tx_src = src;
   }
}

const std::string& mws_text_box::get_text() const
{
   return tx_src->get_text();
}

void mws_text_box::set_text(const std::string& i_text)
{
   tx_src = new_model();
   tx_src->set_text(i_text);
   update_text_view();
   select_char_at(glm::vec2(0.f));
}

void mws_text_box::push_back_text(const std::string& i_text)
{
   if (!tx_src)
   {
      set_text(i_text);
   }
   else
   {
      tx_src->push_back(i_text.c_str(), i_text.length());
      sync_view();
   }
}

void mws_text_box::push_front_text(const std::string& i_text)
{
   if (!tx_src)
   {
      set_text(i_text);
   }
   else
   {
      tx_src->push_front(i_text.c_str(), i_text.length());
      sync_view();
   }
}

void mws_text_box::insert_at_cursor(const std::string& i_text)
{
   tx_src->insert_at_cursor(i_text);
   sync_view_to_cursor_pos();
}

void mws_text_box::delete_at_cursor(int32 i_count)
{
   tx_src->delete_at_cursor(i_count);
   sync_view_to_cursor_pos();
}

void mws_text_box::scroll_text(const glm::vec2& i_off, bool i_snap_to_grid)
{
   if (view)
   {
      view->scroll_text(i_off, i_snap_to_grid);
      update_cursor();
   }
}

void mws_text_box::scroll_to_end(dir_types i_direction)
{
   if (i_direction == dir_types::DIR_DOWN_LEFT)
   {
      uint32 tx_src_line_count = tx_src->get_line_count();
      float font_height = font->get_height();
      float total_height = tx_src_line_count * font_height;

      scroll_text(glm::vec2(-total_height, total_height));
   }
}

void mws_text_box::set_position(const glm::vec2& i_pos)
{
   mws_r = mws_rect(i_pos.x, i_pos.y, mws_r.w, mws_r.h);
   update_text_view();

   if (mws_dbg::enabled(mws_dbg::pfm_mws))
   {
      debug_bg->set_translation(i_pos);
   }
}

void mws_text_box::set_dimension(const glm::vec2& i_dim)
{
   mws_r = mws_rect(mws_r.x, mws_r.y, i_dim.x, i_dim.y);
   update_text_view();

   if (mws_dbg::enabled(mws_dbg::pfm_mws))
   {
      debug_bg->set_scale(i_dim);
   }
}

void mws_text_box::set_font(mws_sp<mws_font> i_font)
{
   font = i_font;
   tx_src->set_font(font);
   update_text_view();
}

void mws_text_box::select_char_at(const glm::vec2& i_pos)
{
   if (view)
   {
      view->select_char_at(i_pos);
      update_cursor();
   }
}

void mws_text_box::update_state()
{
   if (ks.is_active())
   {
      glm::vec2 scroll_pos = ks.update();
      scroll_text(scroll_pos);
      update_gfx_cursor();
   }

   if (gfx_cursor->visible)
   {
      float iv = gfx_cursor_slider.get_value();
      //mws_rect cursor_l = get_cursor_rect(e_left_cursor);
      //mws_rect cursor_r = get_cursor_rect(e_right_cursor);
      //gfx_cursor_left->set_translation(glm::vec2(glm::mix(cursor_l.x, cursor_r.x - gfx_cursor_left->get_scale().x, iv), cursor_l.y));
      //gfx_cursor_right->set_translation(glm::vec2(glm::mix(cursor_r.x + cursor_r.w - gfx_cursor_right->get_scale().x, cursor_r.x, iv), cursor_l.y));
      glm::vec4 color = gfx_color::colors::red.to_vec4();
      color.a = glm::round(iv);
      (*gfx_cursor_middle)["u_v4_color"] = color;
      gfx_cursor_slider.update();
   }
}

void mws_text_box::update_view(mws_sp<mws_camera> g)
{
   //g->setColor(0x7fff0000);
   //g->fillRect(mws_r.x, mws_r.y, mws_r.w, mws_r.h);
   //g->setColor(0xffffffff);
   //g->drawRect(mws_r.x, mws_r.y, mws_r.w, mws_r.h);

   //if(has_focus())
   //{
   //   if (cursor_left && cursor_right)
   //   {
   //      mws_rect cursor_rect(cursor_left->x + mws_r.x, cursor_left->y + mws_r.y, cursor_right->x - cursor_left->x + cursor_right->w, std::max(cursor_left->h, cursor_right->h));
   //      g->drawRect(cursor_rect.x, cursor_rect.y, cursor_rect.w, cursor_rect.h);
   //   }
   //   else if (cursor_left || cursor_right)
   //   {
   //      if (cursor_left)
   //      {
   //         g->drawRect(cursor_left->x + mws_r.x, cursor_left->y + mws_r.y, cursor_left->w, cursor_left->h);
   //      }
   //      else if (cursor_right)
   //      {
   //         g->drawRect(cursor_right->x + mws_r.x, cursor_right->y + mws_r.y, cursor_right->w, cursor_right->h);
   //      }
   //   }
   //}
}

void mws_text_box::on_focus_changed(bool i_has_focus)
{
   if (i_has_focus)
   {
      if (on_gained_focus)
      {
         on_gained_focus();
      }

      if (is_editable())
      {
         update_cursor();
         gfx_cursor->visible = true;
         gfx_cursor_slider.start(0.25f);
      }
   }
   else
   {
      gfx_cursor->visible = false;
      gfx_cursor_slider.stop();

      if (on_lost_focus)
      {
         on_lost_focus();
      }
   }
}

void mws_text_box::receive(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_processed())
   {
      return;
   }

   if (i_dp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
   {
      handle_pointer_evt(mws_ptr_evt::as_pointer_evt(i_dp));
   }
   else if (i_dp->is_type(mws_key_evt::KEYEVT_EVT_TYPE))
   {
      handle_key_evt(mws_key_evt::as_key_evt(i_dp));
   }
}

mws_rect mws_text_box::get_cursor_rect(cursor_types i_cursor_type, bool i_absolute_pos)
{
   mws_rect cursor;

   if (view)
   {
      cursor = view->get_cursor_rect(i_cursor_type);

      if (i_absolute_pos)
      {
         cursor.x += mws_r.x;
         cursor.y += mws_r.y;
      }
   }

   return cursor;
}

void mws_text_box::update_cursor()
{
   if (view)
   {
      view->update_cursor();
      update_gfx_cursor();
   }
}

void mws_text_box::update_gfx_cursor()
{
   mws_rect cursor_bg = get_cursor_rect(e_left_right_cursor);
   mws_rect cursor_m = get_cursor_rect(e_middle_cursor);
   //mws_rect cursor_l = get_cursor_rect(e_left_cursor);
   //mws_rect cursor_r = get_cursor_rect(e_right_cursor);
   //float width = cursor_r.h / 5.f;
   //gfx_cursor_left->set_scale(width, cursor_l.h);
   //gfx_cursor_right->set_scale(width, cursor_r.h);
   gfx_cursor_bg->set_translation(cursor_bg.x, cursor_bg.y);
   gfx_cursor_bg->set_scale(cursor_bg.w, cursor_bg.h);
   gfx_cursor_middle->set_translation(cursor_m.x + cursor_m.w / 4.f, cursor_m.y);
   gfx_cursor_middle->set_scale(cursor_m.w / 2.f, cursor_m.h);
}

void mws_text_box::text_view::setup()
{
   tx_vxo = text_vxo::nwi();
   tx_vxo->camera_id_list = { "mws_cam" };
   (*tx_vxo)[MP_SCISSOR_ENABLED] = true;
   attach(tx_vxo);
   left_char_rect = std::make_shared<mws_rect>();
   right_char_rect = std::make_shared<mws_rect>();
}

void mws_text_box::text_view::select_char_at(const glm::vec2& i_pos)
{
   if (tx_lines.empty())
   {
      return;
   }

   float text_row_remainder = tx_offset.y;

   cursor_grid_pos.y = uint32((i_pos.y + text_row_remainder) / font->get_height());
   cursor_grid_pos.x = 0;

   if (mws_dbg::enabled(mws_dbg::pfm_mws))
   {
      mws_println("select_char_at:cursor_grid_pos [ %d, %d ]", cursor_grid_pos.x, cursor_grid_pos.y);
   }

   *left_char_rect = *right_char_rect = mws_rect();
   left_char_rect->h = right_char_rect->h = font->get_height();

   if (cursor_grid_pos.y >= (int32)tx_lines.size())
   {
      cursor_grid_pos.y = std::max(0, int(tx_lines.size() - 1));
      //mws_println("select_char_at1 cursor_grid_pos.y [ %d ]", cursor_grid_pos.y);
      cursor_grid_pos.x = tx_lines[cursor_grid_pos.y].size();
      uint32 line_index = get_lines_from_the_top_count() + cursor_grid_pos.y;
      uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
      glm::uvec2 cursor_coord = tx_src->get_cursor_coord();
      tx_src->set_cursor_pos(cursor_pos + cursor_grid_pos.x);
      //glm::uvec2 new_cursor_coord = tx_src->get_cursor_coord();
      //mws_println("c0 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]",
      //   line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);

      return;
   }

   std::string& text = tx_lines[cursor_grid_pos.y];

   if (text.empty())
   {
      cursor_grid_pos.x = 0;

      if (get_text_box()->is_editable())
      {
         uint32 line_index = get_lines_from_the_top_count() + cursor_grid_pos.y;
         uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
         glm::uvec2 cursor_coord = tx_src->get_cursor_coord();
         tx_src->set_cursor_pos(cursor_pos + cursor_grid_pos.x);
         //glm::uvec2 new_cursor_coord = tx_src->get_cursor_coord();
         //mws_println("c1 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]",
         //   line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);
      }

      return;
   }

   auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
   float x_off = -tx_offset.x;
   uint32 text_length = (text.back() != '\n') ? text.length() : text.length() - 1;
   uint32 k = 0;

   if (glyphs.size() < text_length)
   {
      return;
   }

   for (k = 0; k < text_length; k++)
   {
      font_glyph glyph = glyphs[k];

      if (glyph.is_valid())
      {
         float span = get_span(glyph, text, k);
         float delta = x_off + span - i_pos.x;

         if (delta > 0.f)
         {
            // cursor falls to the left of this character
            if (delta > span / 2.f)
            {
               if (k == 0)
               {
                  cursor_grid_pos.x = k;
               }
               else
               {
                  cursor_grid_pos.x = k;
               }
            }
            // cursor falls to the right of this character
            else
            {
               cursor_grid_pos.x = k + 1;
            }
            break;
         }
         else
         {
            //left_char_rect->x = x_off;
            x_off += span;
         }
      }
   }

   if (k >= text_length)
   {
      cursor_grid_pos.x = text_length;
   }

   if (get_text_box()->is_editable())
   {
      uint32 line_index = get_lines_from_the_top_count() + cursor_grid_pos.y;
      uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
      glm::uvec2 cursor_coord = tx_src->get_cursor_coord();
      tx_src->set_cursor_pos(cursor_pos + cursor_grid_pos.x);
      //glm::uvec2 new_cursor_coord = tx_src->get_cursor_coord();
      //mws_println("c2 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]",
      //   line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);
   }
}

void mws_text_box::text_view::scroll_text(const glm::vec2& i_offset, bool i_snap_to_grid)
{
   uint32 tx_src_line_count = tx_src->get_line_count();
   uint32 max_actual_lines = glm::min(max_lines_allowed_by_height, tx_src_line_count);
   float font_height = font->get_height();
   bool changed_sign = false;

   tx_offset -= i_offset;
   tx_offset.x = glm::max(0.f, tx_offset.x);

   if (tx_offset.y < 0.f)
   {
      float fact = glm::ceil(glm::abs(tx_offset.y / font_height));

      tx_offset.y += fact * font_height;
      changed_sign = true;
      set_lines_from_the_top_count(get_lines_from_the_top_count() - (int32)fact);
   }

   // disable scrolling upwards when at the page top
   if ((i_offset.y > 0.f) && (get_lines_from_the_top_count() < 0))
   {
      tx_offset.y = 0.f;
   }
   // disable scrolling downwards when at the page bottom
   else if ((i_offset.y < 0.f) && (get_lines_from_the_top_count() >= int32(tx_src_line_count - max_actual_lines - 1)))
   {
      tx_offset.y = 0.f;
   }

   // normal vertical scrolling
   if (tx_offset.y != 0.f || changed_sign)
   {
      float lines = tx_offset.y / font_height;
      mws_assert(lines >= 0.f);

      if (i_snap_to_grid)
      {
         lines = glm::round(lines);
         tx_offset.y = 0.f;
      }

      float lines_scrolled = glm::floor(lines);

      if (((lines > 1.f) && (lines_scrolled != 0.f)) || changed_sign)
      {
         set_lines_from_the_top_count(get_lines_from_the_top_count() + int32(lines_scrolled));

         if (tx_offset.y != 0.f)
         {
            tx_offset.y = tx_offset.y - lines_scrolled * font_height;
         }

         clamp_lines_from_the_top_count(0, tx_src_line_count - max_actual_lines);
         sync_view();
      }
   }

   sync_position();
}

void mws_text_box::text_view::sync_view()
{
   uint32 tx_src_line_count = tx_src->get_line_count();
   uint32 max_actual_lines = glm::min(max_lines_allowed_by_height, tx_src_line_count);
   float font_height = font->get_height();
   uint32 src_line_count = tx_src->get_line_count();
   uint32 actual_line_count = glm::min(max_actual_lines + extra_lines_count, src_line_count);
   tx_lines = tx_src->get_lines_at(get_lines_from_the_top_count(), actual_line_count);
   uint32 line_count = tx_lines.size();
   max_line_width = 0.f;
   tx_vxo->clear_text();

   for (uint32 k = 0; k < line_count; k++)
   {
      float text_width = font->get_text_width(tx_lines[k]);
      max_line_width = glm::max(max_line_width, text_width);
      tx_vxo->add_text(tx_lines[k], glm::vec2(0, k * font_height), font);
   }

   glm::uvec2 global_cursor_pos = tx_src->get_cursor_coord();
   cursor_grid_pos = glm::uvec2(global_cursor_pos.x, global_cursor_pos.y - get_lines_from_the_top_count());

   if (mws_dbg::enabled(mws_dbg::pfm_mws))
   {
      mws_println("scroll_text::cursor_grid_pos [ %d, %d %d %d ]", cursor_grid_pos.x, cursor_grid_pos.y, global_cursor_pos.y, get_lines_from_the_top_count());
   }
}

void mws_text_box::text_view::sync_view_to_cursor_pos()
{
   uint32 tx_src_line_count = tx_src->get_line_count();
   uint32 max_actual_lines = glm::min(max_lines_allowed_by_height, tx_src_line_count);
   glm::uvec2 global_cursor_pos = tx_src->get_cursor_coord();

   if ((int32)global_cursor_pos.y < get_lines_from_the_top_count())
   {
      set_lines_from_the_top_count(global_cursor_pos.y);
      tx_offset = glm::vec2(0.f);
   }
   else if ((int32)global_cursor_pos.y > (get_lines_from_the_top_count() + max_actual_lines))
   {
      set_lines_from_the_top_count(global_cursor_pos.y - max_actual_lines);
   }

   clamp_lines_from_the_top_count(0, tx_src_line_count - max_actual_lines);
   sync_view();
   sync_position();
}

void mws_text_box::text_view::sync_position()
{
   float delta_w = glm::max(0.f, max_line_width - mws_r.w);

   tx_offset.x = glm::min(delta_w, tx_offset.x);
   tx_vxo->position = glm::vec3(glm::vec2(mws_r.x, mws_r.y) - tx_offset, 0.f);
}

float mws_text_box::text_view::get_span(const font_glyph& i_glyph, std::string& i_text, int i_idx) const
{
   float span = 0.f;
   char ch = i_text[i_idx];

   if (ch == '\t')
   {
      span = font->get_text_width("   ");
   }
   else
   {
      float kerning = (i_idx > 0) ? i_glyph.get_kerning(i_text[i_idx - 1]) : 0.0f;
      span = kerning + i_glyph.get_advance_x();
   }

   return span;
}

void mws_text_box::text_view::update_cursor()
{
   if (tx_lines.empty() || (cursor_grid_pos.y < 0 || cursor_grid_pos.y >= (int32)tx_lines.size()) || cursor_grid_pos.x < 0)
   {
      cursor_left = cursor_right = nullptr;
      return;
   }

   std::string& text = tx_lines[cursor_grid_pos.y];
   float text_row_remainder = tx_offset.y;

   // if we have at least one char on this line( even if it's a single '\n' )
   if (!text.empty())
   {
      auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
      bool is_new_line_terminated = (text.back() == '\n');
      uint32 text_length = (is_new_line_terminated) ? text.length() - 1 : text.length();
      float x_off = -tx_offset.x;
      mws_assert(cursor_grid_pos.x <= (int32)text_length);

      if (glyphs.empty())
      {
         return;
      }

      // special case for cursor position before first char
      if (cursor_grid_pos.x == 0)
      {
         float span = get_span(glyphs[0], text, 0);
         cursor_left = nullptr;
         right_char_rect->x = x_off;
         right_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
         right_char_rect->w = span;
         cursor_right = right_char_rect;
      }
      // special case for cursor position after last char
      else if (cursor_grid_pos.x == text_length)
      {
         // either last char is a new line and the cursor position is just before it
         // or, there's no new line at the end, so the cursor position is after the last char
         // we treat both cases the same
         for (uint32 k = 0; (int32)k < cursor_grid_pos.x - 1; k++)
         {
            font_glyph glyph = glyphs[k];
            float span = get_span(glyph, text, k);
            x_off += span;
         }

         left_char_rect->x = x_off;
         left_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
         left_char_rect->w = get_span(glyphs[cursor_grid_pos.x - 1], text, cursor_grid_pos.x - 1);
         cursor_left = left_char_rect;

         right_char_rect->x = x_off + left_char_rect->w;
         right_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
         right_char_rect->w = font->get_text_width(nl_char_size);
         cursor_right = right_char_rect;
      }
      // cursor position is between the first and last char
      else
      {
         for (uint32 k = 0; (int32)k < cursor_grid_pos.x - 1; k++)
         {
            font_glyph glyph = glyphs[k];
            float span = get_span(glyph, text, k);
            x_off += span;
         }

         left_char_rect->x = x_off;
         left_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
         left_char_rect->w = get_span(glyphs[cursor_grid_pos.x - 1], text, cursor_grid_pos.x - 1);
         cursor_left = left_char_rect;

         right_char_rect->x = x_off + left_char_rect->w;
         right_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
         right_char_rect->w = get_span(glyphs[cursor_grid_pos.x], text, cursor_grid_pos.x);
         cursor_right = right_char_rect;
      }
   }
   // empty line
   else
   {
      cursor_left = left_char_rect;
      left_char_rect->x = 0.f;
      left_char_rect->y = cursor_grid_pos.y * font->get_height() - text_row_remainder;
      left_char_rect->w = font->get_text_width(nl_char_size);
      cursor_right = nullptr;
   }
}

mws_rect mws_text_box::text_view::get_cursor_rect(cursor_types i_cursor_type)
{
   mws_rect cursor;

   switch (i_cursor_type)
   {
   case mws_text_box::e_left_cursor:
      if (cursor_left)
      {
         cursor = *cursor_left;
      }
      else if (cursor_right)
      {
         cursor = *cursor_right;
         cursor.w = 0;
      }
      break;

   case mws_text_box::e_right_cursor:
      if (cursor_right)
      {
         cursor = *cursor_right;
      }
      else if (cursor_left)
      {
         cursor = *cursor_left;
         cursor.w = 0;
      }
      break;

   case mws_text_box::e_middle_vertical_cursor:
      if (cursor_right)
      {
         cursor = *cursor_right;
         cursor.w = 0;
      }
      else if (cursor_left)
      {
         cursor = *cursor_left;
         cursor.x += cursor.w;
         cursor.w = 0;
      }
      break;

   case mws_text_box::e_middle_cursor:
      if (cursor_right)
      {
         cursor = *cursor_right;
         cursor.x -= cursor.w / 2.f;
      }
      else if (cursor_left)
      {
         cursor = *cursor_left;
         cursor.x += cursor.w / 2.f;
      }
      break;

   case mws_text_box::e_left_right_cursor:
      if (cursor_left)
      {
         cursor = *cursor_left;

         if (cursor_right)
         {
            cursor.w += cursor_right->w;
         }
      }
      else if (cursor_right)
      {
         cursor = *cursor_right;
      }
      break;
   }

   return cursor;
}

int32 mws_text_box::text_view::get_lines_from_the_top_count()
{
   return lines_from_the_top_count;
}

void mws_text_box::text_view::set_lines_from_the_top_count(int32 i_lines_from_the_top_count)
{
   lines_from_the_top_count = i_lines_from_the_top_count;
}

void mws_text_box::text_view::clamp_lines_from_the_top_count(int32 i_inf_lim, int32 i_sup_lim)
{
   if (get_lines_from_the_top_count() < i_inf_lim)
   {
      set_lines_from_the_top_count(i_inf_lim);
   }
   else if (get_lines_from_the_top_count() > i_sup_lim)
   {
      set_lines_from_the_top_count(i_sup_lim);
   }
}

mws_text_box::mws_text_box()
{
   //if (mws_debug_enabled) { mws_dbg::set_flags(mws_dbg::pfm_mws); }
}

void mws_text_box::on_attach()
{
   mws_text_area::on_attach();
   view = std::make_shared<text_view>();
   view->setup();
   attach(view);
   update_text_view();
}

mws_sp<mws_text_area_model> mws_text_box::new_model()
{
   if (editable)
   {
      return std::make_shared<mws_text_area_model_rw>();
   }

   return std::make_shared<mws_text_area_model_ro>();
}

void mws_text_box::sync_view()
{
   if (view)
   {
      view->sync_view();
      update_cursor();
   }
}

void mws_text_box::sync_view_to_cursor_pos()
{
   if (view)
   {
      view->sync_view_to_cursor_pos();
      update_cursor();
   }
}

void mws_text_box::update_text_view()
{
   if (view)
   {
      glm::vec4 scissor_rect = glm::vec4(glm::vec2(mws_r.x, mws_r.y - 1.f), glm::vec2(mws_r.w, mws_r.h + 1));

      view->tx_src = tx_src;
      view->font = font;
      view->position = position;
      view->set_rect(mws_r);
      view->max_lines_allowed_by_height = int(mws_r.h / font->get_height());
      (*gfx_cursor_bg)[MP_SCISSOR_AREA] = scissor_rect;
      (*gfx_cursor_middle)[MP_SCISSOR_AREA] = scissor_rect;
      (*view->tx_vxo)[MP_SCISSOR_AREA] = scissor_rect;
      view->sync_view_to_cursor_pos();
      update_cursor();

      if (mws_dbg::enabled(mws_dbg::pfm_mws))
      {
         mws_println("view->max_lines_allowed_by_height [ %d ] rect[ %f, %f, %f, %f]",
            view->max_lines_allowed_by_height, mws_r.x, mws_r.y, mws_r.w, mws_r.h);
      }
   }
}

void mws_text_box::handle_pointer_evt(mws_sp<mws_ptr_evt> i_pe)
{
   bool hit = is_inside_box(i_pe->points[0].x, i_pe->points[0].y, mws_r.x, mws_r.y, mws_r.w, mws_r.h);

   if (!hit)
   {
      return;
   }
   //mws_print("hit at [%f, %f]\n", ts->points[0].x, ts->points[0].y);

   float x = i_pe->points[0].x - mws_r.x;
   float y = i_pe->points[0].y - mws_r.y;
   bool dragging_detected = dragging_det.detect_helper(i_pe);

   if (dragging_detected)
   {
      if (dragging_det.is_finished())
      {
         uint32 delta_t = i_pe->time - dragging_det.last_move_pos_time;

         if (delta_t < 150)
         {
            ks.start_slowdown();
         }
         else
         {
            ks.reset();
         }
      }
      else
      {
         ks.begin(i_pe->points[0].x, i_pe->points[0].y);
      }

      scroll_text(dragging_det.drag_diff);
      process(i_pe);
   }

   switch (i_pe->type)
   {
   case mws_ptr_evt::touch_began:
   {
      if (is_editable())
      {
         if (pfm::uses_touchscreen())
         {
            auto inst = static_pointer_cast<mws_text_area>(get_instance());
            get_mws_root()->show_keyboard(inst);
         }

         select_char_at(glm::vec2(x, y));
         //mws_print("touch [%f, %f]\n", i_pe->points[0].x, i_pe->points[0].y);
      }

      ks.grab(x, y);

      if (on_click) { on_click(); }
      if (!i_pe->is_processed()) { process(i_pe); }
      break;
   }

   case mws_ptr_evt::touch_ended:
   {
      if (!i_pe->is_processed()) { process(i_pe); }
      break;
   }
   }
}

void mws_text_box::handle_key_evt(mws_sp<mws_key_evt> i_ke)
{
   if (i_ke->get_type() != mws_key_evt::KE_RELEASED)
   {
      key_types key = i_ke->get_key();

      if (is_action_key(key))
      {
         if (on_action)
         {
            process(i_ke);
            on_action();
         }

         return;
      }

      if (mws_key_evt::is_ascii(key))
      {
         if (editable)
         {
            std::string key_str(1, (char)key);
            insert_at_cursor(key_str);
            process(i_ke);
         }
      }
      else
      {
         bool is_processed = true;
         float off = 51.175f;

         if (i_ke->get_type() == mws_key_evt::KE_PRESSED)
         {
            off = 21.175f;
         }

         if (editable)
         {
            switch (key)
            {
            case KEY_LEFT:
               tx_src->advance_cursor(dir_types::DIR_LEFT);   scroll_text(glm::vec2(off, 0));
               break;

            case KEY_UP:
               tx_src->advance_cursor(dir_types::DIR_UP);     scroll_text(glm::vec2(0, off));
               break;

            case KEY_RIGHT:
               tx_src->advance_cursor(dir_types::DIR_RIGHT); scroll_text(glm::vec2(-off, 0));
               break;

            case KEY_DOWN:
               tx_src->advance_cursor(dir_types::DIR_DOWN);  scroll_text(glm::vec2(0, -off));
               break;

            case KEY_DELETE:
               delete_at_cursor(1);
               break;

            case KEY_BACKSPACE:
               delete_at_cursor(-1);
               break;

            case KEY_TAB:
               insert_at_cursor("   ");
               break;

            case KEY_ENTER:
               insert_at_cursor("\n");
               break;

            default:
               is_processed = false;
            }
         }
         else
         {
            switch (key)
            {
            case KEY_LEFT:
               scroll_text(glm::vec2(off, 0));
               break;

            case KEY_UP:
               scroll_text(glm::vec2(0, off));
               break;

            case KEY_RIGHT:
               scroll_text(glm::vec2(-off, 0));
               break;

            case KEY_DOWN:
               scroll_text(glm::vec2(0, -off));
               break;

            default:
               is_processed = false;
            }
         }

         if (is_processed)
         {
            process(i_ke);
         }
      }

      if (on_key_pressed && i_ke->is_processed())
      {
         on_key_pressed(key);
      }
   }
}

float mws_text_box::get_span(const font_glyph& i_glyph, std::string& i_text, int i_idx) const
{
   float span = 0.f;
   char ch = i_text[i_idx];

   if (ch == '\t')
   {
      span = font->get_text_width("   ");
   }
   else
   {
      float kerning = (i_idx > 0) ? i_glyph.get_kerning(i_text[i_idx - 1]) : 0.0f;
      span = kerning + i_glyph.get_advance_x();
   }

   return span;
}



mws_sp<mws_text_field> mws_text_field::nwi()
{
   mws_sp<mws_text_field> inst(new mws_text_field());
   inst->setup();
   return inst;
}

bool mws_text_field::is_action_key(key_types i_key) const
{
   if (i_key == VKB_ENTER || i_key == VKB_DONE)
   {
      return true;
   }

   return false;
}

void mws_text_field::insert_at_cursor(const std::string& i_text)
{
   std::string text = i_text;
   format_text(text);

   if (!text.empty())
   {
      mws_text_box::insert_at_cursor(text);
   }
}

void mws_text_field::set_text(const std::string& i_text)
{
   std::string text = i_text;
   format_text(text);
   mws_text_box::set_text(text);
}

void mws_text_field::format_text(std::string& i_text)
{
   static const std::string search = "\n";
   static const std::string fmt = "";
   mws_str::replace_all(i_text, search, fmt);
}


mws_text_area_model_ro::mws_text_area_model_ro()
{
   update_line_offsets();
}

bool mws_text_area_model_ro::get_word_wrap() { return word_wrap; }

void mws_text_area_model_ro::set_word_wrap(bool i_word_wrap) { word_wrap = i_word_wrap; }

uint32 mws_text_area_model_ro::get_line_count() { return line_offsets.size() - 1; }

std::string mws_text_area_model_ro::get_line_at(uint32 i_idx, bool i_keep_line_break)
{
   int start_idx = line_offsets[i_idx];
   int len = line_offsets[i_idx + 1] - start_idx;

   if (!i_keep_line_break)
   {
      // if not empty line, discard the new line character at the end
      if (len > 0)
      {
         len--;
      }
   }

   std::string line = text.substr(start_idx, len);

   return line;
}

std::vector<std::string> mws_text_area_model_ro::get_lines_at(uint32 i_idx, uint32 i_line_count, bool i_keep_line_break)
{
   std::vector<std::string> lines;

   for (uint32 k = 0; k < i_line_count; k++)
   {
      lines.push_back(get_line_at(i_idx + k, i_keep_line_break));
   }

   return lines;
}

const std::string& mws_text_area_model_ro::get_text()
{
   return text;
}

void mws_text_area_model_ro::push_back(const char* i_text, uint32 i_length)
{
   std::string new_text(i_text, i_length);
   text += new_text;
   update_back_added_line_offsets(new_text);
}

void mws_text_area_model_ro::push_front(const char* i_text, uint32 i_length)
{
   std::string new_text(i_text, i_length);
   text = new_text + text;
   update_front_added_line_offsets(new_text);
}

void mws_text_area_model_ro::set_text(const std::string& i_text)
{
   set_text(i_text.c_str(), i_text.length());
}

void mws_text_area_model_ro::set_text(const char* i_text, uint32 i_length)
{
   text = std::string(i_text, i_length);
   update_line_offsets();
}

void mws_text_area_model_ro::set_size(uint32 i_width, uint32 i_height) {}

void mws_text_area_model_ro::set_font(mws_sp<mws_font> i_font) {}

int mws_text_area_model_ro::get_char_at_pixel(float i_x, float i_y) { return 0; }

void mws_text_area_model_ro::update_back_added_line_offsets(const std::string& i_new_text)
{
   int len = i_new_text.length();
   int last_offset = line_offsets.back();
   line_offsets.pop_back();

   for (int k = 0; k < len; k++)
   {
      if (i_new_text[k] == '\n')
      {
         int offset = last_offset + k + 1;
         line_offsets.push_back(offset);
      }
   }

   line_offsets.push_back(text.length());
}

void mws_text_area_model_ro::update_front_added_line_offsets(const std::string& i_new_text)
{
   std::vector<uint32> lo;
   int len = i_new_text.length();
   int idx = 1;

   for (uint32 k = 1; k < line_offsets.size(); k++)
   {
      line_offsets[k] += len;
   }

   for (int k = 0; k < len; k++)
   {
      if (i_new_text[k] == '\n')
      {
         line_offsets.insert(line_offsets.begin() + idx, k + 1);
         idx++;
      }
   }

   //update_line_offsets();
}

void mws_text_area_model_ro::update_line_offsets()
{
   int len = text.length();
   line_offsets.clear();
   line_offsets.push_back(0);

   for (int k = 0; k < len; k++)
   {
      if (text[k] == '\n')
      {
         line_offsets.push_back(k + 1);
      }
   }

   line_offsets.push_back(len);
}


mws_text_area_model_rw::mws_text_area_model_rw() {}
bool mws_text_area_model_rw::get_word_wrap() { return false; }
void mws_text_area_model_rw::set_word_wrap(bool i_word_wrap) {}

uint32 mws_text_area_model_rw::get_line_count()
{
   int length = text.length();
   int line_count = 1;

   for (int k = 0; k < length; k++)
   {
      if (text[k] == '\n')
      {
         line_count++;
      }
   }

   return line_count;
}

std::string mws_text_area_model_rw::get_line_at(uint32 i_idx, bool i_keep_line_break)
{
   int length = text.length();
   int line_count = 1;
   int idx = 0;
   int crt_idx = length;
   std::string line;

   for (int k = 0; k < length; k++)
   {
      if (text[k] == '\n')
      {
         if (i_idx == (line_count - 1))
         {
            crt_idx = k + 1;

            break;
         }

         line_count++;
         idx = k + 1;
      }
   }

   if (!i_keep_line_break)
   {
      // if not empty line, discard the new line character at the end
      if (crt_idx - idx > 0)
      {
         if (text[crt_idx - 1] == '\n')
         {
            crt_idx--;
         }
      }
   }

   line = text.substr(idx, crt_idx - idx);

   return line;
}

std::vector<std::string> mws_text_area_model_rw::get_lines_at(uint32 i_idx, uint32 i_line_count, bool i_keep_line_break)
{
   std::vector<std::string> lines;

   for (uint32 k = 0; k < i_line_count; k++)
   {
      lines.push_back(get_line_at(i_idx + k, i_keep_line_break));
   }

   return lines;
}

const std::string& mws_text_area_model_rw::get_text()
{
   return text;
}

void mws_text_area_model_rw::set_text(const std::string& i_text)
{
   text = i_text;
}

void mws_text_area_model_rw::set_text(const char* i_text, uint32 i_length)
{
   text = std::string(i_text, i_length);
}

void mws_text_area_model_rw::set_size(uint32 i_width, uint32 i_height) {}

void mws_text_area_model_rw::set_font(mws_sp<mws_font> i_font) {}

int mws_text_area_model_rw::get_char_at_pixel(float i_x, float i_y) { return 0; }

void mws_text_area_model_rw::insert_at_cursor(const std::string& i_text)
{
   text.insert(get_cursor_pos(), i_text);
   set_cursor_pos(get_cursor_pos() + i_text.length());
}

void mws_text_area_model_rw::delete_at_cursor(int32 i_count)
{
   if (i_count > 0)
   {
      text.erase(get_cursor_pos(), i_count);
   }
   else if (i_count < 0)
   {
      int start_idx = get_cursor_pos() + i_count;
      int count = -i_count;

      if (start_idx >= 0)
      {
         text.erase(start_idx, count);
         set_cursor_pos(start_idx);
      }
   }
}

uint32 mws_text_area_model_rw::get_cursor_pos()
{
   return cursor_pos;
}

void mws_text_area_model_rw::set_cursor_pos(uint32 i_cursor_pos)
{
   if (i_cursor_pos <= text.length())
   {
      cursor_pos = i_cursor_pos;
      cursor_grid_pos = glm::uvec2(0);

      for (uint32 k = 0; k < cursor_pos; k++)
      {
         cursor_grid_pos.x++;

         if (text[k] == '\n')
         {
            cursor_grid_pos.x = 0;
            cursor_grid_pos.y++;
         }
      }
   }
}

glm::uvec2 mws_text_area_model_rw::get_cursor_coord()
{
   return cursor_grid_pos;
}

uint32 mws_text_area_model_rw::get_cursor_pos_at_line(uint32 i_line_idx)
{
   int cursor_idx = 0;
   int line_idx = 0;
   uint32 length = text.length();

   if (i_line_idx == 0)
   {
      return 0;
   }

   for (uint32 k = 0; k < length; k++, cursor_idx++)
   {
      if (text[k] == '\n')
      {
         line_idx++;

         if (line_idx == i_line_idx)
         {
            return cursor_idx + 1;
         }
      }
   }

   return -1;
}

void mws_text_area_model_rw::advance_cursor(dir_types i_direction)
{
   switch (i_direction)
   {
   case DIR_LEFT:
      set_cursor_pos(get_cursor_pos() - 1);
      break;

   case DIR_UP:
   {
      glm::uvec2 pos = get_cursor_coord();
      uint32 row_idx = pos.y;

      if (row_idx > 0)
      {
         uint32 col_idx = pos.x;
         std::string prev_line = get_line_at(row_idx - 1);
         uint32 prev_line_pos = get_cursor_pos_at_line(row_idx - 1);
         uint32 prev_line_col_idx = std::max((int32)prev_line.length() - 1, 0);
         uint32 prev_col_idx = std::min(col_idx, prev_line_col_idx);

         set_cursor_pos(prev_line_pos + prev_col_idx);
      }
      else
      {
         set_cursor_pos(0);
      }
      break;
   }

   case DIR_RIGHT:
      set_cursor_pos(get_cursor_pos() + 1);
      break;

   case DIR_DOWN:
   {
      glm::uvec2 pos = get_cursor_coord();
      uint32 row_idx = pos.y;
      uint32 line_count = get_line_count();

      if (row_idx < line_count - 1)
      {
         uint32 col_idx = pos.x;
         std::string next_line = get_line_at(row_idx + 1);
         uint32 next_line_pos = get_cursor_pos_at_line(row_idx + 1);
         uint32 next_line_col_idx = std::max((int32)next_line.length() - 1, 0);
         uint32 next_col_idx = std::min(col_idx, next_line_col_idx);

         set_cursor_pos(next_line_pos + next_col_idx);
      }
      else
      {
         set_cursor_pos(text.length());
      }
      break;
   }
   }
}

#endif
