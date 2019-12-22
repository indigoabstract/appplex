#pragma once

#include "gfx-vxo.hxx"
#include <glm/fwd.hpp>


class mws_font;
class text_vxo_impl;


class text_vxo : public gfx_vxo
{
public:
   static mws_sp<text_vxo> nwi();
   virtual void clear_text();
   virtual void add_text(const std::string& i_text, const glm::vec2& i_pos, const mws_sp<mws_font> i_font);
   virtual void draw_in_sync(mws_sp<gfx_camera> i_camera);

private:
   text_vxo();

   mws_sp<text_vxo_impl> p;
};
