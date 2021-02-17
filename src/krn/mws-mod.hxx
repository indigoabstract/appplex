#pragma once

#include "pfm.hxx"
#include "krn.hxx"
#include <functional>
#include <memory>
#include <string>
#include <vector>


class mws_app_storage_impl;
class mws_mod_ctrl;
class mws_mod_list;
class mws_page_tab;
class mws_key_ctrl;
class mws_touch_ctrl;
class updatectrl;
class gfx_scene;
class mws_camera;
class mws_file;
class mws_list_model;
class mws_video_params;


class app_impl
{
public:
   virtual ~app_impl() {}

protected:
   app_impl() {}
};


class mws_mod_preferences
{
public:
   virtual bool requires_gfx();
   virtual bool log_enabled() { return false; }
   virtual uint32_t get_preferred_screen_width() { return 0; }
   virtual uint32_t get_preferred_screen_height() { return 0; }
   virtual double get_preferred_aspect_ratio() { return 0.; }
   virtual bool start_full_screen() { return false; }
   virtual bool draw_touch_symbols_trail() { return false; }
   virtual bool show_onscreen_console() { return false; }
   virtual bool show_fps() const { return true; }
   virtual uint32_t get_font_db_pow_of_two_size() const { return 9; }

   virtual bool emulate_mobile_screen() { return false; }
};


class mws_app_storage
{
public:
   const mws_file_map& get_res_file_list() const;
   std::vector<std::byte> load_as_byte_vect(mws_sp<mws_file> i_file) const;
   std::vector<std::byte> load_as_byte_vect(const mws_path& i_file_path) const;
   mws_sp<std::vector<std::byte>> load_as_sp_byte_vect(const mws_path& i_file_path) const;
   std::string load_as_string(mws_sp<mws_file> i_file) const;
   std::string load_as_string(const mws_path& i_file_path) const;
   // writable/private/persistent files directory
   const mws_path& prv_dir() const;
   // read-only/resource files directory
   const mws_path& res_dir() const;
   // temporary files directory
   const mws_path& tmp_dir() const;

   // screenshot
   void save_screenshot(const mws_path& i_file_path = "") const;

   // screen video recording
   void start_recording_screen(const mws_path& i_file_path = "", const mws_video_params* i_params = nullptr);
   void stop_recording_screen();
   bool is_recording_screen();
   void toggle_screen_recording();

private:
   friend class mws_mod;
   friend class mws_mod_ctrl;
   mws_app_storage();

   mws_up<mws_app_storage_impl> p;
};


class mws_mod : public std::enable_shared_from_this<mws_mod>, public mws_node
{
public:
   enum mod_type
   {
      e_mod_base,
      e_mod_list,
   };

   virtual ~mws_mod();

   virtual mod_type get_mod_type();
   uint32_t get_width();
   uint32_t get_height();
   // internal name (only used inside the engine for identification purposes). may be the same as external name
   const std::string& name() const;
   void name(const std::string& i_name);

   // external (display name). this is used for example, when setting the application name. may be the same as internal name
   const std::string& external_name() const;
   void external_name(const std::string& i_name);
   const std::string& description() const;
   void description(const std::string& i_description);

   const mws_path& get_proj_rel_path();
   void set_proj_rel_path(const mws_path& i_path);
   bool is_gfx_mod();
   mws_sp<mws_mod_preferences> get_preferences();
   // true to exit app, false to continue
   virtual bool back();
   void run_on_next_frame_start(const std::function<void()>& i_op);
   void run_on_crt_frame_end(const std::function<void()>& i_op);
   /// return a reference to the app_impl implementation
   template <typename T> T& i_m() const { mws_assert(p.get() != nullptr); return *mws_dynamic_cast<T*>(p.get()); }
   bool i_m_is_null() const { return p.get() == nullptr; }
   virtual void process(mws_sp<mws_dp> i_dp);
   bool handle_function_key(mws_key_types i_key);
   virtual void config_font_db_size();

   uint32_t game_time = 0;
   mws_sp<updatectrl> update_ctrl_inst;
   mws_sp<mws_touch_ctrl> touch_ctrl_inst;
   mws_sp<mws_key_ctrl> key_ctrl_inst;
   mws_sp<gfx_scene> gfx_scene_inst;
   mws_sp<mws_camera> mws_cam;
   mws_sp<mws_page_tab> mws_root;
   mws_app_storage storage;

protected:
   mws_mod(const char* i_include_guard);

   mws_sp<mws_mod> get_smtp_instance();
   void set_internal_name_from_include_guard(const char* i_include_guard);

   static void set_app_exit_on_next_run(bool i_app_exit_on_next_run);
   static bool gfx_available();

   virtual bool update();
   virtual void post_update() {}
   virtual void on_resize();
   virtual void on_pause();
   virtual void on_resume();
   virtual void receive(mws_sp<mws_dp> i_dp);
   // finish-constructor. here you can use things that won't work in the constructor, ie shared_from_this(), etc
   virtual void base_init();
   virtual void init();
   virtual void on_destroy();
   virtual void init_mws();
   virtual void post_init_mws();
   virtual void load();
   virtual void unload();
   virtual mws_sp<mws_sender> sender_inst();
   virtual void update_view(uint32_t update_count);
   virtual void post_update_view();

   mws_up<app_impl> p;
   mws_sp<mws_mod_preferences> prefs;
   uint32_t frame_count = 0;
   float fps = 0.f;
   uint32_t last_frame_time = 0;

private:
   using operation_queue_type = std::vector<std::function<void()>>;
   friend class mws_mod_ctrl;
   friend class mws_mod_list;

   void run_step();
   void base_load();
   void base_unload();
   bool is_init();
   void set_init(bool i_is_init);

   // mws_mod name/id
   std::string name_v;
   // mws_mod external/display name
   std::string external_name_v;
   std::string description_v;
   // mws_mod path, relative to project (appplex) path
   mws_path proj_rel_path;
   mws_wp<mws_mod> parent;
   bool init_val = false;
   operation_queue_type on_frame_begin_q0, on_frame_begin_q1;
   mws_atomic_ptr_swap<operation_queue_type> on_frame_begin_q_ptr = mws_atomic_ptr_swap<operation_queue_type>(&on_frame_begin_q0, &on_frame_begin_q1);
   operation_queue_type on_frame_end_q0, on_frame_end_q1;
   mws_atomic_ptr_swap<operation_queue_type> on_frame_end_q_ptr = mws_atomic_ptr_swap<operation_queue_type>(&on_frame_end_q0, &on_frame_end_q1);
   inline static uint32_t mod_count = 0;
};


class mws_mod_list : public mws_mod
{
public:
   static mws_sp<mws_mod_list> nwi();

   mod_type get_mod_type();
   void add(mws_sp<mws_mod> i_mod);
   mws_sp<mws_mod> mod_at(uint32_t i_index);
   mws_sp<mws_mod> mod_by_name(const std::string& i_name);
   uint32_t get_mod_count()const;
   virtual void on_resize();
   virtual void receive(mws_sp<mws_dp> i_dp);
   void forward();
   static void up_one_level();

protected:
   mws_mod_list();

   virtual void on_destroy();
   virtual void init_mws();

private:
   friend class mws_mod_ctrl;

   std::vector<mws_sp<mws_mod> > ulist;
   mws_wp<mws_list_model> ulmodel;
   static uint32_t mod_list_count;
};


class mws_mod_setup
{
private:
   friend class mws_mod_ctrl;

   static void append_mod_list(mws_sp<mws_mod_list> i_mod_list);
   static mws_sp<mws_mod_list> get_mod_list();
   static void add_mod(mws_sp<mws_mod> i_mod, const mws_path& i_mod_path, bool i_set_current = false);

   static inline mws_wp<mws_mod_list> ul;
   static inline mws_wp<mws_mod> next_crt_mod;
};
