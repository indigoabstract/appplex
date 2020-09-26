#pragma once

#include "gfx-vxo.hxx"
#include "mws-font.hxx"
mws_push_disable_all_warnings
#include <glm/fwd.hpp>
mws_pop_disable_all_warnings


class mws_font;
class mws_text_vxo_impl;


class mws_text_vxo : public gfx_vxo, public mws_draw_text
{
public:
   static mws_sp<mws_text_vxo> nwi();
   virtual void draw_text(const std::string& i_text, float i_x, float i_y, const mws_sp<mws_font> i_font = nullptr) override;
   virtual void clear_text();
   // returns the 4 coordinates of the text's bounding box
   virtual glm::vec4 add_text(const std::string& i_text, const glm::vec2& i_pos, const mws_sp<mws_font> i_font);
   virtual void draw_in_sync(mws_sp<gfx_camera> i_camera);

protected:
   mws_text_vxo();

   mws_sp<mws_text_vxo_impl> p;
};
