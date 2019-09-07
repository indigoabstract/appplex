#include "stdafx.hxx"

#include "mod-screen-metrix.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_screen_metrix::mod_screen_metrix() : mws_mod(mws_stringify(MOD_SCREEN_METRIX)) {}

mws_sp<mod_screen_metrix> mod_screen_metrix::nwi()
{
   return mws_sp<mod_screen_metrix>(new mod_screen_metrix());
}


namespace mod_screen_metrix_ns
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


void mod_screen_metrix::init_mws()
{
   mws_root->new_page<mod_screen_metrix_ns::main_page>();
   mws_cam->clear_color = true;
}
