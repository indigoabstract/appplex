#pragma once

#include "gfx-vxo.hxx"
#include "mws-font.hxx"
#include <glm/fwd.hpp>


class mws_font;
class text_vxo_impl;


class text_vxo : public gfx_vxo, public mws_draw_text
{
public:
   static mws_sp<text_vxo> nwi();
   virtual void draw_text(const std::string& i_text, float i_x, float i_y, const mws_sp<mws_font> i_font = nullptr) override;
   virtual void clear_text();
   // returns the 4 coordinates of the text's bounding box
   virtual glm::vec4 add_text(const std::string& i_text, const glm::vec2& i_pos, const mws_sp<mws_font> i_font);
   virtual void draw_in_sync(mws_sp<gfx_camera> i_camera);

protected:
   text_vxo();

   mws_sp<text_vxo_impl> p;
};
