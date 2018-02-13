#pragma once

#include "pfm-def.h"
#include "min.hpp"
#include <mutex>
#include <memory>
#include <string>
#include <vector>


class unit_ctrl;
class unit_list;
class ux_page_tab;
class keyctrl;
class touchctrl;
class updatectrl;
class gfx_scene;
class ux_camera;
class pfm_file;
class ux_list_model;
class video_params_ffmpeg;


class app_impl
{
protected:
   app_impl() {}
};


class unit_preferences
{
public:
   virtual bool requires_gfx() { return true; }
   virtual bool start_fullscreen() { return false; }
   virtual bool draw_touch_symbols_trail() { return false; }
   virtual bool show_onscreen_console() { return false; }
};


class unit : public std::enable_shared_from_this<unit>, public ia_node
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
      std::shared_ptr<std::vector<uint8> > load_unit_byte_vect(std::string name);
      //shared_array<uint8> load_unit_byte_array(std::string name, int& size);
      bool store_unit_byte_array(std::string name, const uint8* res_ptr, int size);
      bool store_unit_byte_vect(std::string name, const std::vector<uint8>& res_ptr);
      std::shared_ptr<pfm_file> random_access(std::string name);

      // screenshot
      void save_screenshot(std::string ifilename = "");

      // screen video recording
      void start_recording_screen(std::string i_filename = "", const video_params_ffmpeg* i_params = nullptr);
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
   std::shared_ptr<unit_preferences> get_preferences();
   // true to exit app, false to continue
   bool back();
   bool rsk_was_hit(int x0, int y0);
   int schedule_operation(const std::function<void()> &ioperation);
   bool cancel_operation(int ioperation_id);

   int game_time;
   std::shared_ptr<updatectrl> update_ctrl;
   std::shared_ptr<touchctrl> touch_ctrl;
   std::shared_ptr<keyctrl> key_ctrl;
   std::shared_ptr<gfx_scene> gfx_scene_inst;
   std::shared_ptr<ux_camera> ux_cam;
   std::shared_ptr<ux_page_tab> uxroot;
   app_storage storage;

protected:
   unit();

   std::shared_ptr<unit> get_smtp_instance();

   static void set_app_exit_on_next_run(bool iapp_exit_on_next_run);
   static bool gfx_available();

   virtual bool update();
   virtual void on_resize();
   virtual void on_pause();
   virtual void on_resume();
   virtual void receive(std::shared_ptr<iadp> idp);
   // finish-constructor. here you can use things that won't work in the constructor, ie shared_from_this(), etc
   virtual void iInit();
   virtual void init();
   virtual void on_destroy();
   virtual void init_ux();
   virtual void load();
   virtual void unload();
   virtual std::shared_ptr<ia_sender> sender_inst();
   virtual void update_view(int update_count);

   std::unique_ptr<app_impl> p;
   std::shared_ptr<unit_preferences> prefs;
   int frame_count;
   float fps;
   uint32 last_frame_time;

private:
   friend class unit_ctrl;
   friend class unit_list;

   bool iRunFrame();
   void iLoad();
   void iUnload();
   bool isInit();
   void setInit(bool isInit0);

   // unit name/id
   std::string name;
   // unit external/display name
   std::string external_name;
   // unit path, relative to project (appplex) path
   std::string proj_rel_path;
   std::weak_ptr<unit> parent;
   bool initVal;
   std::vector<std::function<void()> > operation_list;
   std::mutex operation_mutex;

   static int unit_count;
};


class unit_list : public unit
{
public:
   static std::shared_ptr<unit_list> new_instance();

   unit_type get_unit_type();
   void add(std::shared_ptr<unit> unit0);
   std::shared_ptr<unit> unit_at(int index0);
   std::shared_ptr<unit> unit_by_name(std::string iname);
   int get_unit_count()const;
   virtual void on_resize();
   virtual void receive(std::shared_ptr<iadp> idp);
   void forward();
   static void up_one_level();

protected:
   unit_list();

   virtual void on_destroy();
   virtual void init_ux();

private:
   friend class unit_ctrl;
   friend class unit_ctrl_ext;

   std::vector<std::shared_ptr<unit> > ulist;
   weak_ptr<ux_list_model> ulmodel;
   static int unit_list_count;
};


class app_units_setup
{
private:
   friend class unit_ctrl;
   friend class unit_ctrl_ext;

   static void create_units(std::shared_ptr<unit_list> ul0);
   static std::shared_ptr<unit_list> get_unit_list();
   static void add_unit(std::shared_ptr<unit> iu, std::string iunit_path, bool iset_current = false);

   static std::weak_ptr<unit_list> ul;
   static std::weak_ptr<unit> next_crt_unit;
};
