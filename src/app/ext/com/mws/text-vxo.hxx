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
   virtual void add_text(const std::string& itext, const glm::vec2& ipos, const mws_sp<mws_font> ifont);
   virtual void draw_in_sync(mws_sp<gfx_camera> icamera);

private:
   text_vxo();

   mws_sp<text_vxo_impl> p;
};
