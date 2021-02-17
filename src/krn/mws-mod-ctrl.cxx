#include "stdafx.hxx"

#include "mws-mod-ctrl.hxx"
#include "mws-impl.hxx"
#include "mws-mod.hxx"
#include "input/input-ctrl.hxx"
#include "krn.hxx"
#include "gfx.hxx"
#include "snd/snd.hxx"
#include "mws-vkb/mws-vkb.hxx"
#include "mod-list.hxx"
#include <atomic>
#include <cstdlib>


mws_sp<mws_app> mws_app_inst();


namespace
{
   std::atomic_int32_t atomic_dim;
   mws_wp<mws_mod> next_mod;
   bool exit_app_on_next_run = false;
   bool app_started = false;
   bool gfx_available = true;
   uint32_t screen_width = 1280;
   uint32_t screen_height = 720;
   mws_sp<mws_mod_ctrl> instance;
}


mws_mod_ctrl::mws_mod_ctrl() {}

mws_sp<mws_mod_ctrl> mws_mod_ctrl::inst()
{
   if (!instance)
   {
      instance = mws_sp<mws_mod_ctrl>(new mws_mod_ctrl());
   }

   return instance;
}

bool mws_mod_ctrl::back_evt()
{
   auto mod = crt_mod.lock();

   if (mod)
   {
      if (mod == ul)
      {
         set_app_exit_on_next_run(true);

         return true;
      }

      return mod->back();
   }

   return false;
}

bool mws_mod_ctrl::app_uses_gfx()
{
   if (mod_gfx_on)
   {
      bool req_gfx = false;
      int mod_count = ul->get_mod_count();

      if (mod_count > 0)
      {
         for (int k = 0; k < mod_count; k++)
         {
            req_gfx = req_gfx || ul->mod_at(k)->get_preferences()->requires_gfx();
         }
      }
      else
      {
         req_gfx = true;
      }

      return req_gfx;
   }
   else
   {
      return false;
   }
}

void mws_mod_ctrl::exit_app(int exit_code)
{
   exit(exit_code);
}

bool mws_mod_ctrl::is_set_app_exit_on_next_run()
{
   return exit_app_on_next_run;
}

void mws_mod_ctrl::set_app_exit_on_next_run(bool i_exit_app_on_next_run)
{
   exit_app_on_next_run = i_exit_app_on_next_run;
}

void mws_mod_ctrl::pre_init_app()
{
   mws_assert(ul == nullptr);
   ul = mws_mod_list::nwi();
   ul->name("app-mws-mod-list");
   mws_mod_setup::next_crt_mod = crt_mod = mws_sp<mws_mod>();
   mws_mod_setup::append_mod_list(ul);
   mws_sp<mws_mod> start_mod = get_app_start_mod();

   if (start_mod)
   {
      set_current_mod(start_mod);
   }
   else
   {
      set_current_mod(ul);
   }
}

void mws_mod_ctrl::init_app()
{
   mws_sp<mws_mod> active_mod = get_current_mod();

   if (mod_gfx_on && gfx_available)
   {
      gfx::global_init();

      if (active_mod)
      {
         active_mod->config_font_db_size();
      }
   }

   if (mod_snd_on)
   {
      snd::init();
   }

#ifndef SINGLE_MOD_BUILD
   if (ul)
   {
      if (!active_mod)
      {
         ul->config_font_db_size();
      }

      ul->base_init();
      ul->base_load();
      ul->set_init(true);
   }
#endif
}

void mws_mod_ctrl::start_app()
{
   load_current_mod();
   get_current_mod()->on_resize();
   app_started = true;
}

void mws_mod_ctrl::destroy_app()
{
   ul->on_destroy();

   if (mod_gfx_on)
   {
      gfx::on_destroy();
   }
}

unicode_string mws_mod_ctrl::app_name()
{
   static unicode_string name;
   mws_sp<mws_mod> mod = get_current_mod();

   if (mod)
   {
      name = mws_str::string_to_unicode_string(mod->external_name());
   }
   else
   {
      name = untr("appplex");
   }

   return name;
}

unicode_string mws_mod_ctrl::app_description()
{
   static unicode_string name;
   mws_sp<mws_mod> mod = get_current_mod();

   if (mod)
   {
      name = mws_str::string_to_unicode_string(mod->description());
   }
   else
   {
      name = untr("appplex description");
   }

   return name;
}

void mws_mod_ctrl::update()
{
   mws_sp<mws_mod> mod = get_current_mod();
   mws_assert(mod != nullptr);

   if (mod_gfx_on)
   {
      uint32_t dim = atomic_dim;
      uint32_t width = (dim >> 16);
      uint32_t height = (dim & 0xffff);

      if (screen_width != width || screen_height != height)
      {
         screen_width = width;
         screen_height = height;
         gfx::on_resize(width, height);

         if (ul && ul->is_init())
         {
            ul->on_resize();
         }
         else
         {
            if (mod && mod->is_init())
            {
               //mws_log::i()->push("mws_mod_ctrl::resize_app()");
               mod->on_resize();
            }
         }
      }
   }

   if (mod_snd_on)
   {
      snd::update();
   }

#ifndef SINGLE_MOD_BUILD

   mws_sp<mws_mod> next = next_mod.lock();

   if (next && next != mod)
   {
      set_current_mod(next);
      load_current_mod();
      mod = next;
   }

#endif

   {
      // swap queues, so the currently empty queue will be used for taking new operations
      on_frame_begin_q_ptr.swap();
      operation_queue_type& on_frame_begin_queue = *on_frame_begin_q_ptr.second();

      if (!on_frame_begin_queue.empty())
      {
         for (const auto& exe : on_frame_begin_queue)
         {
            exe();
         }

         on_frame_begin_queue.clear();
      }
   }

   mod->run_step();
}

void mws_mod_ctrl::pause()
{
   //mws_log::i()->push("mws_mod_ctrl::pause()");
   auto mod = get_current_mod();

   if (mod)
   {
      mod->on_pause();
   }
}

void mws_mod_ctrl::resume()
{
   //mws_log::i()->push("mws_mod_ctrl::resume()");
   auto mod = get_current_mod();

   if (mod)
   {
      mod->on_resume();
   }
}

void mws_mod_ctrl::resize_app(uint32_t i_width, uint32_t i_height)
{
   uint32_t dim = (i_width << 16) | i_height;
   atomic_dim = dim;
}

void mws_mod_ctrl::pointer_action(mws_sp<mws_ptr_evt_base> i_te)
{
   if (mod_input_on)
   {
      mws_sp<mws_mod> mod = get_current_mod();

      if (mod)
      {
         mod->touch_ctrl_inst->enqueue_pointer_event(i_te);
      }
   }
}

void mws_mod_ctrl::key_action(mws_key_actions i_action_type, mws_key_types i_key)
{
   if (mod_input_on)
   {
      mws_sp<mws_mod> mod = get_current_mod();

      if (mod)
      {
         bool physical_keyboard_enabled = true;

         if (mod_mws_vkb_on)
         {
            mws_sp<mws_vkb> vkb = mws_vkb::gi();

            if (vkb && vkb->is_visible())
            {
               bool is_f_key = (i_key >= mws_key_f1 && i_key <= mws_key_f12);

               // handle function key pressed when virtual keyboard is visible
               if (is_f_key && i_action_type == mws_key_press)
               {
                  mod->handle_function_key(i_key);
               }

               physical_keyboard_enabled = false;
            }
         }

         // disable the physical keyboard when the virtual one is visible
         if (physical_keyboard_enabled)
         {
            switch (i_action_type)
            {
            case mws_key_press:
               mod->key_ctrl_inst->key_pressed(i_key);
               break;

            case mws_key_release:
               mod->key_ctrl_inst->key_released(i_key);
               break;
            }
         }
      }
   }
}

mws_sp<mws_mod> mws_mod_ctrl::get_current_mod()
{
   return crt_mod.lock();
}

mws_app_storage& mws_mod_ctrl::app_storage() { return get_current_mod()->storage; }

void mws_mod_ctrl::set_next_mod(mws_sp<mws_mod> i_mod)
{
   next_mod = i_mod;
}

mws_sp<mws_mod> mws_mod_ctrl::get_app_start_mod()
{
   return mws_mod_setup::next_crt_mod.lock();
}

void mws_mod_ctrl::set_gfx_available(bool i_is_gfx_available)
{
   gfx_available = i_is_gfx_available;
}

bool mws_mod_ctrl::is_gfx_available() { return gfx_available; }
uint32_t mws_mod_ctrl::get_screen_width() { return screen_width; }
uint32_t mws_mod_ctrl::get_screen_height() { return screen_height; }

void mws_mod_ctrl::run_on_next_frame_start(const std::function<void()>& i_op)
{
   on_frame_begin_q_ptr.first()->push_back(i_op);
}

void mws_mod_ctrl::load_current_mod()
{
   mws_sp<mws_mod> mod = crt_mod.lock();

   if (mod)
   {
      if (!mod->is_init())
      {
         mod->base_init();
         mod->set_init(true);
      }

      mod->base_load();
   }
}

mws_sp<mws_ptr_evt_base> mws_ptr_evt_base::nwi()
{
   if (mod_input_on)
   {
      return mws_ptr_evt::nwi();
   }
   else
   {
      return mws_sp<mws_ptr_evt_base>(new mws_ptr_evt_base());
   }
}
