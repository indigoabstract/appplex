#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_MWS

#include "mws-text-box.hpp"
#include "unit.hpp"
#include "mws-camera.hpp"
#include "mws-com.hpp"
#include "mws-font.hpp"
#include "font-db.hpp"
#include "text-vxo.hpp"
#include "min.hpp"


mws_text_area_model_ro::mws_text_area_model_ro()
{
   update_line_offsets();
}

bool mws_text_area_model_ro::get_word_wrap() { return word_wrap; }

void mws_text_area_model_ro::set_word_wrap(bool i_word_wrap) { word_wrap = i_word_wrap; }

int mws_text_area_model_ro::get_line_count() { return line_offsets.size() - 1; }

std::string mws_text_area_model_ro::get_line_at(int i_idx, bool i_keep_line_break)
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

std::vector<std::string> mws_text_area_model_ro::get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break)
{
   std::vector<std::string> lines;

   for (int k = 0; k < i_line_count; k++)
   {
      lines.push_back(get_line_at(i_idx + k, i_keep_line_break));
   }

   return lines;
}

std::string mws_text_area_model_ro::get_text()
{
   return text;
}

void mws_text_area_model_ro::push_back(const char* i_text, int i_length)
{
   std::string new_text(i_text, i_length);
   text += new_text;
   update_back_added_line_offsets(new_text);
}

void mws_text_area_model_ro::push_front(const char* i_text, int i_length)
{
   std::string new_text(i_text, i_length);
   text = new_text + text;
   update_front_added_line_offsets(new_text);
}

void mws_text_area_model_ro::insert_at_cursor(const std::string& i_text)
{
   mws_throw ia_exception("n/a");
}

void mws_text_area_model_ro::delete_at_cursor(int32 i_count)
{
   mws_throw ia_exception("n/a");
}

uint32 mws_text_area_model_ro::get_cursor_pos()
{
   mws_throw ia_exception("n/a");
}

void mws_text_area_model_ro::set_cursor_pos(uint32 i_cursor_pos)
{
   mws_throw ia_exception("n/a");
}

void mws_text_area_model_ro::set_text(const std::string& i_text)
{
   set_text(i_text.c_str(), i_text.length());
}

void mws_text_area_model_ro::set_text(const char* i_text, int i_length)
{
   text = std::string(i_text, i_length);
   update_line_offsets();
}

void mws_text_area_model_ro::set_size(int i_width, int i_height) {}

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

int mws_text_area_model_rw::get_line_count()
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

std::string mws_text_area_model_rw::get_line_at(int i_idx, bool i_keep_line_break)
{
   int length = text.length();
   int line_count = 0;
   int idx = 0;
   int crt_idx = 0;
   std::string line;

   for (int k = 0; k < length; k++)
   {
      crt_idx = k + 1;

      if (text[k] == '\n')
      {
         if (i_idx == line_count)
         {
            if (!i_keep_line_break)
            {
               // if not empty line, discard the new line character at the end
               if (crt_idx > 0)
               {
                  crt_idx--;
               }
            }

            break;
         }

         line_count++;
         idx = k;
      }
   }

   line = text.substr(idx, crt_idx - idx);

   return line;
}

std::vector<std::string> mws_text_area_model_rw::get_lines_at(int i_idx, int i_line_count, bool i_keep_line_break)
{
   std::vector<std::string> lines;

   for (int k = 0; k < i_line_count; k++)
   {
      lines.push_back(get_line_at(i_idx + k, i_keep_line_break));
   }

   return lines;
}

std::string mws_text_area_model_rw::get_text()
{
   return text;
}

void mws_text_area_model_rw::push_back(const char* i_text, int i_length)
{
   mws_throw ia_exception("n/a");
}

void mws_text_area_model_rw::push_front(const char* i_text, int i_length)
{
   mws_throw ia_exception("n/a");
}

void mws_text_area_model_rw::insert_at_cursor(const std::string& i_text)
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
   if (cursor_pos >= 0 && cursor_pos < text.length())
   {
      cursor_pos = i_cursor_pos;
   }
}

void mws_text_area_model_rw::set_text(const std::string& i_text)
{
   text = i_text;
}

void mws_text_area_model_rw::set_text(const char* i_text, int i_length)
{
   text = std::string(i_text, i_length);
}

void mws_text_area_model_rw::set_size(int i_width, int i_height) {}
void mws_text_area_model_rw::set_font(mws_sp<mws_font> i_font) {}
int mws_text_area_model_rw::get_char_at_pixel(float i_x, float i_y) { return 0; }


mws_sp<mws_text_box> mws_text_box::nwi()
{
   mws_sp<mws_text_box> inst(new mws_text_box());
   inst->setup();
   return inst;
}

void mws_text_box::setup()
{
   mws_page_item::setup();
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
   scroll_text(glm::vec2(0.f));
}

void mws_text_box::delete_at_cursor(int32 i_count)
{
   tx_src->delete_at_cursor(i_count);
   scroll_text(glm::vec2(0.f));
}

void mws_text_box::scroll_text(const glm::vec2& ioff)
{
   //tx_vxo->position += glm::vec3(ioff, 0);
   glm::vec2 prev_off = text_offset;
   text_offset -= ioff;
   text_offset = glm::max(text_offset, glm::vec2());
   tx_vxo->clear_text();
   int line_idx = int(text_offset.y / font->get_height());

   if (line_idx + text_rows > tx_src->get_line_count())
   {
      text_offset = prev_off;
      line_idx = int(text_offset.y / font->get_height());
      //line_idx = tx_src->get_line_count() - text_rows;
   }

   int rows = std::min(text_rows, tx_src->get_line_count());

   if (line_idx + rows >= tx_src->get_line_count())
   {
      line_idx = tx_src->get_line_count() - rows;
   }

   tx_rows = tx_src->get_lines_at(line_idx, rows);
   float rem = glm::mod(text_offset.y, font->get_height());
   tx_vxo->position = glm::vec3(pos.x - text_offset.x, pos.y - rem, 0);

   for (size_t k = 0; k < tx_rows.size(); k++)
   {
      tx_vxo->add_text(tx_rows[k], glm::vec2(0, k * font->get_height()), font);
   }
}

void mws_text_box::set_position(const glm::vec2& ipos)
{
   pos = ipos;
   tx_vxo->position = glm::vec3(ipos, 0.f);
   (*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
}

void mws_text_box::set_dimension(const glm::vec2& idim)
{
   dim = idim;
   (*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
   text_rows = int(dim.y / font->get_height() + 1);
   mws_r = mws_rect(pos.x, pos.y, dim.x, dim.y);

   if (tx_src)
   {
      scroll_text(glm::vec2());
      select_char_at(glm::vec2());
   }
}

void mws_text_box::select_char_at(const glm::vec2& ipos)
{
   float rem = glm::mod(text_offset.y, font->get_height());
   size_t row_idx = size_t((ipos.y + rem) / font->get_height());

   if (row_idx >= tx_rows.size())
   {
      select_char_rect = glm::vec4(0.f);
      return;
   }

   std::string& text = tx_rows[row_idx];
   auto& glyphs = font_db::inst()->get_glyph_vect(font->get_inst(), text);
   float x_off = -text_offset.x;
   size_t k = 0;

   for (k = 0; k < text.length(); k++)
   {
      font_glyph glyph = glyphs[k];

      if (glyph.is_valid())
      {
         float span = 0;
         char ch = text[k];
         if (ch == '\t')
         {
            span = 2 * font->get_height();
         }
         else
         {
            float kerning = 0.0f;
            if (k > 0)
            {
               kerning = glyph.get_kerning(text[k - 1]);
            }
            span = kerning + glyph.get_advance_x();
         }

         if (x_off + span > ipos.x)
         {
            break;
         }
         else
         {
            x_off += span;
         }
      }
   }

   if (k < text.length())
   {
      char c = text[k];
      //mws_print("%c %d %d\n", c, row_idx, k);
      select_char_rect.x = x_off;
      select_char_rect.y = row_idx * font->get_height() - rem;
      select_char_rect.z = (glyphs.size() > k) ? glyphs[k].get_advance_x() : 0;
      select_char_rect.w = font->get_height();
   }
   else
   {
      select_char_rect = glm::vec4(0.f);
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
   g->drawRect(select_char_rect.x + pos.x, select_char_rect.y + pos.y, select_char_rect.z, select_char_rect.w);
}

void mws_text_box::receive(mws_sp<iadp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      handle_pointer_evt(pointer_evt::as_pointer_evt(idp));
   }
   else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      handle_key_evt(key_evt::as_key_evt(idp));
   }
}

mws_text_box::mws_text_box()
{
   tx_vxo = text_vxo::nwi();
   tx_vxo->camera_id_list.push_back("mws_cam");
   (*tx_vxo)[MP_SCISSOR_ENABLED] = true;
   font = mws_font::nwi(48);
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
      if (pfm::has_touchscreen() || get_unit()->get_preferences()->emulate_mobile_screen())
      {
         auto inst = static_pointer_cast<mws_text_area>(get_instance());
         get_mws_root()->show_keyboard(inst);
      }

      ks.grab(x, y);
      select_char_at(glm::vec2(x, y));
      mws_print("touch [%f, %f]\n", i_pe->points[0].x, i_pe->points[0].y);

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
                  bool shift_held = get_unit()->key_ctrl_inst->key_is_held(KEY_SHIFT);
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
            scroll_text(glm::vec2(off, 0));
            break;
         }

         case KEY_UP:
         {
            scroll_text(glm::vec2(0, off));
            break;
         }

         case KEY_RIGHT:
         {
            scroll_text(glm::vec2(-off, 0));
            break;
         }

         case KEY_DOWN:
         {
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

#endif
