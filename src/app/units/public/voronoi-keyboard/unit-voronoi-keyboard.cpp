#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_VORONOI_KEYBOARD

#include "unit-voronoi-keyboard.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"


unit_voronoi_keyboard::unit_voronoi_keyboard()
{
   set_name("voronoi-keyboard");
}

shared_ptr<unit_voronoi_keyboard> unit_voronoi_keyboard::nwi()
{
   return shared_ptr<unit_voronoi_keyboard>(new unit_voronoi_keyboard());
}

void unit_voronoi_keyboard::init()
{
}


namespace unit_voronoi_keyboard_ns
{
   class main_page : public mws_page
   {
   public:
      main_page() {}

      virtual void init()
      {
         mws_page::init();
      }

      virtual void receive(shared_ptr<iadp> idp)
      {
         mws_page::receive(idp);
      }

      virtual void update_state()
      {
         mws_page::update_state();
      }

      virtual void update_view(shared_ptr<mws_camera> g)
      {
         mws_page::update_view(g);

         const std::string& text = get_unit()->get_name();

         g->drawText(text, 10, 20);
      }
   };
}


void unit_voronoi_keyboard::init_mws()
{
   mws_root->new_page<unit_voronoi_keyboard_ns::main_page>();
   mws_cam->clear_color = true;
}

void unit_voronoi_keyboard::load()
{
}

#endif
