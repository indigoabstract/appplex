#pragma once

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "../mws.hpp"


class gfx_quad_2d;
class vkb_voronoi_main;


class mws_vkb : public mws_virtual_keyboard
{
public:
   static mws_sp<mws_vkb> gi();
   virtual ~mws_vkb() {}
   virtual void receive(mws_sp<iadp> i_dp) override;
   virtual void on_resize() override;
   virtual void set_target(mws_sp<mws_text_box> i_tbx) override;
   void load(std::string i_filename);

protected:
   mws_vkb() {}
   virtual void setup() override;

   mws_sp<mws_text_box> tbx;
   mws_sp<vkb_voronoi_main> vk;
   std::string vkb_filename;
   static mws_sp<mws_vkb> inst;
};

#endif
