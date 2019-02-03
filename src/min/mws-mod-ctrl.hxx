#pragma once

#include "pfm.hxx"
#include <memory>


class mws_mod;
class mws_mod_list;
class mws_ptr_evt_base;
class gfx;


class mws_mod_ctrl
{
public:
   static mws_sp<mws_mod_ctrl> inst();
   // returns a shared pointer to the current app
   static mws_sp<mws_mod> get_app();
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
   void pointer_action(mws_sp<mws_ptr_evt_base> ite);
   void key_action(key_actions iaction_type, int ikey);
   mws_sp<mws_mod> get_current_mod();
   void set_next_mod(mws_sp<mws_mod> i_mod);
   void destroy_app();
   void start_app();
   mws_sp<mws_mod> get_app_start_mod();
   void set_gfx_available(bool iis_gfx_available);

private:
   mws_mod_ctrl();

   void set_current_mod(mws_sp<mws_mod> i_mod);

   mws_wp<mws_mod> crt_mod;
   mws_wp<mws_mod> next_mod;
   mws_sp<mws_mod_list> ul;
   bool exit_app_on_next_run;
   bool app_started;
   static mws_sp<mws_mod_ctrl> instance;
};
