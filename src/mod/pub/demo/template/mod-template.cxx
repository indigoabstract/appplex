#include "stdafx.hxx"

#include "mod-template.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_template::mod_template() : mws_mod(mws_stringify(MOD_TEMPLATE)) {}

mws_sp<mod_template> mod_template::nwi()
{
   return mws_sp<mod_template>(new mod_template());
}

void mod_template::init()
{
}


namespace
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

         const std::string& text = get_mod()->name();

         i_g->drawText(text, 10, 20);
      }
   };
}


void mod_template::init_mws()
{
   mws_root->new_page<main_page>();
   mws_cam->clear_color = true;
}

void mod_template::load()
{
}
