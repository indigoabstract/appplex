#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef UNIT_JCV

#include "unit-jcv.hxx"
#include "jc_voronoi.h"
#include "com/mws/mws-camera.hxx"
#include "com/mws/mws-com.hxx"


unit_jcv::unit_jcv() : unit(mws_stringify(UNIT_JCV)) {}

mws_sp<unit_jcv> unit_jcv::nwi()
{
   return mws_sp<unit_jcv>(new unit_jcv());
}

void unit_jcv::init()
{
}


namespace unit_jcv_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_page::init();
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
      {
         mws_page::receive(i_dp);
      }

      virtual void update_state() override
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         mws_page::update_view(i_g);

         const std::string& text = get_unit()->get_name();

         i_g->drawText(text, 10, 20);
      }
   };
}


void unit_jcv::init_mws()
{
   mws_root->new_page<unit_jcv_ns::main_page>();
   mws_cam->clear_color = true;
}

void unit_jcv::load()
{
}

#endif
