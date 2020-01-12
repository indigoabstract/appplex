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
class gfx_quad_2d;


class mws_text_box : public mws_text_area
{
public:
   static mws_sp<mws_text_box> nwi();
   void setup() override;
   virtual bool is_action_key(key_types i_key) const override;
   virtual void do_action() override;
   virtual bool is_editable() const;
   virtual void set_editable(bool i_is_editable);
   virtual const std::string& get_text() const;
   virtual void set_text(const std::string& i_text) override;
   virtual void push_back_text(const std::string& i_text);
   virtual void push_front_text(const std::string& i_text);
   virtual void insert_at_cursor(const std::string& i_text);
   virtual void delete_at_cursor(int32 i_count);
   virtual void scroll_text(const glm::vec2& i_off, bool i_snap_to_grid = false);
   virtual void scroll_to_end(dir_types i_direction);
   virtual void set_position(const glm::vec2& i_pos);
   virtual void set_dimension(const glm::vec2& i_dim);
   virtual void set_font(mws_sp<mws_font> i_font);
   virtual void select_char_at(const glm::vec2& i_pos);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   virtual void on_focus_changed(bool i_has_focus);
   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual mws_rect get_cursor_rect(cursor_types i_cursor_type, bool i_absolute_pos = true) override;
   virtual void update_cursor();
   virtual void update_gfx_cursor();

   // called after a click inside the text box
   std::function<void()> on_click;
   // called after gaining focus(if did not already have focus)
   std::function<void()> on_gained_focus;
   // called after losing focus
   std::function<void()> on_lost_focus;
   // called after enter(or an action key) was pressed
   std::function<void()> on_action;
   // called after a key was pressed
   std::function<void(key_types i_key)> on_key_pressed;

protected:
   class text_view : public mws_page_item
   {
   public:
      virtual void setup();
      virtual void select_char_at(const glm::vec2& i_pos);
      virtual void scroll_text(const glm::vec2& i_offset, bool i_snap_to_grid = false);
      virtual void sync_view();
      virtual void sync_view_to_cursor_pos();
      virtual float get_span(const font_glyph& i_glyph, std::string& i_text, int i_idx) const;
      virtual void update_cursor();
      virtual mws_sp<mws_text_box> get_text_box() { return static_pointer_cast<mws_text_box>(get_parent()); }
      virtual mws_rect get_cursor_rect(cursor_types i_cursor_type);
      virtual uint32 get_top_line_idx();

      mws_sp<mws_text_area_model> tx_src;
      mws_sp<text_vxo> tx_vxo;
      mws_sp<mws_font> font;
      // text grid, contains a view of tx_src, only as much text as it fits on the screen
      std::vector<std::string> tx_lines;
      // cursor position in the above text grid. x or y coord can be negative
      glm::ivec2 cursor_grid_pos;
      // used for scrolling. the vertical text offset is always less than font's height
      glm::vec2 tx_offset;
      // number of lines visible on screen
      uint32 max_lines_allowed_by_height = 0;
      // points to the first line of the text grid and it's an index into the lines of tx_src
      int32 top_line_idx = 0;
      // maximum width of the lines in the text grid
      float max_line_width = 0.f;

      mws_sp<mws_rect> cursor_left;
      mws_sp<mws_rect> cursor_right;
      mws_sp<mws_rect> left_char_rect;
      mws_sp<mws_rect> right_char_rect;
      // add 2 extra lines of text at the bottom to prevent the 'popping' text becoming visible when scrolling upwards
      inline static const uint32 extra_lines_count = 2;
   };


   mws_text_box() {}
   virtual void on_attach() override;
   virtual void sync_view();
   virtual void sync_view_to_cursor_pos();
   virtual void update_text_view();
   virtual mws_sp<mws_text_area_model> new_model();
   virtual void handle_pointer_evt(mws_sp<mws_ptr_evt> i_pe);
   virtual void handle_key_evt(mws_sp<mws_key_evt> i_ke);
   virtual float get_span(const font_glyph& i_glyph, std::string& i_text, int i_idx) const;

   mws_sp<text_view> view;
   bool editable = false;
   mws_sp<mws_text_area_model> tx_src;
   mws_sp<gfx_node> gfx_cursor;
   mws_sp<gfx_quad_2d> gfx_cursor_bg;
   mws_sp<gfx_quad_2d> gfx_cursor_middle;
   //mws_sp<gfx_quad_2d> gfx_cursor_left;
   //mws_sp<gfx_quad_2d> gfx_cursor_right;
   ping_pong_time_slider<float> gfx_cursor_slider;
   mws_sp<mws_font> font;
   kinetic_scrolling ks;
   dragging_detector dragging_det;
   mws_sp<gfx_quad_2d> debug_bg;
};


class mws_text_field : public mws_text_box
{
public:
   static mws_sp<mws_text_field> nwi();
   virtual bool is_action_key(key_types i_key) const override;
   virtual void insert_at_cursor(const std::string& i_text) override;
   virtual void set_text(const std::string& i_text) override;

protected:
   mws_text_field() {}
   static void format_text(std::string& i_text);
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
   virtual glm::uvec2 get_cursor_coord() override { mws_throw mws_exception("n/a"); }
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
   virtual glm::uvec2 get_cursor_coord() override;
   virtual uint32 get_cursor_pos_at_line(uint32 i_line_idx) override;
   virtual void advance_cursor(dir_types i_direction) override;

protected:
   std::string text;
   // cursor linear position(index in the 'text' element)
   uint32 cursor_pos = 0;
   // [x, y] cursor position. y tells the line position, x tells the position withing that line 
   glm::uvec2 cursor_grid_pos = glm::uvec2(0);
   bool word_wrap = false;
   mws_sp<mws_font> font;
};
