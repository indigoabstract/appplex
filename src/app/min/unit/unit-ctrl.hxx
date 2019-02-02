#pragma once

#include "pfm.hxx"
#include <memory>


class unit;
class mod_list;
class pointer_evt;
class gfx;


class unit_ctrl
{
public:
   static mws_sp<unit_ctrl> inst();
   // returns a shared pointer to the current app
   static mws_sp<unit> get_app();
   bool back_evt();
   bool app_uses_gfx();
   void exit_app(int exit_code = 0);
   bool is_set_app_exit_on_next_run();
   void set_app_exit_on_next_run(bool iexit_app_on_next_run);
   void pre_init_app();
   void init_app();
   const unicodestring& get_app_name();
   const unicodestring& get_app_description();
   void update();
   void pause();
   void resume();
   void resize_app(int i_width, int i_height);
   void pointer_action(mws_sp<pointer_evt> ite);
   void key_action(key_actions iaction_type, int ikey);
   mws_sp<unit> get_current_unit();
   void set_next_unit(mws_sp<unit> iunit);
   void destroy_app();
   void start_app();
   mws_sp<unit> get_app_start_unit();
   void set_gfx_available(bool iis_gfx_available);

private:
   unit_ctrl();

   void set_current_unit(mws_sp<unit> unit0);

   mws_wp<unit> crt_unit;
   mws_wp<unit> next_unit;
   mws_sp<mod_list> ul;
   bool exit_app_on_next_run;
   bool app_started;
   static mws_sp<unit_ctrl> instance;
};
