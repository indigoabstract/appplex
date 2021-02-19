#pragma once

// mws - multi-widget-set (as in, opposed to, say, a one-widget-set :) )
#include "pfm.hxx"
#include "util/util.hxx"
#include "input/input-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-scene.hxx"
mws_push_disable_all_warnings
#include <glm/fwd.hpp>
#include <string>
#include <variant>
#include <vector>
mws_pop_disable_all_warnings

class mws_mod_ctrl;
class linear_transition;
class ms_linear_transition;
class mws_mod;
class mws_mod;
class mws_obj;
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
class mws_text_vxo;
struct vkb_file_info;


const std::string MWS_EVT_MODEL_UPDATE = "mws-model-update";
const std::string MWS_EVT_PAGE_TRANSITION = "mws-page-transition";


// contains a relative(to parent) dimension
class mws_rel_dim
{
public:
   mws_rel_dim(float i_size = 0.f) { size = i_size; }
   float get_ratio() const { return size; }

protected:
   float size = 0.f;
};


using mws_size_elem = std::variant<std::monostate, mws_px, mws_rel_dim>;
class mws_size
{
public:
   static mws_size nwi_px(float i_width_px, float i_height_px)
   {
      return mws_size(mws_size_elem(mws_px(i_width_px, mws_dim::e_horizontal)), mws_size_elem(mws_px(i_height_px, mws_dim::e_vertical)));
   };

   mws_size() {}
   mws_size(const mws_size_elem& i_width, const mws_size_elem& i_height) : width(i_width), height(i_height) {}

   mws_size_elem get_width() const { return width; }
   void set_width(const mws_size_elem& i_width) { width = i_width; }
   mws_size_elem get_height() const { return height; }
   void set_height(const mws_size_elem& i_height) { height = i_height; }

protected:
   mws_size_elem width;
   mws_size_elem height;
};


class mws_rect
{
public:
   mws_rect() {}
   mws_rect(float i_x, float i_y, float i_w, float i_h) { set(i_x, i_y, i_w, i_h); }
   void set(float i_x, float i_y, float i_w, float i_h) { x = i_x; y = i_y; w = i_w; h = i_h; }

   float x = 0.f;
   float y = 0.f;
   float w = 0.f;
   float h = 0.f;
};


class mws_model : public std::enable_shared_from_this<mws_model>, public mws_node
{
public:
   virtual ~mws_model() {}
   mws_sp<mws_model> get_instance();

   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual void notify_update();
   void set_view(mws_sp<mws_obj> iview);
   mws_sp<mws_obj> get_view();

protected:
   mws_model() {}

   mws_sp<mws_sender> sender_inst();

   mws_wp<mws_obj> view;
};


class mws_obj : public gfx_node, public mws_node
{
public:
   static mws_sp<mws_obj> nwi();
   virtual ~mws_obj() {}
   virtual void init() {}
   virtual void on_destroy() {}
   mws_sp<mws_obj> get_instance();
   virtual gfx_obj::e_gfx_obj_type get_type()const override;
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo>>& i_opaque, std::vector<mws_sp<gfx_vxo>>& i_translucent) override;
   virtual void attach(mws_sp<gfx_node> i_node) override;
   virtual void list_mws_children(std::vector<mws_sp<mws_obj>>& i_mws_subobj_list);
   virtual void set_enabled(bool i_is_enabled);
   bool is_enabled()const;
   void set_visible(bool i_is_visible);
   bool is_visible()const;
   void set_id(std::string i_id);
   const std::string& get_id();
   virtual mws_sp<mws_obj> find_by_id(const std::string& i_id);
   virtual mws_sp<mws_obj> contains_id(const std::string& i_id);
   virtual bool contains_mws(const mws_sp<mws_obj> i_mws);
   mws_sp<mws_obj> get_mws_parent() const;
   mws_sp<mws_page_tab> get_mws_root() const;
   virtual mws_sp<mws_mod> get_mod() const;
   virtual void on_focus_changed(bool /*i_has_focus*/) {}
   virtual void process(mws_sp<mws_dp> i_dp);
   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   virtual const mws_size& get_best_size() const;
   virtual void set_best_size(const mws_size& i_size);
   virtual mws_rect get_pos();
   virtual float get_z();
   virtual void set_z(float i_z_position);

   std::function<void(mws_sp<mws_dp> i_idp)> receive_handler;

protected:
   mws_obj(mws_sp<gfx> i_gi = nullptr);
   virtual void setup() {}
   virtual mws_sp<mws_sender> sender_inst();

   bool enabled = true;
   bool is_opaque = true;
   mws_rect mws_r;
   mws_wp<mws_page_tab> mwsroot;
   mws_wp<mws_camera> mws_cam;
   mws_size best_size;
   std::string id;
};


class mws_vkb_file_store
{
public:
   virtual ~mws_vkb_file_store() {}
   virtual std::vector<vkb_file_info> get_vkb_list() = 0;
   virtual bool file_exists(const std::string& i_vkb_filename) = 0;
   virtual void save_vkb(const std::string& i_vkb_filename, const std::string& i_data) = 0;
   virtual std::string load_vkb(const std::string& i_vkb_filename) = 0;
};


class mws_virtual_keyboard : public mws_obj
{
public:
   virtual ~mws_virtual_keyboard() {}
   virtual mws_key_types apply_key_modifiers(mws_key_types i_key_id) const = 0;
   virtual void on_resize() = 0;
   // directs the keyboard output to the target text_area
   virtual void set_target(mws_sp<mws_text_area> i_ta) = 0;
   virtual mws_sp<mws_font> get_font() = 0;
   virtual void set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt) = 0;
   virtual mws_sp<mws_vkb_file_store> get_file_store() const = 0;
   virtual void set_file_store(mws_sp<mws_vkb_file_store> i_store) = 0;
   virtual std::vector<mws_sp<gfx_tex>> get_tex_list() = 0;
   // explicitly loads the keyboard in a blocking or non-blocking way
   virtual void load(bool i_blocking_load = false) = 0;
   // if this returns true, then the keyboard needs to be (re)loaded first, which means a (potentially long) loading wait time
   virtual bool upcoming_loading_wait() = 0;

protected:
   mws_virtual_keyboard() {}
};


/** page navigator base class */
class mws_page_nav
{
public:
   virtual ~mws_page_nav() {}
   virtual std::string get_main_page_id() const = 0;
   virtual void set_main_page_id(const std::string& i_main_page_id) = 0;
   virtual void pop() = 0;
   virtual void push(std::string i_page_id) = 0;
   virtual void reset_pages() = 0;
};


class mws_page_tab : public mws_obj
{
public:
   virtual ~mws_page_tab() {}
   static mws_sp<mws_page_tab> nwi(mws_sp<mws_mod> i_mod);
   virtual void add_to_draw_list(const std::string& i_cam_id, std::vector<mws_sp<gfx_vxo>>& i_opaque, std::vector<mws_sp<gfx_vxo>>& i_translucent) override;
   virtual void init();
   virtual void init_subobj();
   virtual void on_destroy();

   virtual mws_sp<mws_obj> contains_id(const std::string& i_id);
   virtual bool contains_mws(const mws_sp<mws_obj> i_mws);
   mws_sp<mws_page_tab> get_mws_page_tab_instance();
   virtual mws_sp<mws_mod> get_mod();
   bool is_empty();
   mws_sp<mws_text_vxo> get_text_vxo() const;

   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   virtual void on_resize();
   virtual void add_page(mws_sp<mws_page> i_page);
   template <typename T, typename... var_args> mws_sp<T> new_page(var_args... i_args) { mws_sp<T> p(new T(i_args...)); add_page(p); return p; }
   // returns true if it handled(consumed) the back event, false otherwise
   virtual bool handle_back_evt();
   virtual mws_sp<mws_virtual_keyboard> get_keyboard();
   virtual void show_keyboard(mws_sp<mws_text_area> i_tbx);
   virtual mws_sp<mws_vkb_file_store> get_file_store();
   virtual void set_file_store(mws_sp<mws_vkb_file_store> i_store) { vkb_store = i_store; }
   virtual mws_sp<mws_page_nav> get_page_nav() const { return page_nav; };
   virtual void set_page_nav(mws_sp<mws_page_nav> i_page_nav) { mws_assert(i_page_nav != nullptr); page_nav = i_page_nav; };

   std::vector<mws_sp<mws_page>> page_tab;

protected:
   friend class mws_mod_ctrl;
   friend class mws_page;
   mws_page_tab(mws_sp<mws_mod> i_mod);
   void add(mws_sp<mws_page> p);
   int get_page_index(mws_sp<mws_page> p);
   void new_instance_helper();

   mws_sp<mws_virtual_keyboard> vkb;
   mws_sp<mws_vkb_file_store> vkb_store;
   mws_sp<mws_text_vxo> tab_text_vxo;
   mws_wp<mws_mod> mod;
   mws_sp<mws_page_nav> page_nav;
};


class mws_page_item : public mws_obj
{
public:
   virtual void set_rect(const mws_rect& i_rect);
   virtual void set_position(const glm::vec2& i_position);
   virtual void set_size(const glm::vec2& i_size);
   mws_sp<mws_page> get_page();
   virtual bool has_focus();
   virtual void set_focus(bool i_set_focus = true);

protected:
   mws_page_item();
   void setup() override;
};


class mws_layout : public mws_page_item
{
public:
   virtual ~mws_layout() {}
   virtual void on_resize() {}
};


struct mws_layout_info
{
   uint32_t width = 0;
   uint32_t height = 0;
   mws_sp<mws_layout> layout;
   float32 aspect_ratio = 0.f;
};


/** represents a single ui page or a screen */
class mws_page : public mws_obj
{
public:
   virtual ~mws_page() {}
   static mws_sp<mws_page> nwi(mws_sp<mws_page_tab> i_parent);
   virtual void init();
   virtual void on_destroy();

   virtual mws_sp<mws_obj> contains_id(const std::string& i_id);
   virtual bool contains_mws(const mws_sp<mws_obj> i_mws);
   mws_sp<mws_page> get_mws_page_instance();
   mws_sp<mws_page_tab> get_mws_page_parent() const;
   virtual mws_layout_info find_closest_layout_match(const mws_layout_info& i_lyt_info);
   virtual void add_layout(mws_layout_info& i_lyt_info);

   virtual void on_visibility_changed_handler(bool i_is_visible) override;
   virtual void on_show_transition(const mws_sp<linear_transition> i_transition);
   virtual void on_hide_transition(const mws_sp<linear_transition> i_transition);

   virtual void receive(mws_sp<mws_dp> i_dp);
   virtual void update_input_sub_mws(mws_sp<mws_dp> i_dp);
   virtual void update_input_std_behaviour(mws_sp<mws_dp> i_dp);
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   virtual glm::vec2 get_dim() const;
   mws_sp<mws_obj> get_mws_at(uint32_t i_idx);
   virtual bool is_selected(mws_sp<mws_obj> i_item);
   virtual void set_focus(mws_sp<mws_obj> i_item, bool i_set_focus = true);

protected:
   friend class mws_page_tab;
   friend class mws_page_item;
   mws_page();
   virtual void on_resize();
   std::vector<mws_sp<mws_obj>> mws_subobj_list;
   mws_sp<mws_obj> selected_item;
   std::vector<mws_layout_info> layouts;
   mws_layout_info crt_layout;
};


class mws_text_area : public mws_page_item, public mws_text_buffer
{
public:
   enum cursor_types
   {
      e_left_cursor,
      e_middle_vertical_cursor,
      e_middle_cursor,
      e_right_cursor,
      e_left_right_cursor,
   };

   virtual ~mws_text_area() {}
   virtual void do_action() {}
   virtual bool is_action_key(mws_key_types /*i_key*/) const { return false; }
   virtual mws_rect get_cursor_rect(cursor_types i_cursor_type, bool i_absolute_pos = true) = 0;
   virtual void set_text(const std::string& i_text) = 0;

protected:
   mws_text_area() {}
};
