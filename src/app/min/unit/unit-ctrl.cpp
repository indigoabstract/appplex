#include "stdafx.h"

#include "unit-ctrl.hpp"
#include "appplex-conf.hpp"
#include "pfm.hpp"
#include "unit.hpp"
#include "com/unit/input-ctrl.hpp"
#include "min.hpp"
#include "gfx.hpp"
//#include "snd.hpp"
#include <cstdlib>


shared_ptr<unit_ctrl> unit_ctrl::instance;

unit_ctrl::unit_ctrl()
{
   exit_app_on_next_run = false;
   app_started = false;
}

shared_ptr<unit_ctrl> unit_ctrl::inst()
{
   if (!instance)
   {
      instance = shared_ptr<unit_ctrl>(new unit_ctrl());
   }

   return instance;
}

mws_sp<unit> unit_ctrl::get_app()
{
   return inst()->crt_unit.lock();
}

bool unit_ctrl::back_evt()
{
   auto u = crt_unit.lock();

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

bool unit_ctrl::app_uses_gfx()
{
   bool req_gfx = false;
   int unit_count = ul->get_unit_count();

   if (unit_count > 0)
   {
      for (int k = 0; k < unit_count; k++)
      {
         req_gfx = req_gfx || ul->unit_at(k)->get_preferences()->requires_gfx();
      }
   }
   else
   {
      req_gfx = true;
   }

   return req_gfx;
}

void unit_ctrl::exit_app(int exit_code)
{
   exit(exit_code);
}

bool unit_ctrl::is_set_app_exit_on_next_run()
{
   return exit_app_on_next_run;
}

void unit_ctrl::set_app_exit_on_next_run(bool iexit_app_on_next_run)
{
   exit_app_on_next_run = iexit_app_on_next_run;
}

void unit_ctrl::destroy_app()
{
   ul->on_destroy();
}

void unit_ctrl::pre_init_app()
{
   // trigger resource directory listing
   pfm::get_pfm_main_inst();

   if (!ul)
   {
      ul = unit_list::nwi();
      ul->set_name("app-unit-list");
      app_units_setup::next_crt_unit = crt_unit = ul;

      app_units_setup::create_units(ul);
   }
}

void unit_ctrl::init_app()
{
#ifdef MOD_GFX
   if (pfm::data.gfx_available)
   {
      gfx::global_init();
   }
#endif // MOD_GFX

#ifdef MOD_SND
   snd::init();
#endif // MOD_SND

   if (ul)
   {
      ul->iInit();
      ul->iLoad();
      ul->setInit(true);
   }
}

const unicodestring& unit_ctrl::get_app_name()
{
   static const unicodestring name(untr("appplex"));

   return name;
}

const unicodestring& unit_ctrl::get_app_description()
{
   static const unicodestring name(untr("appplex description"));

   return name;
}

bool unit_ctrl::update()
{
#ifdef MOD_SND
   snd::update();
#endif // MOD_SND

   shared_ptr<unit> u = get_current_unit();
   mws_assert(u != shared_ptr<unit>());

#ifndef SINGLE_UNIT_BUILD

   shared_ptr<unit> nu = next_unit.lock();

   if (nu && nu != u)
   {
      unit_ctrl::set_current_unit(nu);
      u = nu;
   }

#endif

   bool showFrame = u->iRunFrame();

   return showFrame;
}

void unit_ctrl::pause()
{
   auto u = get_current_unit();

   if (u)
   {
      u->on_pause();
   }
}

void unit_ctrl::resume()
{
   auto u = get_current_unit();

   if (u)
   {
      u->on_resume();
   }
}

void unit_ctrl::resize_app(int iwidth, int iheight)
{
   pfm::data.screen_width = iwidth;
   pfm::data.screen_height = iheight;

   if (ul)
   {
      ul->on_resize();
   }
   else
   {
      auto u = get_current_unit();

      if (u)
      {
         u->on_resize();
      }
   }
}

void unit_ctrl::pointer_action(std::shared_ptr<pointer_evt> ite)
{
   shared_ptr<unit> u = get_current_unit();

   if (u)
   {
      u->touch_ctrl->enqueue_pointer_event(ite);
   }
}

void unit_ctrl::key_action(key_actions iaction_type, int ikey)
{
   shared_ptr<unit> u = get_current_unit();

   if (u)
   {
      switch (iaction_type)
      {
      case KEY_PRESS:
         u->key_ctrl->key_pressed(ikey);
         break;

      case KEY_RELEASE:
         u->key_ctrl->key_released(ikey);
         break;
      }
   }
}

shared_ptr<unit> unit_ctrl::get_current_unit()
{
   return crt_unit.lock();
}

void unit_ctrl::set_next_unit(std::shared_ptr<unit> iunit)
{
   next_unit = iunit;
}

void unit_ctrl::start_app()
{
   auto u = app_units_setup::next_crt_unit.lock();

   unit_ctrl::set_current_unit(u);
   u->on_resize();

   app_started = true;
}

shared_ptr<unit> unit_ctrl::get_app_start_unit()
{
   auto u = app_units_setup::next_crt_unit.lock();

   if (u)
   {
      return u;
   }

   return ul;
}

void unit_ctrl::set_gfx_available(bool iis_gfx_available)
{
   pfm::data.gfx_available = iis_gfx_available;
}

void unit_ctrl::set_current_unit(shared_ptr<unit> unit0)
{
   if (unit0)
   {
      if (!crt_unit.expired())
      {
         crt_unit.lock()->iUnload();
      }

      crt_unit = unit0;
      pfm::filesystem::load_res_file_map(unit0);

      if (!unit0->isInit())
      {
         unit0->iInit();
         unit0->setInit(true);
      }

      unit0->iLoad();
   }
   else
   {
      trx("warning: tried to make current a null unit");
   }
}
