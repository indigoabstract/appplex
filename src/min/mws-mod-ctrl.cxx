#include "stdafx.hxx"

#include "mws-mod-ctrl.hxx"
#include "mws-mod.hxx"
#include "input/input-ctrl.hxx"
#include "min.hxx"
#include "gfx.hxx"
#include "snd/snd.hxx"
#include "mod-list.hxx"
#include <cstdlib>


mws_sp<mws_mod_ctrl> mws_mod_ctrl::instance;

mws_mod_ctrl::mws_mod_ctrl()
{
   exit_app_on_next_run = false;
   app_started = false;
}

mws_sp<mws_mod_ctrl> mws_mod_ctrl::inst()
{
   if (!instance)
   {
      instance = mws_sp<mws_mod_ctrl>(new mws_mod_ctrl());
   }

   return instance;
}

mws_sp<mws_mod> mws_mod_ctrl::get_app()
{
   return inst()->crt_mod.lock();
}

bool mws_mod_ctrl::back_evt()
{
   auto u = crt_mod.lock();

   if (u)
   {
      if (u == ul)
      {
         set_app_exit_on_next_run(true);

         return true;
      }

      return u->back();
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

void mws_mod_ctrl::set_app_exit_on_next_run(bool iexit_app_on_next_run)
{
   exit_app_on_next_run = iexit_app_on_next_run;
}

void mws_mod_ctrl::destroy_app()
{
   ul->on_destroy();

   if (mod_gfx_on)
   {
      gfx::on_destroy();
   }
}

void mws_mod_ctrl::pre_init_app()
{
   // trigger resource directory listing
   pfm::get_pfm_main_inst();

   if (!ul)
   {
      ul = mws_mod_list::nwi();
      ul->set_name("app-mws_mod-list");
      mws_mod_setup::next_crt_mod = crt_mod = ul;

      mws_mod_setup::append_mod_list(ul);
   }
}

void mws_mod_ctrl::init_app()
{
   if (mod_gfx_on && pfm::data.gfx_available)
   {
      gfx::global_init();
   }

   if (mod_snd_on)
   {
      snd::init();
   }

   if (ul)
   {
      ul->base_init();
      ul->base_load();
      ul->set_init(true);
   }
}

const unicodestring& mws_mod_ctrl::get_app_name()
{
   static const unicodestring name(untr("appplex"));

   return name;
}

const unicodestring& mws_mod_ctrl::get_app_description()
{
   static const unicodestring name(untr("appplex description"));

   return name;
}

void mws_mod_ctrl::update()
{
   if (mod_snd_on)
   {
      snd::update();
   }

   mws_sp<mws_mod> u = get_current_mod();
   mws_assert(u != nullptr);

#ifndef SINGLE_MOD_BUILD

   mws_sp<mws_mod> nu = next_mod.lock();

   if (nu&& nu != u)
   {
      mws_mod_ctrl::set_current_mod(nu);
      u = nu;
   }

#endif

   u->run_step();
}

void mws_mod_ctrl::pause()
{
   //mws_log::i()->push("mws_mod_ctrl::pause()");
   auto u = get_current_mod();

   if (u)
   {
      u->on_pause();
   }
}

void mws_mod_ctrl::resume()
{
   //mws_log::i()->push("mws_mod_ctrl::resume()");
   auto u = get_current_mod();

   if (u)
   {
      u->on_resume();
   }
}

void mws_mod_ctrl::resize_app(int i_width, int i_height)
{
   if (mod_gfx_on)
   {
      pfm::data.screen_width = i_width;
      pfm::data.screen_height = i_height;
      gfx::on_resize(i_width, i_height);

      if (ul && ul->is_init())
      {
         ul->on_resize();
      }
      else
      {
         auto u = get_current_mod();

         if (u && u->is_init())
         {
            //mws_log::i()->push("mws_mod_ctrl::resize_app()");
            u->on_resize();
         }
      }
   }
}

void mws_mod_ctrl::pointer_action(mws_sp<mws_ptr_evt_base> i_te)
{
   if (mod_input_on)
   {
      mws_sp<mws_mod> u = get_current_mod();

      if (u)
      {
         u->touch_ctrl->enqueue_pointer_event(i_te);
      }
   }
}

void mws_mod_ctrl::key_action(key_actions i_action_type, int i_key)
{
   if (mod_input_on)
   {
      mws_sp<mws_mod> u = get_current_mod();

      if (u)
      {
         switch (i_action_type)
         {
         case KEY_PRESS:
            u->key_ctrl_inst->key_pressed(i_key);
            break;

         case KEY_RELEASE:
            u->key_ctrl_inst->key_released(i_key);
            break;
         }
      }
   }
}

mws_sp<mws_mod> mws_mod_ctrl::get_current_mod()
{
   return crt_mod.lock();
}

void mws_mod_ctrl::set_next_mod(mws_sp<mws_mod> i_mod)
{
   next_mod = i_mod;
}

void mws_mod_ctrl::start_app()
{
   auto u = mws_mod_setup::next_crt_mod.lock();

   mws_mod_ctrl::set_current_mod(u);
   //mws_log::i()->push("mws_mod_ctrl::start_app()");
   u->on_resize();

   app_started = true;
}

mws_sp<mws_mod> mws_mod_ctrl::get_app_start_mod()
{
   auto u = mws_mod_setup::next_crt_mod.lock();

   if (u)
   {
      return u;
   }

   return ul;
}

void mws_mod_ctrl::set_gfx_available(bool iis_gfx_available)
{
   pfm::data.gfx_available = iis_gfx_available;
}

void mws_mod_ctrl::set_current_mod(mws_sp<mws_mod> i_mod)
{
   if (i_mod)
   {
      if (!crt_mod.expired())
      {
         crt_mod.lock()->base_unload();
      }

      crt_mod = i_mod;
      pfm::filesystem::load_res_file_map(i_mod);

      if (!i_mod->is_init())
      {
         i_mod->base_init();
         i_mod->set_init(true);
      }

      i_mod->base_load();
   }
   else
   {
      mws_signal_error("warning: tried to make current a null mws_mod");
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
