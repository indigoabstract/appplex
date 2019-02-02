#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_MWS

#include "mws-text-box.hxx"
#include "mws-mod.hxx"
#include "mws-camera.hxx"
#include "mws-com.hxx"
#include "mws-font.hxx"
#include "font-db.hxx"
#include "text-vxo.hxx"
#include "min.hxx"


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
   tx_vxo = text_vxo::nwi();
   tx_vxo->camera_id_list.push_back("mws_cam");
   (*tx_vxo)[MP_SCISSOR_ENABLED] = true;
   font = mws_font::nwi(36.f);
   font->set_color(gfx_color::colors::white);
   left_char_rect = std::make_shared<mws_rect>();
   right_char_rect = std::make_shared<mws_rect>();
   attach(tx_vxo);
}

bool mws_text_box::is_editable() const
{
   return editable;
}

void mws_text_box::set_editable(bool i_is_editable)
{
   if (editable != i_is_editable)
   {
      auto src = new_model();

      if (tx_src)
      {
         std::string text = tx_src->get_text();
         src->set_text(text);
      }

      tx_src = src;
      editable = i_is_editable;
   }
}

void mws_text_box::set_text(const std::string& i_text)
{
   tx_src = new_model();
   tx_src->set_text(i_text);
   tx_vxo->clear_text();
   tx_vxo->add_text(i_text, glm::vec2(), font);
   scroll_text(glm::vec2());
   select_char_at(glm::vec2());
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

      int line_count = tx_src->get_line_count();
      std::string text;

      for (int k = 0; k < line_count; k++)
      {
         text += tx_src->get_line_at(k);// +"\n";
      }

      tx_vxo->clear_text();
      tx_vxo->add_text(text, glm::vec2(), font);
      scroll_text(glm::vec2());
      select_char_at(glm::vec2());
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

      int line_count = tx_src->get_line_count();
      std::string text;

      for (int k = 0; k < line_count; k++)
      {
         text += tx_src->get_line_at(k);// +"\n";
      }

      tx_vxo->clear_text();
      tx_vxo->add_text(text, glm::vec2(), font);
      scroll_text(glm::vec2());
      select_char_at(glm::vec2());
   }
}

void mws_text_box::insert_at_cursor(const std::string& i_text)
{
   tx_src->insert_at_cursor(i_text);
   update_text();
}

void mws_text_box::delete_at_cursor(int32 i_count)
{
   tx_src->delete_at_cursor(i_count);
   update_text();
}

void mws_text_box::scroll_text(const glm::vec2& i_off)
{
   //tx_vxo->position += glm::vec3(ioff, 0);
   glm::vec2 prev_off = text_offset;
   text_offset -= i_off;
   text_offset = glm::max(text_offset, glm::vec2());
   tx_vxo->clear_text();
   top_line_idx = int(text_offset.y / font->get_height());

   if (top_line_idx + text_rows > tx_src->get_line_count())
   {
      text_offset = prev_off;
      top_line_idx = int(text_offset.y / font->get_height());
      //top_line_idx = tx_src->get_line_count() - text_rows;
   }

   int rows = std::min(text_rows, tx_src->get_line_count());

   if (top_line_idx + rows >= tx_src->get_line_count())
   {
      top_line_idx = tx_src->get_line_count() - rows;
   }

   tx_rows = tx_src->get_lines_at(top_line_idx, rows);
   text_row_remainder = glm::mod(text_offset.y, font->get_height());
   tx_vxo->position = glm::vec3(pos.x - text_offset.x, pos.y - text_row_remainder, 0);

   for (size_t k = 0; k < tx_rows.size(); k++)
   {
      tx_vxo->add_text(tx_rows[k], glm::vec2(0, k * font->get_height()), font);
   }

   if (is_editable())
   {
      glm::ivec2 cursor_coord = tx_src->get_cursor_coord();
      cursor_row_idx = cursor_coord.y;
      cursor_col_idx = cursor_coord.x;

      update_gfx_cursor();
   }
}

void mws_text_box::set_position(const glm::vec2 & i_pos)
{
   pos = i_pos;
   tx_vxo->position = glm::vec3(i_pos, 0.f);
   (*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
}

void mws_text_box::set_dimension(const glm::vec2 & i_dim)
{
   dim = i_dim;
   (*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
   text_rows = int(dim.y / font->get_height() + 1);
   mws_r = mws_rect(pos.x, pos.y, dim.x, dim.y);

   if (tx_src)
   {
      scroll_text(glm::vec2());
      select_char_at(glm::vec2());
   }
}

void mws_text_box::select_char_at(const glm::vec2 & i_pos)
{
   cursor_row_idx = size_t((i_pos.y + text_row_remainder) / font->get_height());
   cursor_col_idx = 0;

   *left_char_rect = *right_char_rect = mws_rect();
   left_char_rect->h = right_char_rect->h = font->get_height();

   if (cursor_row_idx >= tx_rows.size())
   {
      cursor_row_idx = std::max(0, int(tx_rows.size() - 1));
      cursor_col_idx = tx_rows[cursor_row_idx].size();
      {
         uint32 line_index = top_line_idx + cursor_row_idx;
         uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
         glm::ivec2 cursor_coord = tx_src->get_cursor_coord();
         tx_src->set_cursor_pos(cursor_pos + cursor_col_idx);
         glm::ivec2 new_cursor_coord = tx_src->get_cursor_coord();
         mws_println("c0 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]", line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);
      }
      update_gfx_cursor();
      return;
   }

   std::string& text = tx_rows[cursor_row_idx];

   if (text.empty())
   {
      cursor_col_idx = 0;
      {
         uint32 line_index = top_line_idx + cursor_row_idx;
         uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
         glm::ivec2 cursor_coord = tx_src->get_cursor_coord();
         tx_src->set_cursor_pos(cursor_pos + cursor_col_idx);
         glm::ivec2 new_cursor_coord = tx_src->get_cursor_coord();
         mws_println("c1 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]", line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);
      }
      update_gfx_cursor();
      return;
   }

   auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
   float x_off = -text_offset.x;
   size_t text_length = (text.back() != '\n') ? text.length() : text.length() - 1;
   size_t k = 0;

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
                  cursor_col_idx = k;
               }
               else
               {
                  cursor_col_idx = k;
               }
            }
            // cursor falls to the right of this character
            else
            {
               cursor_col_idx = k + 1;
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
      cursor_col_idx = text_length;
   }

   if (is_editable())
   {
      uint32 line_index = top_line_idx + cursor_row_idx;
      uint32 cursor_pos = tx_src->get_cursor_pos_at_line(line_index);
      glm::ivec2 cursor_coord = tx_src->get_cursor_coord();
      tx_src->set_cursor_pos(cursor_pos + cursor_col_idx);
      glm::ivec2 new_cursor_coord = tx_src->get_cursor_coord();
      mws_println("c2 [line %d, pos %d] old-coord[%d, %d] new-coord[%d, %d]", line_index, cursor_pos, cursor_coord.x, cursor_coord.y, new_cursor_coord.x, new_cursor_coord.y);
      update_gfx_cursor();
   }
}

void mws_text_box::update_state()
{
   if (ks.is_active())
   {
      point2d scroll_pos = ks.update();
      scroll_text(glm::vec2(scroll_pos.x, scroll_pos.y));
   }
}

void mws_text_box::update_view(mws_sp<mws_camera> g)
{
   g->drawRect(pos.x, pos.y, dim.x, dim.y);

   if (cursor_left && cursor_right)
   {
      mws_rect cursor_rect(cursor_left->x + pos.x, cursor_left->y + pos.y, cursor_right->x - cursor_left->x + cursor_right->w, std::max(cursor_left->h, cursor_right->h));
      g->drawRect(cursor_rect.x, cursor_rect.y, cursor_rect.w, cursor_rect.h);
   }
   else if (cursor_left || cursor_right)
   {
      if (cursor_left)
      {
         g->drawRect(cursor_left->x + pos.x, cursor_left->y + pos.y, cursor_left->w, cursor_left->h);
      }
      else if (cursor_right)
      {
         g->drawRect(cursor_right->x + pos.x, cursor_right->y + pos.y, cursor_right->w, cursor_right->h);
      }
   }
}

void mws_text_box::receive(mws_sp<mws_dp> i_dp)
{
   if (i_dp->is_processed())
   {
      return;
   }

   if (i_dp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      handle_pointer_evt(pointer_evt::as_pointer_evt(i_dp));
   }
   else if (i_dp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      handle_key_evt(key_evt::as_key_evt(i_dp));
   }
}

void mws_text_box::update_text()
{
   tx_vxo->clear_text();
   top_line_idx = int(text_offset.y / font->get_height());

   if (top_line_idx + text_rows > tx_src->get_line_count())
   {
      top_line_idx = int(text_offset.y / font->get_height());
   }

   int rows = std::min(text_rows, tx_src->get_line_count());

   if (top_line_idx + rows >= tx_src->get_line_count())
   {
      top_line_idx = tx_src->get_line_count() - rows;
   }

   tx_rows = tx_src->get_lines_at(top_line_idx, rows);
   text_row_remainder = glm::mod(text_offset.y, font->get_height());
   tx_vxo->position = glm::vec3(pos.x - text_offset.x, pos.y - text_row_remainder, 0);

   for (size_t k = 0; k < tx_rows.size(); k++)
   {
      tx_vxo->add_text(tx_rows[k], glm::vec2(0, k * font->get_height()), font);
   }

   glm::ivec2 cursor_coord = tx_src->get_cursor_coord();
   cursor_row_idx = cursor_coord.y;
   cursor_col_idx = cursor_coord.x;

   update_gfx_cursor();
}

void mws_text_box::update_gfx_cursor()
{
   std::string& text = tx_rows[cursor_row_idx];

   // if we have at least one char on this line( even if it's a single '\n' )
   if (!text.empty())
   {
      auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
      bool is_new_line_terminated = (text.back() == '\n');
      size_t text_length = (is_new_line_terminated) ? text.length() - 1 : text.length();
      float x_off = -text_offset.x;
      mws_assert(cursor_col_idx <= text_length);

      // special case for cursor position before first char
      if (cursor_col_idx == 0)
      {
         float span = get_span(glyphs[0], text, 0);
         cursor_left = nullptr;
         right_char_rect->x = x_off;
         right_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
         right_char_rect->w = span;
         cursor_right = right_char_rect;
      }
      // special case for cursor position after last char
      else if (cursor_col_idx == text_length)
      {
         // either last char is a new line and the cursor position is just before it
         // or, there's no new line at the end, so the cursor position is after the last char
         // we treat both cases the same
         for (size_t k = 0; k < cursor_col_idx - 1; k++)
         {
            font_glyph glyph = glyphs[k];
            float span = get_span(glyph, text, k);
            x_off += span;
         }

         left_char_rect->x = x_off;
         left_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
         left_char_rect->w = get_span(glyphs[cursor_col_idx - 1], text, cursor_col_idx - 1);
         cursor_left = left_char_rect;

         right_char_rect->x = x_off + left_char_rect->w;
         right_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
         right_char_rect->w = font->get_text_width(nl_char_size);
         cursor_right = right_char_rect;
      }
      // cursor position is between the first and last char
      else
      {
         for (size_t k = 0; k < cursor_col_idx - 1; k++)
         {
            font_glyph glyph = glyphs[k];
            float span = get_span(glyph, text, k);
            x_off += span;
         }

         left_char_rect->x = x_off;
         left_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
         left_char_rect->w = get_span(glyphs[cursor_col_idx - 1], text, cursor_col_idx - 1);
         cursor_left = left_char_rect;

         right_char_rect->x = x_off + left_char_rect->w;
         right_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
         right_char_rect->w = get_span(glyphs[cursor_col_idx], text, cursor_col_idx);
         cursor_right = right_char_rect;
      }
   }
   // empty line
   else
   {
      cursor_left = left_char_rect;
      left_char_rect->x = 0.f;
      left_char_rect->y = cursor_row_idx * font->get_height() - text_row_remainder;
      left_char_rect->w = font->get_text_width(nl_char_size);
      cursor_right = nullptr;
   }
}

mws_sp<mws_text_area_model> mws_text_box::new_model()
{
   if (editable)
   {
      return std::make_shared<mws_text_area_model_rw>();
   }

   return std::make_shared<mws_text_area_model_ro>();
}

void mws_text_box::handle_pointer_evt(mws_sp<pointer_evt> i_pe)
{
   bool hit = is_inside_box(i_pe->points[0].x, i_pe->points[0].y, pos.x, pos.y, mws_r.w, mws_r.h);

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
      i_pe->process();
   }

   switch (i_pe->type)
   {
   case pointer_evt::touch_began:
   {
      if (pfm::has_touchscreen() || get_mod()->get_preferences()->emulate_mobile_screen())
      {
         auto inst = static_pointer_cast<mws_text_area>(get_instance());
         get_mws_root()->show_keyboard(inst);
      }

      ks.grab(x, y);
      select_char_at(glm::vec2(x, y));
      //mws_print("touch [%f, %f]\n", i_pe->points[0].x, i_pe->points[0].y);

      if (!i_pe->is_processed()) { i_pe->process(); }
      break;
   }

   case pointer_evt::touch_ended:
   {
      if (!i_pe->is_processed()) { i_pe->process(); }
      break;
   }
   }
}

void mws_text_box::handle_key_evt(mws_sp<key_evt> i_ke)
{
   if (i_ke->get_type() != key_evt::KE_RELEASED)
   {
      key_types key = i_ke->get_key();

      if (key >= KEY_SPACE && key <= KEY_TILDE_SIGN)
      {
         if (editable)
         {
            if (key >= KEY_A && key <= KEY_Z)
            {
               if (editable)
               {
                  bool shift_held = get_mod()->key_ctrl_inst->key_is_held(KEY_SHIFT);
                  char key_char = char(key + ('a' - 'A'));

                  if (shift_held)
                  {
                     key_char = (char)key;
                  }

                  std::string key_str(1, key_char);

                  insert_at_cursor(key_str);
               }
            }
            else
            {
               char key_char = (char)key;
               std::string key_str(1, key_char);

               insert_at_cursor(key_str);
            }
         }
      }
      else
      {
         float off = 51.175f;

         if (i_ke->get_type() == key_evt::KE_PRESSED)
         {
            off = 21.175f;
         }

         switch (key)
         {
         case KEY_LEFT:
         {
            tx_src->advance_cursor(dir_types::DIR_LEFT);
            scroll_text(glm::vec2(off, 0));
            break;
         }

         case KEY_UP:
         {
            tx_src->advance_cursor(dir_types::DIR_UP);
            scroll_text(glm::vec2(0, off));
            break;
         }

         case KEY_RIGHT:
         {
            tx_src->advance_cursor(dir_types::DIR_RIGHT);
            scroll_text(glm::vec2(-off, 0));
            break;
         }

         case KEY_DOWN:
         {
            tx_src->advance_cursor(dir_types::DIR_DOWN);
            scroll_text(glm::vec2(0, -off));
            break;
         }

         case KEY_DELETE:
         {
            if (editable)
            {
               delete_at_cursor(1);
            }
            break;
         }

         case KEY_BACKSPACE:
         {
            if (editable)
            {
               delete_at_cursor(-1);
            }
            break;
         }

         case KEY_TAB:
         {
            if (editable)
            {
               insert_at_cursor("   ");
            }
            break;
         }

         case KEY_ENTER:
         {
            if (editable)
            {
               insert_at_cursor("\n");
            }
            break;
         }
         }
      }
   }
}

float mws_text_box::get_span(const font_glyph & i_glyph, std::string & i_text, int i_idx) const
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

void mws_text_area_model_ro::set_text(const std::string & i_text)
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

void mws_text_area_model_ro::update_back_added_line_offsets(const std::string & i_new_text)
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

void mws_text_area_model_ro::update_front_added_line_offsets(const std::string & i_new_text)
{
   std::vector<uint32> lo;
   int len = i_new_text.length();
   int idx = 1;

   for (size_t k = 1; k < line_offsets.size(); k++)
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

void mws_text_area_model_rw::set_text(const std::string & i_text)
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

void mws_text_area_model_rw::insert_at_cursor(const std::string & i_text)
{
   text.insert(cursor_pos, i_text);
   cursor_pos += i_text.length();
   mws_println(text.c_str());
}

void mws_text_area_model_rw::delete_at_cursor(int32 i_count)
{
   if (i_count > 0)
   {
      text.erase(cursor_pos, i_count);
   }
   else if (i_count < 0)
   {
      int start_idx = cursor_pos + i_count;
      int count = -i_count;

      if (start_idx >= 0)
      {
         text.erase(start_idx, count);
         cursor_pos = start_idx;
      }
   }
   mws_println(text.c_str());
}

uint32 mws_text_area_model_rw::get_cursor_pos()
{
   return cursor_pos;
}

void mws_text_area_model_rw::set_cursor_pos(uint32 i_cursor_pos)
{
   if (i_cursor_pos >= 0 && i_cursor_pos <= text.length())
   {
      cursor_pos = i_cursor_pos;
   }
}

glm::ivec2 mws_text_area_model_rw::get_cursor_coord()
{
   glm::ivec2 cursor_coord(0);

   for (uint32 k = 0; k < cursor_pos; k++)
   {
      cursor_coord.x++;

      if (text[k] == '\n')
      {
         cursor_coord.x = 0;
         cursor_coord.y++;
      }
   }

   return cursor_coord;
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
      set_cursor_pos(cursor_pos - 1);
      break;

   case DIR_UP:
   {
      glm::ivec2 pos = get_cursor_coord();
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
      set_cursor_pos(cursor_pos + 1);
      break;

   case DIR_DOWN:
   {
      glm::ivec2 pos = get_cursor_coord();
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
