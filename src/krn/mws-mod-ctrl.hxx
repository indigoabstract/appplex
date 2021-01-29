#pragma once

#include "pfm.hxx"
#include "krn.hxx"
#include <functional>


class mws_mod;
class mws_mod_list;
class mws_ptr_evt_base;
class mws_app_storage;


class mws_mod_ctrl
{
public:
   static mws_sp<mws_mod_ctrl> inst();
   bool back_evt();
   bool app_uses_gfx();
   void exit_app(int exit_code = 0);
   bool is_set_app_exit_on_next_run();
   void set_app_exit_on_next_run(bool i_exit_app_on_next_run = true);
   void pre_init_app();
   void init_app();
   void start_app();
   void destroy_app();
   unicode_string app_name();
   unicode_string app_description();
   void update();
   void pause();
   void resume();
   void resize_app(uint32_t i_width, uint32_t i_height);
   void pointer_action(mws_sp<mws_ptr_evt_base> i_te);
   void key_action(mws_key_actions i_action_type, mws_key_types i_key);
   mws_sp<mws_mod> get_current_mod();
   // storage for current mod
   mws_app_storage& app_storage();
   void set_next_mod(mws_sp<mws_mod> i_mod);
   mws_sp<mws_mod> get_app_start_mod();
   void set_gfx_available(bool i_is_gfx_available);
   static bool is_gfx_available();
   static uint32_t get_screen_width();
   static uint32_t get_screen_height();
   void run_on_next_frame_start(const std::function<void()>& i_op);

private:
   using operation_queue_type = std::vector<std::function<void()>>;
   mws_mod_ctrl();
   void set_current_mod(mws_sp<mws_mod> i_mod);
   void load_current_mod();

   mws_sp<mws_mod_list> ul;
   mws_wp<mws_mod> crt_mod;
   operation_queue_type on_frame_begin_q0, on_frame_begin_q1;
   mws_atomic_ptr_swap<operation_queue_type> on_frame_begin_q_ptr = mws_atomic_ptr_swap<operation_queue_type>(&on_frame_begin_q0, &on_frame_begin_q1);
};
