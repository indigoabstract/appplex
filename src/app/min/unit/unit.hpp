#pragma once

#include "pfm-def.h"
#include "min.hpp"
#include <mutex>
#include <memory>
#include <string>
#include <vector>


class unit_ctrl;
class unit_list;
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


class unit_preferences
{
public:
   virtual bool requires_gfx();
   virtual int get_preferred_screen_width() { return 0; }
   virtual int get_preferred_screen_height() { return 0; }
   virtual double get_preferred_aspect_ratio() { return 0.; }
   virtual bool start_full_screen() { return false; }
   virtual bool draw_touch_symbols_trail() { return false; }
   virtual bool show_onscreen_console() { return false; }

   virtual bool emulate_mobile_screen() { return false; }
};


class unit : public std::enable_shared_from_this<unit>, public mws_node
{
public:
   enum unit_type
   {
      e_unit_base,
      e_unit_list,
   };

   class app_storage_impl;
   class app_storage
   {
   public:
      app_storage();

      // file access
      mws_sp<std::vector<uint8> > load_unit_byte_vect(std::string name);
      //shared_array<uint8> load_unit_byte_array(std::string name, int& size);
      bool store_unit_byte_array(std::string name, const uint8* res_ptr, int size);
      bool store_unit_byte_vect(std::string name, const std::vector<uint8>& res_ptr);
      mws_sp<pfm_file> random_access(std::string name);

      // screenshot
      void save_screenshot(std::string ifilename = "");

      // screen video recording
      void start_recording_screen(std::string i_filename = "", const mws_video_params* i_params = nullptr);
      void stop_recording_screen();
      bool is_recording_screen();
      void toggle_screen_recording();

   private:
      friend class unit;

      std::unique_ptr<app_storage_impl> p;
   };

   virtual ~unit();

   virtual unit_type get_unit_type();
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
   bool is_gfx_unit();
   mws_sp<unit_preferences> get_preferences();
   // true to exit app, false to continue
   virtual bool back();
   bool rsk_was_hit(int x0, int y0);
   int schedule_operation(const std::function<void()> &ioperation);
   bool cancel_operation(int ioperation_id);
   /// return a reference to the app_impl implementation
   template <typename T> T& i_m() const { mws_assert(p.get() != nullptr); return *mws_dynamic_cast<T*>(p.get()); }
   bool i_m_is_null() const { return p.get() == nullptr; }

   int game_time;
   mws_sp<updatectrl> update_ctrl;
   mws_sp<touchctrl> touch_ctrl;
   mws_sp<key_ctrl> key_ctrl_inst;
   mws_sp<gfx_scene> gfx_scene_inst;
   mws_sp<mws_camera> mws_cam;
   mws_sp<mws_page_tab> mws_root;
   app_storage storage;

protected:
   unit(const char* i_include_guard);

   mws_sp<unit> get_smtp_instance();
   void set_internal_name_from_include_guard(const char* i_include_guard);

   static void set_app_exit_on_next_run(bool iapp_exit_on_next_run);
   static bool gfx_available();

   virtual bool update();
   virtual void post_update() {}
   virtual void on_resize();
   virtual void on_pause();
   virtual void on_resume();
   virtual void receive(mws_sp<mws_dp> idp);
   // finish-constructor. here you can use things that won't work in the constructor, ie shared_from_this(), etc
   virtual void base_init();
   virtual void init();
   virtual void on_destroy();
   virtual void init_mws();
   virtual void load();
   virtual void unload();
   virtual mws_sp<mws_sender> sender_inst();
   virtual void update_view(int update_count);
   virtual void post_update_view();

   std::unique_ptr<app_impl> p;
   mws_sp<unit_preferences> prefs;
   int frame_count;
   float fps;
   uint32 last_frame_time;

private:
   friend class unit_ctrl;
   friend class unit_list;

   void run_step();
   void base_load();
   void base_unload();
   bool is_init();
   void set_init(bool i_is_init);

   // unit name/id
   std::string name;
   // unit external/display name
   std::string external_name;
   // unit path, relative to project (appplex) path
   std::string proj_rel_path;
   mws_wp<unit> parent;
   bool init_val;
   std::vector<std::function<void()> > operation_list;
   std::mutex operation_mutex;

   static int unit_count;
};


class unit_list : public unit
{
public:
   static mws_sp<unit_list> nwi();

   unit_type get_unit_type();
   void add(mws_sp<unit> i_unit);
   mws_sp<unit> unit_at(int i_index);
   mws_sp<unit> unit_by_name(std::string i_name);
   int get_unit_count()const;
   virtual void on_resize();
   virtual void receive(mws_sp<mws_dp> idp);
   void forward();
   static void up_one_level();

protected:
   unit_list();

   virtual void on_destroy();
   virtual void init_mws();

private:
   friend class unit_ctrl;
   friend class unit_ctrl_ext;

   std::vector<mws_sp<unit> > ulist;
   mws_wp<mws_list_model> ulmodel;
   static int unit_list_count;
};


class app_units_setup
{
private:
   friend class unit_ctrl;
   friend class unit_ctrl_ext;

   static void create_units(mws_sp<unit_list> ul0);
   static mws_sp<unit_list> get_unit_list();
   static void add_unit(mws_sp<unit> iu, std::string iunit_path, bool iset_current = false);

   static mws_wp<unit_list> ul;
   static mws_wp<unit> next_crt_unit;
};
