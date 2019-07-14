#pragma once

#include "mws/mws.hxx"
#include <unordered_map>


class mws_font;
class mws_vrn_main;
class text_vxo;


const std::string VKB_PREFIX = "vkb-";
const std::string VKB_EXT = ".kxmd";

const key_types VKB_ESCAPE = KEY_ESCAPE;
const key_types VKB_DONE = KEY_END;
const key_types VKB_ALT = KEY_ALT;
const key_types VKB_SHIFT = KEY_SHIFT;
const key_types VKB_ENTER = KEY_ENTER;
const key_types VKB_BACKSPACE = KEY_BACKSPACE;
const key_types VKB_DELETE = KEY_DELETE;
const key_types VKB_HIDE_KB = KEY_F1;
const key_types VKB_NEXT_SCREEN = KEY_F2;
const key_types VKB_PREV_SCREEN = KEY_F3;
const key_types VKB_CAPS_LOCK = KEY_F4;


enum class key_mod_types
{
   mod_none = 0,
   mod_alt,
   mod_shift,
   count,
};


enum class vkb_mod_lock_types
{
   no_lock = 0,
   alt_lock,
   caps_lock,
   hide_lock,
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
   mws_vkb_impl(uint32 i_obj_type_mask);
   static vkb_info get_vkb_info(const std::string& i_filename);
   static std::string get_vkb_filename(uint32 i_map_idx);
   virtual void setup() override;
   virtual void update_state() override;
   virtual void on_resize(uint32 i_width, uint32 i_height);
   virtual vkb_file_info get_closest_match(uint32 i_width, uint32 i_height);
   virtual void set_font(mws_sp<mws_font> i_fnt);
   virtual void done();
   std::string get_key_name(key_types i_key_id) const;
   key_types get_key_type(const std::string& i_key_name) const;
   void load_map(std::string i_filename);
   vkb_mod_lock_types get_active_lock() const;
   void set_active_lock(vkb_mod_lock_types i_lock);
   std::vector<key_types>& get_key_vect();
   uint32 get_key_vect_size();
   void set_key_vect_size(uint32 i_size);
   key_types get_key_at(int i_idx);
   void set_key_at(int i_idx, key_types i_key_id);
   void highlight_key_at(int i_idx);
   void fade_key_at(int i_idx);
   void erase_key_at(int i_idx);
   void push_back_key(key_types i_key_id);
   void next_page();
   void prev_page();
   void set_on_top();

   struct key_highlight
   {
      uint32 key_idx;
      uint32 release_time;
   };

   mws_sp<mws_vkb_file_store> file_store;
   uint32 obj_type_mask = 0;
   mws_sp<mws_vrn_main> vk;
   int selected_kernel_idx = -1;
   int current_key_idx = -1;
   key_mod_types key_mod = key_mod_types::mod_none;
   vkb_mod_lock_types active_lock = vkb_mod_lock_types::no_lock;
   std::vector<key_types> key_mod_vect[(uint32)key_mod_types::count];
   mws_sp<mws_font> key_font;
   mws_sp<mws_font> selected_key_font;
   mws_sp<text_vxo> vk_keys;
   bool keys_visible = true;
   std::unordered_map<key_types, std::string> key_map;
   int map_idx = 0;
   std::string loaded_filename;
   uint32 crt_page_idx = 0;
   std::vector<key_highlight> highlight_vect;
};


class mws_vkb : public mws_virtual_keyboard
{
public:
   static mws_sp<mws_vkb> gi();
   virtual ~mws_vkb() {}
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   virtual void on_resize() override;
   virtual void set_target(mws_sp<mws_text_area> i_tbx) override;
   virtual void set_font(mws_sp<mws_font> i_fnt) override;
   virtual mws_sp<mws_vkb_file_store> get_file_store() const override;
   virtual void set_file_store(mws_sp<mws_vkb_file_store> i_store) override;

protected:
   mws_vkb() {}
   virtual void setup() override;
   // when finished, call this to hide the keyboard
   virtual void done();
   virtual mws_sp<mws_vkb_impl> get_impl();

   mws_sp<mws_vkb_file_store> file_store;
   mws_sp<mws_vkb_impl> impl;
   mws_sp<mws_text_area> ta;
   std::string vkb_filename;
   static mws_sp<mws_vkb> inst;
};
