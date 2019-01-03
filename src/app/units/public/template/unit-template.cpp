#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_TEMPLATE

#include "unit-template.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"

unit_template::unit_template() : unit(mws_stringify(UNIT_TEMPLATE)) {}

shared_ptr<unit_template> unit_template::nwi()
{
   return shared_ptr<unit_template>(new unit_template());
}

void unit_template::init()
{
}


namespace unit_template_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_page::init();
      }

      virtual void receive(shared_ptr<iadp> idp) override
      {
         mws_page::receive(idp);
      }

      virtual void update_state() override
      {
         mws_page::update_state();
      }

      virtual void update_view(shared_ptr<mws_camera> g) override
      {
         mws_page::update_view(g);

         const std::string& text = get_unit()->get_name();

         g->drawText(text, 10, 20);
      }
   };
}


void unit_template::init_mws()
{
   mws_root->new_page<unit_template_ns::main_page>();
   mws_cam->clear_color = true;
}

void unit_template::load()
{
}

#endif
