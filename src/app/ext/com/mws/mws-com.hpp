#pragma once

#include "mws.hpp"
#include "com/unit/gesture-detectors.hpp"
#include "gfx-color.hpp"


class gfx_quad_2d;
class mws_font;


class mws_panel : public mws_page_item
{
public:
   static std::shared_ptr<mws_panel> nwi();
   virtual ~mws_panel() {}
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual mws_sp<gfx_quad_2d> get_vxo();

protected:
   mws_panel() {}
   void setup() override;

   mws_sp<gfx_quad_2d> vxo;
};


class mws_img_btn : public mws_page_item
{
public:
   static std::shared_ptr<mws_img_btn> nwi();
   virtual ~mws_img_btn() {}
   void set_enabled(bool i_is_enabled) override;
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void set_img_name(std::string i_img_name);
   virtual void receive(shared_ptr<iadp> idp) override;
   virtual bool is_hit(float x, float y);
   virtual void on_click();
   virtual mws_sp<gfx_quad_2d> get_vxo();

   std::function<void(mws_sp<mws_img_btn> i_img_btn)> on_click_handler;

protected:
   mws_img_btn() {}
   void setup() override;

   mws_sp<gfx_quad_2d> vxo;
};


class mws_button : public mws_page_item
{
public:
   virtual ~mws_button() {}
   static shared_ptr<mws_button> nwi();
   void set_enabled(bool i_is_enabled);
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void receive(shared_ptr<iadp> idp);
   virtual bool is_hit(float x, float y);
   virtual void on_click();
   virtual void update_state() override;
   virtual void set_text(std::string i_text);
   virtual void set_text_visible(bool i_visible) { text_visible = i_visible; }
   virtual void set_bg_color(const gfx_color& i_color);
   virtual void set_bg_visible(bool i_visible);
   virtual void set_font(mws_sp<mws_font> i_font);
   virtual mws_sp<gfx_quad_2d> get_vxo();

   std::function<void(mws_sp<mws_button> i_btn)> on_click_handler;

protected:
   mws_button() {}
   void setup() override;

   std::string text;
   bool text_visible = true;
   gfx_color color;
   shared_ptr<mws_font> font;
   mws_sp<gfx_quad_2d> vxo;
};


class mws_slider : public mws_page_item
{
public:
   static std::shared_ptr<mws_slider> nwi();
   virtual ~mws_slider() {}
   void set_value(float i_value);
   float get_value() const { return value; }
   virtual void set_rect(const mws_rect& i_rect) override;
   virtual void receive(shared_ptr<iadp> idp) override;
   virtual bool is_hit(float x, float y, bool& i_ball_hit, bool& i_bar_hit);
   virtual mws_sp<gfx_vxo> get_bar_vxo() const;
   virtual mws_sp<gfx_vxo> get_ball_vxo() const;

   std::function<void(mws_sp<mws_slider> i_slider)> on_drag_handler;

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
   virtual ~mws_list_model() {}

   virtual int get_length() = 0;
   virtual std::string elem_at(int idx) = 0;
   virtual int get_selected_elem();
   virtual void set_selected_elem(int iselectedElem);
   virtual void on_elem_selected(int idx) = 0;

protected:
   int selected_elem;
};


class mws_list : public mws_page_item
{
public:
   virtual ~mws_list() {}
   static shared_ptr<mws_list> nwi(shared_ptr<mws_page> iparent);
   static shared_ptr<mws_list> new_shared_instance(shared_ptr<mws_page> i_parent, mws_list* new_instance);
   virtual void init();

   virtual void receive(shared_ptr<iadp> idp);
   virtual bool is_hit(float x, float y);

   virtual void update_state();
   virtual void update_view(shared_ptr<mws_camera> g);
   void set_model(shared_ptr<mws_list_model> imodel);
   shared_ptr<mws_list_model> get_model();

protected:
   mws_list(shared_ptr<mws_page> iparent);
   void setup() override;

   int element_at(float x, float y);

   float item_height;
   float vertical_space;
   float item_w;
   float item_x;
   shared_ptr<mws_list_model> model;
};


class mws_tree_model_node
{
public:
   mws_tree_model_node(const std::string& idata) : data(idata) {}
   virtual ~mws_tree_model_node() {}

   std::vector<shared_ptr<mws_tree_model_node> > nodes;
   std::string data;
};


class mws_tree_model : public mws_model
{
public:
   mws_tree_model();
   virtual ~mws_tree_model() {}

   virtual void set_length(int ilength);
   virtual int get_length();
   virtual void set_root_node(shared_ptr<mws_tree_model_node> iroot);
   virtual shared_ptr<mws_tree_model_node> get_root_node();

protected:
   int length;
   shared_ptr<mws_tree_model_node> root;
};


class mws_tree : public mws_page_item
{
public:
   virtual ~mws_tree() {}
   static shared_ptr<mws_tree> nwi(shared_ptr<mws_page> iparent);
   static shared_ptr<mws_tree> new_shared_instance(mws_tree* newTreeClassInstance);
   virtual void init();

   virtual void receive(shared_ptr<iadp> idp);

   virtual void update_state();
   virtual void update_view(shared_ptr<mws_camera> g);
   void set_model(shared_ptr<mws_tree_model> imodel);
   shared_ptr<mws_tree_model> get_model();

protected:
   mws_tree(shared_ptr<mws_page> iparent);
   void setup() override;

   void get_max_width(shared_ptr<mws_font> f, const shared_ptr<mws_tree_model_node> node, int level, float& maxWidth);
   void draw_tree_elem(shared_ptr<mws_camera> g, const shared_ptr<mws_tree_model_node> node, int level, int& elemIdx);

   shared_ptr<mws_tree_model> model;
};


class text_area_model
{
public:
   virtual int get_line_count() = 0;
   virtual std::string get_line_at(int iidx) = 0;
   virtual std::vector<std::string> get_lines_at(int iidx, int iline_count) = 0;
};
