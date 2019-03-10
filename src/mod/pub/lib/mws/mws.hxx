#pragma once

// mws - multi-widget-set (as in, opposed to, say, a one-widget-set :) )
#include "pfm.hxx"
#include "util/util.hxx"
#include "input/input-ctrl.hxx"
#include "gfx-scene.hxx"
#include <string>
#include <vector>


class mws_mod_ctrl;
class linear_transition;
class ms_linear_transition;
class mws_mod;
class mws_mod;
class mws;
class mws_page_tab;
class mws_page;
class mws_page_item;
class mws_button;
class mws_list;
class mws_tree;
class mws_font;
class mws_camera;
class mws_text_area;
class mws_virtual_keyboard;
class gfx_vxo;
class text_vxo;


const std::string MWS_EVT_MODEL_UPDATE = "mws-model-update";
const std::string MWS_EVT_PAGE_TRANSITION = "mws-page-transition";


class mws_rect
{
public:
   mws_rect();
   mws_rect(float i_x, float i_y, float i_w, float i_h);
   void set(float i_x, float i_y, float i_w, float i_h);

   float x = 0.f;
   float y = 0.f;
   float w = 0.f;
   float h = 0.f;
};


class mws_model : public enable_shared_from_this<mws_model>, public mws_node
{
public:
   virtual ~mws_model() {}
   mws_sp<mws_model> get_instance();

   virtual void receive(mws_sp<mws_dp> idp);
   virtual void notify_update();
   void set_view(mws_sp<mws> iview);
   mws_sp<mws> get_view();

protected:
   mws_model() {}

   mws_sp<mws_sender> sender_inst();

   mws_wp<mws> view;
};


class mws : public gfx_node, public mws_node
{
public:
   static mws_sp<mws> nwi();

   virtual void init() {}
   virtual void on_destroy() {}
   virtual ~mws() {}
   mws_sp<mws> get_instance();

   virtual gfx_obj::e_gfx_obj_type get_type()const override;
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent) override;
   virtual void attach(mws_sp<gfx_node> i_node) override;
   virtual void list_mws_children(std::vector<mws_sp<mws> >& i_mws_subobj_list);
   virtual void set_enabled(bool i_is_enabled);
   bool is_enabled()const;
   void set_visible(bool i_is_visible);
   bool is_visible()const;
   void set_id(std::string iid);
   const std::string& get_id();
   virtual mws_sp<mws> find_by_id(const std::string& iid);
   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws> get_mws_parent();
   mws_sp<mws_page_tab> get_mws_root();
   virtual mws_sp<mws_mod> get_mod();

   virtual void on_focus_changed(bool i_has_focus) {}
   virtual void receive(mws_sp<mws_dp> idp);
   virtual void set_receive_handler(std::function<void(mws_sp<mws> i_mws, mws_sp<mws_dp> i_idp)> i_receive_handler);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   mws_rect get_pos();
   virtual float get_z();
   virtual void set_z(float i_z_position);

protected:
   mws(mws_sp<gfx> i_gi = nullptr);
   virtual void setup() {}

   bool enabled = true;
   bool is_opaque = true;
   mws_rect mws_r;
   mws_wp<mws_page_tab> mwsroot;
   mws_wp<mws_camera> mws_cam;
   std::function<void(mws_sp<mws> i_mws, mws_sp<mws_dp> i_idp)> receive_handler;

private:
   virtual mws_sp<mws_sender> sender_inst();

   std::string id;
};


class mws_virtual_keyboard : public mws
{
public:
   virtual ~mws_virtual_keyboard() {}
   virtual void on_resize() = 0;
   virtual void set_target(mws_sp<mws_text_area> i_ta) = 0;
   virtual void set_font(mws_sp<mws_font> i_fnt) = 0;

protected:
   mws_virtual_keyboard() {}
};


class mws_page_tab : public mws
{
public:
   virtual ~mws_page_tab() {}
   static mws_sp<mws_page_tab> nwi(mws_sp<mws_mod> i_mod);
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent) override;
   virtual void init();
   virtual void init_subobj();
   virtual void on_destroy();

   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws_page_tab> get_mws_page_tab_instance();
   virtual mws_sp<mws_mod> get_mod();
   bool is_empty();
   mws_sp<text_vxo> get_text_vxo() const;

   virtual void receive(mws_sp<mws_dp> idp);
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   virtual void on_resize();
   virtual void add_page(mws_sp<mws_page> i_page);
   template <typename T> mws_sp<T> new_page() { mws_sp<T> p(new T()); add_page(p); return p; }
   // returns true if it handled(consumed) the back event, false otherwise
   virtual bool handle_back_evt();
   virtual mws_sp<mws_virtual_keyboard> get_keyboard();
   virtual void show_keyboard(mws_sp<mws_text_area> i_tbx);

   std::vector<mws_sp<mws_page> > page_tab;

protected:
   mws_page_tab(mws_sp<mws_mod> i_mod);

private:
   friend class mws_mod_ctrl;
   friend class mws_page;

   void add(mws_sp<mws_page> p);
   int get_page_index(mws_sp<mws_page> p);
   void new_instance_helper();

   mws_sp<mws_virtual_keyboard> vkb;
   mws_sp<text_vxo> tab_text_vxo;
   mws_wp<mws_mod> u;
};


class mws_page : public mws
{
public:
   virtual ~mws_page() {}
   static mws_sp<mws_page> nwi(mws_sp<mws_page_tab> iparent);
   virtual void init();
   virtual void on_destroy();

   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws_page> get_mws_page_instance();
   mws_sp<mws_page_tab> get_mws_page_parent();

   virtual void on_visibility_changed(bool iis_visible);
   virtual void on_show_transition(const mws_sp<linear_transition> itransition);
   virtual void on_hide_transition(const mws_sp<linear_transition> itransition);

   virtual void receive(mws_sp<mws_dp> idp);
   virtual void update_input_sub_mws(mws_sp<mws_dp> idp);
   virtual void update_input_std_behaviour(mws_sp<mws_dp> idp);
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   mws_sp<mws> get_mws_at(int idx);
   virtual bool is_selected(mws_sp<mws> i_item);
   virtual void select(mws_sp<mws> i_item);

protected:
   mws_page();
   virtual void on_resize();
   std::vector<mws_sp<mws> > mws_subobj_list;
   mws_sp<mws> selected_item;

private:
   friend class mws_page_tab;
   friend class mws_page_item;

   static mws_sp<mws_page> new_standalone_instance();
   void add(mws_sp<mws_page_item> b);

   std::vector<mws_sp<mws_page_item> > mlist;
};


class mws_page_item : public mws
{
public:
   virtual ~mws_page_item() {}

   virtual void set_rect(const mws_rect& i_rect);
   virtual void set_size(float i_width, float i_height);
   mws_sp<mws_page> get_mws_page_item_parent();
   virtual bool has_focus();
   virtual void select();

protected:
   mws_page_item();
   void setup() override;

   void add_to_page();
};


class mws_text_area : public mws_page_item
{
public:
   virtual ~mws_text_area() {}
   virtual void do_action() {}
   virtual bool is_action_key(key_types i_key) const { return false; }

protected:
   mws_text_area() {}
};
