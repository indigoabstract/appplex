#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEXXED

#include "mws-text-box.hpp"
#include "gap-buffer.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "com/mws/mws-font.hpp"
#include "com/mws/font-db.hpp"
#include "com/mws/text-vxo.hpp"
#include "min.hpp"


mws_sp<text_box> text_box::nwi()
{
   mws_sp<text_box> inst(new text_box());
   inst->setup();
   return inst;
}

void text_box::setup()
{
   mws_page_item::setup();
   attach(tx_vxo);
}

void text_box::set_text(const std::string& i_text)
{
   //tx_src = std::make_shared<GapBuffer>(i_text);
   tx_src = std::make_shared<text_area_model_ro>();
   tx_src->set_text(i_text);
   tx_vxo->clear_text();
   tx_vxo->add_text(i_text, glm::vec2(), font);
   scroll_text(glm::vec2());
   select_char_at(glm::vec2());
}

void text_box::push_back_text(const std::string& i_text)
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

void text_box::push_front_text(const std::string& i_text)
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

void text_box::insert_at_cursor(const std::string& i_text)
{
}

void text_box::delete_at_cursor(int32 i_count)
{
}

void text_box::scroll_text(const glm::vec2& ioff)
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

void text_box::set_position(const glm::vec2& ipos)
{
   pos = ipos;
   tx_vxo->position = glm::vec3(ipos, 0.f);
   (*tx_vxo)[MP_SCISSOR_AREA] = glm::vec4(pos, dim);
}

void text_box::set_dimension(const glm::vec2& idim)
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

void text_box::select_char_at(const glm::vec2& ipos)
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
      mws_print("%c %d %d\n", c, row_idx, k);
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

void text_box::update_state()
{
   if (ks.is_active())
   {
      point2d scroll_pos = ks.update();
      scroll_text(glm::vec2(scroll_pos.x, scroll_pos.y));
   }
}

void text_box::update_view(mws_sp<mws_camera> g)
{
   g->drawRect(pos.x, pos.y, dim.x, dim.y);
   g->drawRect(select_char_rect.x + pos.x, select_char_rect.y + pos.y, select_char_rect.z, select_char_rect.w);
}

void text_box::receive(mws_sp<iadp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);
      bool hit = is_inside_box(ts->points[0].x, ts->points[0].y, pos.x, pos.y, mws_r.w, mws_r.h);

      if (!hit)
      {
         return;
      }
      //mws_print("hit at [%f, %f]\n", ts->points[0].x, ts->points[0].y);

      float x = ts->points[0].x - mws_r.x;
      float y = ts->points[0].y - mws_r.y;
      bool dragging_detected = dragging_det.detect_helper(ts);

      if (dragging_detected)
      {
         if (dragging_det.is_finished())
         {
            uint32 delta_t = ts->time - dragging_det.last_move_pos_time;

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
            ks.begin(ts->points[0].x, ts->points[0].y);
         }

         scroll_text(dragging_det.drag_diff);
         ts->process();
      }

      switch (ts->type)
      {
      case pointer_evt::touch_began:
      {
         auto inst = static_pointer_cast<mws_text_area>(get_instance());

         ks.grab(x, y);
         select_char_at(glm::vec2(x, y));
         get_mws_root()->show_keyboard(inst);
         mws_print("touch [%f, %f]\n", ts->points[0].x, ts->points[0].y);

         if (!ts->is_processed()) { ts->process(); }
         break;
      }

      case pointer_evt::touch_ended:
      {
         if (!ts->is_processed()) { ts->process(); }
         break;
      }
      }
   }
   else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      mws_sp<key_evt> ke = key_evt::as_key_evt(idp);
      float off = 51.175f;

      if (ke->get_type() != key_evt::KE_RELEASED)
      {
         if (ke->get_type() == key_evt::KE_PRESSED)
         {
            off = 21.175f;
         }

         switch (ke->get_key())
         {
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

         case KEY_LEFT:
         {
            scroll_text(glm::vec2(off, 0));
            break;
         }
         }
      }
   }

   //mws_page::receive(idp);
}

text_box::text_box()
{
   tx_vxo = text_vxo::nwi();
   tx_vxo->camera_id_list.push_back("mws_cam");
   (*tx_vxo)[MP_SCISSOR_ENABLED] = true;
   font = mws_font::nwi(48);
}

#endif
