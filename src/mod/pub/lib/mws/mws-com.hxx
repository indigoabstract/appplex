#pragma once

#include "mws.hxx"
#include "input/gesture-detectors.hxx"
#include "gfx-color.hxx"
#include "gfx-quad-2d.hxx"
#include <stack>


class gfx_quad_2d;
class mws_font;


class mws_table_border : public gfx_2d_sprite
{
public:
   static mws_sp<mws_table_border> nwi();
   void set_color(const gfx_color& i_color);

protected:
   mws_table_border();
   void setup();
};


class mws_table_layout : public mws_layout
{
public:
   enum border_types
   {
      e_left_border,
      e_top_border,
      e_right_border,
      e_btm_border,
   };

   static mws_sp<mws_table_layout> nwi();
   virtual void on_resize() override;
   virtual void set_position(const glm::vec2& i_position) override;
   virtual void set_size(const glm::vec2& i_size) override;
   virtual void add_row(mws_sp<mws_page_item> i_item);
   virtual void add_col(uint32_t i_row_idx, mws_sp<mws_page_item> i_item);
   virtual void set_cell_at(uint32_t i_row_idx, uint32_t i_col_idx, mws_sp<mws_page_item> i_item);
   // returns one of the 4 table enclosing borders
   virtual mws_sp<mws_table_border> get_border(border_types i_border_type) const;
   // returns one of the inner row borders/dividers
   virtual mws_sp<mws_table_border> get_row_divider(uint32_t i_row_idx) const;
   // returns one of the inner col borders/dividers in the specified row
   virtual mws_sp<mws_table_border> get_col_divider(uint32_t i_row_idx, uint32_t i_col_idx) const;
   virtual void set_color(const gfx_color& i_color);
   virtual float get_border_size() const;
   virtual void set_border_size(float i_border_size);

protected:
   mws_table_layout() {}
   virtual void setup() override;

   gfx_color color;
   float border_size = 0.f;
   std::vector<mws_sp<mws_table_border>> borders;
   std::vector<mws_sp<mws_page_item>> item_rows;
   std::vector<mws_sp<mws_table_border>> row_divs;
};


class mws_stack_page_nav : public mws_page_nav
{
public:
   static mws_sp<mws_stack_page_nav> nwi(mws_sp<mws_page_tab> i_tab);
   std::string get_main_page_id() const override;
   void set_main_page_id(const std::string& i_main_page_id) override;
   void pop() override;
   void push(std::string i_page_id) override;
   void reset_pages() override;

protected:
   mws_stack_page_nav() {}
   virtual void set_current(const std::string& i_page_id);
   virtual void setup();

   mws_wp<mws_page_tab> tab;
   std::stack<std::string> page_stack;
   std::string main_page_id;
};


class mws_panel : public mws_page_item
{
public:
   static mws_sp<mws_panel> nwi();
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual mws_sp<gfx_quad_2d> get_vxo();

protected:
   mws_panel() {}
   void setup() override;

   mws_sp<gfx_quad_2d> vxo;
};


class mws_label : public mws_page_item
{
public:
   static mws_sp<mws_label> nwi();
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void update_state() override;
   virtual void set_text(std::string i_text);
   virtual mws_sp<mws_font> get_font() const;
   virtual void set_font(mws_sp<mws_font> i_font);

protected:
   mws_label() {}
   void setup() override;

   std::string text;
   mws_sp<mws_font> font;
};


class mws_img_btn : public mws_page_item
{
public:
   static mws_sp<mws_img_btn> nwi();
   void set_enabled(bool i_is_enabled) override;
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void set_img_name(std::string i_img_name);
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual bool is_hit(float x, float y);
   virtual void on_click();
   virtual mws_sp<gfx_quad_2d> get_vxo();

   std::function<void()> on_click_handler;

protected:
   mws_img_btn() {}
   void setup() override;

   mws_sp<gfx_quad_2d> vxo;
};


class mws_button : public mws_page_item
{
public:
   static mws_sp<mws_button> nwi();
   void set_enabled(bool i_is_enabled);
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual bool is_hit(float x, float y);
   virtual void update_state() override;
   virtual const std::string& get_text() const;
   virtual void set_text(std::string i_text);
   virtual void set_text_visible(bool i_visible) { text_visible = i_visible; }
   virtual const gfx_color& get_bg_color() const;
   virtual void set_bg_color(const gfx_color& i_color);
   virtual void set_bg_visible(bool i_visible);
   virtual mws_sp<mws_font> get_font() const;
   virtual void set_font(mws_sp<mws_font> i_font);
   virtual mws_sp<gfx_quad_2d> get_vxo();

   std::function<void()> on_click_handler;

protected:
   mws_button() {}
   void setup() override;

   std::string text;
   bool text_visible = true;
   gfx_color color;
   mws_sp<mws_font> font;
   mws_sp<gfx_quad_2d> vxo;
};


class mws_slider : public mws_page_item
{
public:
   static mws_sp<mws_slider> nwi();
   void set_value(float i_value);
   float get_value() const { return value; }
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual bool is_hit(float x, float y, bool& i_ball_hit, bool& i_bar_hit);
   virtual mws_sp<gfx_vxo> get_bar_vxo() const;
   virtual mws_sp<gfx_vxo> get_ball_vxo() const;

   std::function<void()> on_drag_handler;

protected:
   mws_slider();
   void setup() override;

   float value;
   mws_sp<gfx_vxo> slider_bar;
   mws_sp<gfx_vxo> slider_ball;
   bool active;
   dragging_detector dragging_dt;
};


class mws_list_model : public mws_model
{
public:
   mws_list_model();
   virtual uint32_t get_length() = 0;
   virtual std::string elem_at(uint32_t i_idx) = 0;
   virtual uint32_t get_selected_elem();
   virtual void set_selected_elem(uint32_t i_selected_elem);
   virtual void on_elem_selected(uint32_t i_idx) = 0;

protected:
   uint32_t selected_elem;
};


class mws_list : public mws_page_item
{
public:
   static mws_sp<mws_list> nwi();
   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual bool is_hit(float x, float y);
   virtual void update_state();
   void set_model(mws_sp<mws_list_model> imodel);
   mws_sp<mws_list_model> get_model();

protected:
   mws_list();
   void setup() override;
   uint32_t element_at(float x, float y);

   float item_height;
   float vertical_space;
   float item_w;
   float item_x;
   mws_sp<mws_list_model> model;
};


class mws_tree_model_node
{
public:
   mws_tree_model_node(const std::string& i_data) : data(i_data) {}
   virtual ~mws_tree_model_node() {}

   std::vector<mws_sp<mws_tree_model_node>> nodes;
   std::string data;
};


class mws_tree_model : public mws_model
{
public:
   mws_tree_model();
   virtual void set_length(uint32_t i_length);
   virtual uint32_t get_length();
   virtual void set_root_node(mws_sp<mws_tree_model_node> i_root);
   virtual mws_sp<mws_tree_model_node> get_root_node();

protected:
   uint32_t length;
   mws_sp<mws_tree_model_node> root;
};


class mws_tree : public mws_page_item
{
public:
   static mws_sp<mws_tree> nwi();
   virtual void init() override;
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual void update_view(mws_sp<mws_camera> i_g) override;
   void set_model(mws_sp<mws_tree_model> i_model);
   mws_sp<mws_tree_model> get_model();

protected:
   mws_tree() {}
   void setup() override;
   void get_max_width(mws_sp<mws_font> i_f, const mws_sp<mws_tree_model_node> i_node, uint32_t i_level, float& i_max_width);
   void draw_tree_elem(mws_sp<mws_camera> i_g, const mws_sp<mws_tree_model_node> i_node, uint32_t i_level, uint32_t& i_elem_idx);

   mws_sp<mws_tree_model> model;
   double_tap_detector dbl_tap_det;
};


class mws_text_area_model
{
public:
   // common
   virtual bool get_word_wrap() = 0;
   virtual void set_word_wrap(bool i_word_wrap) = 0;
   virtual uint32_t get_line_count() = 0;
   virtual std::string get_line_at(uint32_t i_idx, bool i_keep_line_break = true) = 0;
   virtual std::vector<std::string> get_lines_at(uint32_t i_idx, uint32_t i_line_count, bool i_keep_line_break = true) = 0;
   virtual const std::string& get_text() = 0;
   virtual void set_text(const std::string& i_text) = 0;
   virtual void set_text(const char* i_text, uint32_t i_length) = 0;
   virtual void set_size(uint32_t i_width, uint32_t i_height) = 0;
   virtual void set_font(mws_sp<mws_font> i_font) = 0;
   virtual uint32_t get_char_at_pixel(float i_x, float i_y) = 0;

   // viewer only
   virtual void push_back(const char* i_text, uint32_t i_length) = 0;
   virtual void push_front(const char* i_text, uint32_t i_length) = 0;

   // editor only
   virtual void insert_at_cursor(const std::string& i_text) = 0;
   virtual void delete_at_cursor(int32_t i_count) = 0;
   virtual uint32_t get_cursor_pos() = 0;
   virtual void set_cursor_pos(uint32_t i_cursor_pos) = 0;
   virtual uint32_t get_cursor_pos_at_line(uint32_t i_line_idx) = 0;
   // returns [ horizontal, vertical ] position of the cursor
   virtual glm::uvec2 get_cursor_coord() = 0;
   virtual void advance_cursor(mws_dir_types i_direction) = 0;
};
