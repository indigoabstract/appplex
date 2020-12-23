#pragma once

#include "mws/mws.hxx"
#include "input/gesture-detectors.hxx"
#include "gfx-pbo.hxx"


class mws_draw_text;
class mws_font;
class mws_mod;
class mws_vrn_main;
class mws_text_vxo;
class gfx_tex;
class mws_vrn_kernel_pt_vect;


const std::string VKB_PREFIX = "vkb-";
const std::string VKB_EXT = ".kxmd";

// mod keys
const mws_key_types VKB_ALT = mws_key_alt;
const mws_key_types VKB_HIDE_KB = mws_key_f1;
const mws_key_types VKB_SHIFT = mws_key_shift;
// special keys
const mws_key_types VKB_ESCAPE = mws_key_escape;
const mws_key_types VKB_DONE = mws_key_end;
const mws_key_types VKB_ENTER = mws_key_enter;
const mws_key_types VKB_BACKSPACE = mws_key_backspace;
const mws_key_types VKB_DELETE = mws_key_delete;
const mws_key_types VKB_NEXT_SCREEN = mws_key_f2;
const mws_key_types VKB_PREV_SCREEN = mws_key_f3;


enum class key_mod_types
{
   // base keys (the vkb's 'physical' keys). their values are used when there are no active modifier keys
   mod_none = 0,
   // keys visible when alt is pressed
   mod_alt,
   // keys visible when shift is pressed
   mod_shift,
   count,
};


struct vkb_info
{
   uint32_t width = 0;
   uint32_t height = 0;
   // file index for this aspect ratio
   uint32_t index = 0;
   float aspect_ratio = 0.f;
};


struct vkb_file_info
{
   vkb_info info;
   mws_sp<mws_file> file;
};


class mws_vkb_file_store_impl : public mws_vkb_file_store
{
public:
   mws_vkb_file_store_impl(mws_sp<mws_mod> i_mod);

   std::vector<vkb_file_info> get_vkb_list() override;
   bool file_exists(const std::string& i_vkb_filename) override;
   void save_vkb(const std::string& i_vkb_filename, const std::string& i_data) override;
   std::string load_vkb(const std::string& i_vkb_filename) override;

protected:
   std::vector<vkb_file_info> vkb_info_vect;
   mws_wp<mws_mod> mod;
};


class mws_vkb_impl : public mws_obj
{
public:
   static inline const float key_lights_off_seconds = 2.5f;
   static inline const float pressed_key_lights_hold_seconds = 0.3f;
   static inline const float pressed_key_lights_off_seconds = 0.8f;

   mws_vkb_impl(uint32_t i_obj_type_mask);
   static vkb_info get_vkb_info(const std::string& i_filename);
   static std::string get_vkb_filename(uint32_t i_map_idx);
   virtual void setup() override;
   virtual void receive(mws_sp<mws_dp> i_dp) override {}
   virtual void update_state() override {}
   virtual void update_view(mws_sp<mws_camera> g) override {}
   virtual mws_sp<mws_ptr_evt> on_receive(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta);
   virtual void on_update_state();
   virtual void on_resize(uint32_t i_width, uint32_t i_height);
   virtual vkb_file_info get_closest_vkb_match(uint32_t i_width, uint32_t i_height);
   virtual mws_sp<mws_font> get_font() const;
   virtual void set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt);
   virtual void start_anim();
   virtual void done();
   // returns keyboard dimensions
   virtual glm::ivec2 get_dimensions() const;
   virtual std::string get_key_name(mws_key_types i_key_id) const;
   virtual mws_key_types get_key_type(const std::string& i_key_name) const;
   void load_map(std::string i_filename);
   virtual std::vector<mws_key_types>& get_key_vect();
   virtual uint32_t get_key_vect_size();
   // returns the key id at position i_idx
   virtual mws_key_types get_key_at(int i_idx);
   // returns the key id at position 'i_idx' in mod 'i_key_mod'
   virtual mws_key_types get_mod_key_at(key_mod_types i_key_mod, int i_idx);
   virtual void next_page();
   virtual void prev_page();
   virtual void set_on_top();
   virtual std::vector<mws_sp<gfx_tex>> get_tex_list();
   virtual void build_cell_border_tex();
   virtual void build_keys_tex();
   virtual void show_pressed_key(const mws_sp<mws_text_area> i_ta, uint32_t i_key_idx);
   virtual mws_sp<mws_font> get_key_font() const { return letter_font; }
   virtual bool is_mod_key(mws_key_types i_key_id);
   // check if i_mod_key is pressed or locked
   enum class mod_key_types { alt = VKB_ALT, hide_vkb = VKB_HIDE_KB, shift = VKB_SHIFT, };
   enum class base_key_state_types { key_free, key_held, key_locked, };
   // returns true if i_mod_key is either held or locked. you can use i_state to find out the actual state of i_mod_key
   virtual bool is_mod_key_held(mod_key_types i_mod_key, base_key_state_types* i_state = nullptr, int* i_key_idx = nullptr) const;
   virtual void set_mod_key_lock(mod_key_types i_mod_key, bool i_set_lock = true);
   virtual void clear_mod_key_locks();
   // release all held and locked keys (optional)
   virtual void release_all_keys(bool i_release_locked_keys = true);

   int pressed_key_ker_idx = -1;
   mws_sp<mws_vkb_file_store> file_store;
   mws_sp<mws_vrn_main> vk;
   std::string loaded_filename;

protected:
   class res_specific_params
   {
   public:
      // keyboard resolution, in pixels (it's the same for both landscape or portrait keyboards, just put the largest value)
      uint32_t resolution_px = 0;
      // weight factors for kawase bloom filter on the keys
      std::vector<float> key_weight_fact;
   };


   class mws_vkb_pressed_key : public gfx_node
   {
   public:
      mws_vkb_pressed_key();
      void init(mws_sp<gfx_tex> i_cell_border_tex);
      void show_pressed_key(const mws_sp<mws_text_area> i_ta, uint32_t i_key_idx, mws_sp<mws_vrn_main> i_vk, mws_sp<gfx_tex> i_keys_tex);
      void start_light_turnoff();
      bool is_fading(uint32_t i_crt_time);
      void set_fade_gradient(float i_gradient);

      glm::vec4 arrow_color = glm::vec4(0.9f, 0.08f, 1.f, 0.7f);
      glm::vec4 label_bg_color = glm::vec4(0.f, 0.f, 0.f, 0.93f);
      mws_sp<gfx_vxo> arrow;
      mws_sp<gfx_vxo> label;
      mws_sp<gfx_vxo> label_bg;
      mws_sp<gfx_vxo> border;
      uint32_t light_turnoff_start = 0;
   };


   virtual res_specific_params get_closest_resolution_match(uint32_t i_resolution_px);
   virtual void setup_font_dimensions();
   virtual void init_shaders();
   virtual void set_key_transparency(float i_alpha);
   virtual void draw_keys(mws_sp<mws_draw_text> i_dt, mws_sp<mws_font> i_letter_font, mws_sp<mws_font> i_word_font, key_mod_types i_mod, mws_vrn_kernel_pt_vect& i_kp_vect);
   virtual void set_key_vect_size(uint32_t i_size);
   virtual void set_key_at(int i_idx, mws_key_types i_key_id);
   virtual void erase_key_at(int i_idx);
   virtual void push_back_key(mws_key_types i_key_id);
   virtual bool touch_began(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_moved(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_ended(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_cancelled(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual void highlight_key_at(int i_idx, bool i_light_on = true);
   virtual void fade_key_at(int i_idx);
   // set the state of the keys. if return value is true, the iterators are invalidated and need to be aborted
   // returns true when keyboard has been hidden (and the key state cleared), false otherwise
   virtual bool set_key_state(int i_key_idx, base_key_state_types i_state);
   // call this after modifying / inserting / deleting a base key (a mod_node key)
   virtual void rebuild_key_state();

   bool build_textures = true;
   uint32_t obj_type_mask = 0;
   glm::ivec2 vkb_dim = glm::ivec2(0);
   glm::vec2 diag_original_dim = glm::vec2(0.f);
   key_mod_types key_mod = key_mod_types::mod_none;
   std::vector<mws_key_types> key_mod_vect[(uint32_t)key_mod_types::count];
   std::unordered_map<mws_key_types, std::string> key_map;
   int map_idx = 0;
   uint32_t crt_page_idx = 0;
   struct key_highlight { int key_idx; uint32_t release_time; };
   std::vector<key_highlight> highlight_vect;
   double_tap_detector dbl_tap_det;
   struct base_key_state { mws_key_types key_id; base_key_state_types state; uint8_t pressed_count; };
   std::vector<base_key_state> base_key_st;
   mws_sp<mws_ptr_evt> prev_ptr_evt;
   std::unordered_map<int, mws_key_types> mod_keys_st;
   const float fade_duration_in_seconds = 0.85f;
   basic_time_slider<float> fade_slider;
   enum class fade_types { e_none, e_hide_vkb, e_show_vkb };
   fade_types fade_type = fade_types::e_none;
   mws_sp<mws_font> letter_font;
   mws_sp<mws_font> word_font;
   mws_sp<gfx_tex> cell_border_tex;
   mws_gfx_ppb key_border_tex;
   mws_sp<gfx_quad_2d> key_border_quad;
   std::vector<mws_gfx_ppb> keys_tex;
   mws_sp<gfx_quad_2d> keys_bg_outline_quad;
   mws_sp<gfx_quad_2d> keys_quad;
   mws_sp<mws_vkb_pressed_key> pressed_key;
   uintptr_t pressed_vkb_hide_finger_id = -1;
   inline static const std::string vkb_keys_fonts_sh = "mws-vkb-keys-fonts";
   inline static const std::string vkb_keys_outline_sh = "mws-vkb-keys-outline";
   inline static const std::string vkb_hsv_shift_sh = "mws-vkb-hsv-shift";
   mws_sp<gfx_shader> vkb_keys_fonts_shader;
   mws_sp<gfx_shader> vkb_keys_outline_shader;
   mws_sp<gfx_shader> vkb_hsv_shift_shader;
   inline static std::vector<res_specific_params> resolution_params;
};


class mws_vkb : public mws_virtual_keyboard
{
public:
   static mws_sp<mws_vkb> gi();
   virtual ~mws_vkb() {}
   // i_key_id is not actually a key id, but a position index for the points on the keyboard
   virtual mws_key_types apply_key_modifiers(mws_key_types i_key_id) const override;
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual void update_state() override;
   virtual void on_resize() override;
   virtual void set_target(mws_sp<mws_text_area> i_tbx) override;
   virtual mws_sp<mws_font> get_font() override;
   virtual void set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt) override;
   virtual mws_sp<mws_vkb_file_store> get_file_store() const override;
   virtual void set_file_store(mws_sp<mws_vkb_file_store> i_store) override;
   virtual std::vector<mws_sp<gfx_tex>> get_tex_list() override;
   virtual void load(bool i_blocking_load = false) override;
   virtual bool upcoming_loading_wait() override;

protected:
   mws_vkb();
   virtual void setup() override;
   virtual void update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx) override;
   // when finished, call this to hide the keyboard
   virtual void done();
   virtual std::function<void()> get_waiting_msg_op();
   virtual mws_sp<mws_vkb_impl> get_active_vkb();
   virtual void nwi_inex();
   virtual mws_sp<mws_vkb_impl> nwi_vkb();

   mws_sp<mws_vkb_file_store> file_store;
   // points to the current active vkb
   mws_sp<mws_vkb_impl> active_vkb;
   // points to the first vkb loaded, which contains the filestore
   mws_sp<mws_vkb_impl> vkb_file_store;
   // two vkbs are used, one for landscape, one for portrait orientations
   mws_sp<mws_vkb_impl> vkb_landscape;
   mws_sp<mws_vkb_impl> vkb_portrait;
   bool size_changed = false;
   mws_sp<mws_text_area> ta;
   std::string vkb_filename;
   std::function<void()> show_waiting_msg_op;
   static inline mws_sp<mws_vkb> inst;
};
