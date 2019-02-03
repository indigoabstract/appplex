#include "stdafx.hxx"

#include "mod-jcv.hxx"
#include "jc_voronoi.h"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_jcv::mod_jcv() : mws_mod(mws_stringify(MOD_JCV)) {}

mws_sp<mod_jcv> mod_jcv::nwi()
{
   return mws_sp<mod_jcv>(new mod_jcv());
}

void mod_jcv::init()
{
}


namespace mod_jcv_ns
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

         const std::string& text = get_mod()->get_name();

         i_g->drawText(text, 10, 20);
      }
   };
}


void mod_jcv::init_mws()
{
   mws_root->new_page<mod_jcv_ns::main_page>();
   mws_cam->clear_color = true;
}

void mod_jcv::load()
{
}
