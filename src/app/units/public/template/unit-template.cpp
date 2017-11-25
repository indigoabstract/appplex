#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_TEMPLATE

#include "unit-template.hpp"
#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-com.hpp"


unit_template::unit_template()
{
   set_name("template");
}

shared_ptr<unit_template> unit_template::new_instance()
{
   return shared_ptr<unit_template>(new unit_template());
}

void unit_template::init()
{
}


namespace unit_template_ns
{
   class main_page : public ux_page
   {
   public:
      main_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent) {}

      virtual void init()
      {
         ux_page::init();
      }

      virtual void receive(shared_ptr<iadp> idp)
      {
         ux_page::receive(idp);
      }

      virtual void update_state()
      {
         ux_page::update_state();
      }

      virtual void update_view(shared_ptr<ux_camera> g)
      {
         ux_page::update_view(g);

         const std::string& text = get_unit()->get_name();

         g->drawText(text, 10, 20);
      }
   };
}


void unit_template::init_ux()
{
   ux_page::new_shared_instance(new unit_template_ns::main_page(uxroot));
   ux_cam->clear_color = true;
}

void unit_template::load()
{
}

#endif
