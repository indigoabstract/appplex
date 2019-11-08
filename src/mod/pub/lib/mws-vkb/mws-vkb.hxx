#pragma once

#include "mws/mws.hxx"
#include "input/gesture-detectors.hxx"
#include "gfx-pbo.hxx"
//#include <unordered_map>


class mws_font;
class mws_vrn_main;
class text_vxo;
class gfx_tex;


const std::string VKB_PREFIX = "vkb-";
const std::string VKB_EXT = ".kxmd";

// mod keys
const key_types VKB_ALT = KEY_ALT;
const key_types VKB_HIDE_KB = KEY_F1;
const key_types VKB_SHIFT = KEY_SHIFT;
// special keys
const key_types VKB_ESCAPE = KEY_ESCAPE;
const key_types VKB_DONE = KEY_END;
const key_types VKB_ENTER = KEY_ENTER;
const key_types VKB_BACKSPACE = KEY_BACKSPACE;
const key_types VKB_DELETE = KEY_DELETE;
const key_types VKB_NEXT_SCREEN = KEY_F2;
const key_types VKB_PREV_SCREEN = KEY_F3;


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
   uint32 width = 0;
   uint32 height = 0;
   // file index for this aspect ratio
   uint32 index = 0;
   float aspect_ratio = 0.f;
};


struct vkb_file_info
{
   vkb_info info;
   mws_sp<pfm_file> file;
};


class mws_vkb_file_store_impl : public mws_vkb_file_store
{
public:
   std::vector<vkb_file_info> get_vkb_list() override;
   bool file_exists(const std::string& i_vkb_filename) override;
   void save_vkb(const std::string& i_vkb_filename, const std::string& i_data) override;
   mws_sp<std::string> load_vkb(const std::string& i_vkb_filename) override;

protected:
   std::vector<vkb_file_info> vkb_info_vect;
};


class mws_vkb_impl : public mws
{
public:
   static inline const float key_lights_off_seconds = 2.5f;

   mws_vkb_impl(uint32 i_obj_type_mask);
   static vkb_info get_vkb_info(const std::string& i_filename);
   static std::string get_vkb_filename(uint32 i_map_idx);
   virtual void setup() override;
   virtual void receive(mws_sp<mws_dp> i_dp) override {}
   virtual void update_state() override {}
   virtual void update_view(mws_sp<mws_camera> g) override {}
   virtual mws_sp<mws_ptr_evt> on_receive(mws_sp<mws_ptr_evt> i_pe, mws_sp<mws_text_area> i_ta);
   virtual void on_update_state();
   virtual void on_resize(uint32 i_width, uint32 i_height);
   virtual vkb_file_info get_closest_match(uint32 i_width, uint32 i_height);
   virtual mws_sp<mws_font> get_font() const;
   virtual void set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt);
   virtual void done();
   std::string get_key_name(key_types i_key_id) const;
   key_types get_key_type(const std::string& i_key_name) const;
   void load_map(std::string i_filename);
   std::vector<key_types>& get_key_vect();
   uint32 get_key_vect_size();
   // returns the key id at position i_idx
   key_types get_key_at(int i_idx);
   // returns the key id at position 'i_idx' in mod 'i_key_mod'
   key_types get_mod_key_at(key_mod_types i_key_mod, int i_idx);
   void next_page();
   void prev_page();
   void set_on_top();
   std::vector<mws_sp<gfx_tex>> get_tex_list();
   void build_cell_border_tex();
   void build_keys_tex();
   mws_sp<mws_font> get_key_font() const { return letter_font; }
   bool is_mod_key(key_types i_key_id);
   // check if i_mod_key is pressed or locked
   enum class mod_key_types { alt = VKB_ALT, hide_vkb = VKB_HIDE_KB, shift = VKB_SHIFT, };
   enum class base_key_state_types { key_free, key_held, key_locked, };
   // returns true if i_mod_key is either held or locked. you can use i_state to find out the actual state of i_mod_key
   bool is_mod_key_held(mod_key_types i_mod_key, base_key_state_types* i_state = nullptr, int* i_key_idx = nullptr) const;
   void set_mod_key_lock(mod_key_types i_mod_key, bool i_set_lock = true);
   void clear_mod_key_locks();
   // release all held and locked keys (optional)
   void release_all_keys(bool i_release_locked_keys = true);

   mws_sp<mws_vkb_file_store> file_store;
   mws_sp<mws_vrn_main> vk;
   int selected_kernel_idx = -1;
   int current_key_idx = -1;
   glm::vec2 diag_dim;
   bool keys_visible = true;
   std::string loaded_filename;

protected:
   void set_key_vect_size(uint32 i_size);
   void set_key_at(int i_idx, key_types i_key_id);
   void erase_key_at(int i_idx);
   void push_back_key(key_types i_key_id);
   virtual bool touch_began(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_moved(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_ended(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   virtual bool touch_cancelled(mws_sp<mws_ptr_evt> i_crt, mws_sp<mws_text_area> i_ta);
   void highlight_key_at(int i_idx, bool i_light_on = true);
   void fade_key_at(int i_idx);
   // set the state of the keys. if return value is true, the iterators are invalidated and need to be aborted
   // returns true when keyboard has been hidden (and the key state cleared), false otherwise
   bool set_key_state(int i_key_idx, base_key_state_types i_state);
   // call this after modifying / inserting / deleting a base key (a mod_node key)
   void rebuild_key_state();

   uint32 obj_type_mask = 0;
   key_mod_types key_mod = key_mod_types::mod_none;
   std::vector<key_types> key_mod_vect[(uint32)key_mod_types::count];
   std::unordered_map<key_types, std::string> key_map;
   int map_idx = 0;
   uint32 crt_page_idx = 0;
   struct key_highlight { int key_idx; uint32 release_time; };
   std::vector<key_highlight> highlight_vect;
   double_tap_detector dbl_tap_det;
   struct base_key_state { key_types key_id; base_key_state_types state; uint8 pressed_count; };
   std::vector<base_key_state> base_key_st;
   mws_sp<mws_ptr_evt> prev_ptr_evt;
   std::unordered_map<int, key_types> mod_keys_st;

private:
   mws_sp<mws_font> letter_font;
   mws_sp<mws_font> word_font;
   mws_sp<gfx_tex> cell_border_tex;
   mws_gfx_ppb key_border_tex;
   mws_sp<gfx_quad_2d> key_border_quad;
   std::vector<mws_gfx_ppb> keys_tex;
   mws_sp<gfx_quad_2d> keys_quad;
};


class mws_vkb : public mws_virtual_keyboard
{
public:
   static mws_sp<mws_vkb> gi();
   virtual ~mws_vkb() {}
   // i_key_id is not actually a key id, but a position index for the points on the keyboard
   virtual key_types apply_key_modifiers(key_types i_key_id) const override;
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual void update_state() override;
   virtual void on_resize() override;
   virtual void set_target(mws_sp<mws_text_area> i_tbx) override;
   virtual mws_sp<mws_font> get_font() override;
   virtual void set_font(mws_sp<mws_font> i_letter_fnt, mws_sp<mws_font> i_word_fnt) override;
   virtual mws_sp<mws_vkb_file_store> get_file_store() const override;
   virtual void set_file_store(mws_sp<mws_vkb_file_store> i_store) override;
   virtual std::vector<mws_sp<gfx_tex>> get_tex_list() override;

protected:
   mws_vkb();
   virtual void setup() override;
   // when finished, call this to hide the keyboard
   virtual void done();
   virtual mws_sp<mws_vkb_impl> get_impl();
   virtual mws_sp<mws_vkb_impl> nwi_impl();

   mws_sp<mws_vkb_file_store> file_store;
   // points to the current active vkb
   mws_sp<mws_vkb_impl> impl;
   // points to the first vkb loaded, which contains the filestore
   mws_sp<mws_vkb_impl> vkb_file_store;
   // two vkbs are used, one for landscape, one for portrait orientations
   mws_sp<mws_vkb_impl> vkb_landscape;
   mws_sp<mws_vkb_impl> vkb_portrait;
   bool size_changed = false;
   mws_sp<mws_text_area> ta;
   std::string vkb_filename;
   static mws_sp<mws_vkb> inst;
};
