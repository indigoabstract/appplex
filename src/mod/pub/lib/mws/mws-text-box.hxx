#pragma once

#include "mws.hxx"
#include "mws-com.hxx"
#include <glm/vec2.hpp>


class mws_dp;
class font_glyph;
class mws_font;
class mws_camera;
class mws_text_area_model;
class text_vxo;


class mws_text_box : public mws_text_area
{
public:
   static mws_sp<mws_text_box> nwi();
   void setup() override;
   virtual bool is_editable() const;
   virtual void set_editable(bool i_is_editable);
   virtual void set_text(const std::string& i_text);
   virtual void push_back_text(const std::string& i_text);
   virtual void push_front_text(const std::string& i_text);
   virtual void insert_at_cursor(const std::string& i_text);
   virtual void delete_at_cursor(int32 i_count);
   virtual void scroll_text(const glm::vec2& i_off);
   virtual void set_position(const glm::vec2& i_pos);
   virtual void set_dimension(const glm::vec2& i_dim);
   virtual void select_char_at(const glm::vec2& i_pos);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual void update_text();
   virtual void update_gfx_cursor();

protected:
   mws_text_box() {}
   virtual mws_sp<mws_text_area_model> new_model();
   virtual void handle_pointer_evt(mws_sp<mws_ptr_evt> i_pe);
   virtual void handle_key_evt(mws_sp<mws_key_evt> i_ke);
   virtual float get_span(const font_glyph& i_glyph, std::string& i_text, int i_idx) const;

   bool editable = false;
   mws_sp<mws_text_area_model> tx_src;
   mws_sp<text_vxo> tx_vxo;
   mws_sp<mws_font> font;
   glm::vec2 pos = glm::vec2(0.f);
   glm::vec2 dim = glm::vec2(0.f);
   glm::vec2 text_offset = glm::vec2(0.f);
   float text_row_remainder = 0.f;
   uint32 top_line_idx = 0;
   uint32 text_rows = 0;
   uint32 cursor_row_idx = 0;
   uint32 cursor_col_idx = 0;
   kinetic_scrolling ks;
   std::vector<std::string> tx_rows;
   mws_sp<mws_rect> cursor_left;
   mws_sp<mws_rect> cursor_right;
   mws_sp<mws_rect> left_char_rect;
   mws_sp<mws_rect> right_char_rect;
   dragging_detector dragging_det;
};


class mws_text_field : public mws_text_box
{
public:
   static mws_sp<mws_text_field> nwi();

protected:
   mws_text_field() {}
};


class mws_text_area_model_ro : public mws_text_area_model
{
public:
   mws_text_area_model_ro();
   // common
   virtual bool get_word_wrap() override;
   virtual void set_word_wrap(bool i_word_wrap) override;
   virtual uint32 get_line_count() override;
   virtual std::string get_line_at(uint32 i_idx, bool i_keep_line_break = true) override;
   virtual std::vector<std::string> get_lines_at(uint32 i_idx, uint32 i_line_count, bool i_keep_line_break = true) override;
   virtual const std::string& get_text() override;
   virtual void set_text(const std::string& i_text) override;
   virtual void set_text(const char* i_text, uint32 i_length) override;
   virtual void set_size(uint32 i_width, uint32 i_height) override;
   virtual void set_font(mws_sp<mws_font> i_font) override;
   virtual int get_char_at_pixel(float i_x, float i_y) override;

   // viewer only
   virtual void push_back(const char* i_text, uint32 i_length) override;
   virtual void push_front(const char* i_text, uint32 i_length) override;

   // editor only
   virtual void insert_at_cursor(const std::string& i_text) override { mws_throw mws_exception("n/a"); }
   virtual void delete_at_cursor(int32 i_count) override { mws_throw mws_exception("n/a"); }
   virtual uint32 get_cursor_pos() override { mws_throw mws_exception("n/a"); }
   virtual void set_cursor_pos(uint32 i_cursor_pos) override { mws_throw mws_exception("n/a"); }
   virtual glm::ivec2 get_cursor_coord() override { mws_throw mws_exception("n/a"); }
   virtual uint32 get_cursor_pos_at_line(uint32 i_line_idx) override { mws_throw mws_exception("n/a"); }
   virtual void advance_cursor(dir_types i_direction) override { mws_throw mws_exception("n/a"); }

protected:
   void update_back_added_line_offsets(const std::string& i_new_text);
   void update_front_added_line_offsets(const std::string& i_new_text);
   void update_line_offsets();

   std::string text;
   bool word_wrap = false;
   std::vector<uint32> line_offsets;
   int width = 0;
   int height = 0;
   mws_sp<mws_font> font;
};


class mws_text_area_model_rw : public mws_text_area_model
{
public:
   mws_text_area_model_rw();
   // common
   virtual bool get_word_wrap() override;
   virtual void set_word_wrap(bool i_word_wrap) override;
   virtual uint32 get_line_count() override;
   virtual std::string get_line_at(uint32 i_idx, bool i_keep_line_break = true) override;
   virtual std::vector<std::string> get_lines_at(uint32 i_idx, uint32 i_line_count, bool i_keep_line_break = true) override;
   virtual const std::string& get_text() override;
   virtual void set_text(const std::string& i_text) override;
   virtual void set_text(const char* i_text, uint32 i_length) override;
   virtual void set_size(uint32 i_width, uint32 i_height) override;
   virtual void set_font(mws_sp<mws_font> i_font) override;
   virtual int get_char_at_pixel(float i_x, float i_y) override;

   // viewer only
   virtual void push_back(const char* i_text, uint32 i_length) override { mws_throw mws_exception("n/a"); }
   virtual void push_front(const char* i_text, uint32 i_length) override { mws_throw mws_exception("n/a"); }

   // editor only
   virtual void insert_at_cursor(const std::string& i_text) override;
   virtual void delete_at_cursor(int32 i_count) override;
   virtual uint32 get_cursor_pos() override;
   virtual void set_cursor_pos(uint32 i_cursor_pos) override;
   virtual glm::ivec2 get_cursor_coord() override;
   virtual uint32 get_cursor_pos_at_line(uint32 i_line_idx) override;
   virtual void advance_cursor(dir_types i_direction) override;

protected:
   std::string text;
   uint32 cursor_pos = 0;
   bool word_wrap = false;
   mws_sp<mws_font> font;
};
