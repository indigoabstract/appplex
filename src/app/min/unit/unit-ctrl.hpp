#pragma once

#include "pfm.hpp"
#include <memory>


class unit;
class unit_list;
class pointer_evt;
class gfx;


class unit_ctrl
{
public:
   static std::shared_ptr<unit_ctrl> inst();
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
   bool update();
   void pause();
   void resume();
   void resize_app(int iwidth, int iheight);
   void pointer_action(std::shared_ptr<pointer_evt> ite);
   void key_action(key_actions iaction_type, int ikey);
   std::shared_ptr<unit> get_current_unit();
   void set_next_unit(std::shared_ptr<unit> iunit);
   void destroy_app();
   void start_app();
   std::shared_ptr<unit> get_app_start_unit();
   void set_gfx_available(bool iis_gfx_available);

private:
   unit_ctrl();

   void set_current_unit(std::shared_ptr<unit> unit0);

   std::weak_ptr<unit> crt_unit;
   std::weak_ptr<unit> next_unit;
   std::shared_ptr<unit_list> ul;
   bool exit_app_on_next_run;
   bool app_started;
   static std::shared_ptr<unit_ctrl> instance;
};
