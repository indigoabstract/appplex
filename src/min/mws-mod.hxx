#pragma once

#include "pfm-def.h"
#include "min.hxx"
#include <mutex>
#include <memory>
#include <string>
#include <vector>


class mws_mod_ctrl;
class mws_mod_list;
class mws_page_tab;
class key_ctrl;
class touchctrl;
class updatectrl;
class gfx_scene;
class mws_camera;
class pfm_file;
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
   virtual int get_preferred_screen_width() { return 0; }
   virtual int get_preferred_screen_height() { return 0; }
   virtual double get_preferred_aspect_ratio() { return 0.; }
   virtual bool start_full_screen() { return false; }
   virtual bool draw_touch_symbols_trail() { return false; }
   virtual bool show_onscreen_console() { return false; }
   virtual bool show_fps() const { return true; }
   virtual uint32 get_font_db_pow_of_two_size() const { return 9; }

   virtual bool emulate_mobile_screen() { return false; }
};


class mws_mod : public std::enable_shared_from_this<mws_mod>, public mws_node
{
public:
   enum mod_type
   {
      e_mod_base,
      e_mod_list,
   };

   class app_storage_impl;
   class app_storage
   {
   public:
      app_storage();

      // file access
      mws_sp<std::vector<uint8> > load_mod_byte_vect(std::string name);
      //shared_array<uint8> load_mod_byte_array(std::string name, int& size);
      bool store_mod_byte_array(std::string name, const uint8* res_ptr, int size);
      bool store_mod_byte_vect(std::string name, const std::vector<uint8>& res_ptr);
      mws_sp<pfm_file> random_access(std::string name);

      // screenshot
      void save_screenshot(std::string i_filename = "");

      // screen video recording
      void start_recording_screen(std::string i_filename = "", const mws_video_params* i_params = nullptr);
      void stop_recording_screen();
      bool is_recording_screen();
      void toggle_screen_recording();

   private:
      friend class mws_mod;

      mws_up<app_storage_impl> p;
   };

   virtual ~mws_mod();

   virtual mod_type get_mod_type();
   int get_width();
   int get_height();
   // internal name (only used inside the engine for identification purposes). may be the same as external name
   const std::string& get_name();
   void set_name(std::string i_name);

   // external (display name). this is used for example, when setting the application name. may be the same as internal name
   const std::string& get_external_name();
   void set_external_name(std::string i_name);

   const std::string& get_proj_rel_path();
   void set_proj_rel_path(std::string ipath);
   bool is_gfx_mod();
   mws_sp<mws_mod_preferences> get_preferences();
   // true to exit app, false to continue
   virtual bool back();
   bool rsk_was_hit(int x0, int y0);
   int schedule_operation(const std::function<void()>& ioperation);
   bool cancel_operation(int ioperation_id);
   /// return a reference to the app_impl implementation
   template <typename T> T& i_m() const { mws_assert(p.get() != nullptr); return *mws_dynamic_cast<T*>(p.get()); }
   bool i_m_is_null() const { return p.get() == nullptr; }
   virtual void process(mws_sp<mws_dp> i_dp);
   bool handle_function_key(key_types i_key);
   virtual void config_font_db_size();

   int game_time;
   mws_sp<updatectrl> update_ctrl_inst;
   mws_sp<touchctrl> touch_ctrl_inst;
   mws_sp<key_ctrl> key_ctrl_inst;
   mws_sp<gfx_scene> gfx_scene_inst;
   mws_sp<mws_camera> mws_cam;
   mws_sp<mws_page_tab> mws_root;
   app_storage storage;

protected:
   mws_mod(const char* i_include_guard);

   mws_sp<mws_mod> get_smtp_instance();
   void set_internal_name_from_include_guard(const char* i_include_guard);

   static void set_app_exit_on_next_run(bool iapp_exit_on_next_run);
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
   virtual void update_view(int update_count);
   virtual void post_update_view();

   mws_up<app_impl> p;
   mws_sp<mws_mod_preferences> prefs;
   int frame_count;
   float fps;
   uint32 last_frame_time;

private:
   friend class mws_mod_ctrl;
   friend class mws_mod_list;

   void run_step();
   void base_load();
   void base_unload();
   bool is_init();
   void set_init(bool i_is_init);

   // mws_mod name/id
   std::string name;
   // mws_mod external/display name
   std::string external_name;
   // mws_mod path, relative to project (appplex) path
   std::string proj_rel_path;
   mws_wp<mws_mod> parent;
   bool init_val;
   std::vector<std::function<void()> > operation_list;
   std::mutex operation_mutex;

   static int mod_count;
};


class mws_mod_list : public mws_mod
{
public:
   static mws_sp<mws_mod_list> nwi();

   mod_type get_mod_type();
   void add(mws_sp<mws_mod> i_mod);
   mws_sp<mws_mod> mod_at(int i_index);
   mws_sp<mws_mod> mod_by_name(std::string i_name);
   int get_mod_count()const;
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
   static int mod_list_count;
};


class mws_mod_setup
{
private:
   friend class mws_mod_ctrl;

   static void append_mod_list(mws_sp<mws_mod_list> i_mod_list);
   static mws_sp<mws_mod_list> get_mod_list();
   static void add_mod(mws_sp<mws_mod> i_mod, std::string i_mod_path, bool i_set_current = false);

   static mws_wp<mws_mod_list> ul;
   static mws_wp<mws_mod> next_crt_mod;
};
