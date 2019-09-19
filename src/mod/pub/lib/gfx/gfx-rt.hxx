#pragma once

#include "gfx-obj.hxx"
#include "gfx-color.hxx"
#include "gfx-tex.hxx"
#include <memory>


class gfx_rt : public gfx_obj
{
public:
   ~gfx_rt();
   virtual gfx_obj::e_gfx_obj_type get_type()const;
   virtual bool is_valid()const;
   int get_width();
   int get_height();
   mws_sp<gfx_tex> get_color_attachment();
   void set_color_attachment(mws_sp<gfx_tex> icolor_att);
   void set_depth_stencil_attachment(mws_sp<gfx_tex> idepth_stencil_att);
   void clear_buffers(bool i_clear_color = true, bool i_clear_depth = true, bool i_clear_stencil = true, gfx_color i_clear_color_val = gfx_color::colors::black);
   void reload();
   void check_valid_state();

   unsigned int framebuffer;
   mws_sp<gfx_tex> color_att;
   gfx_uint depth_buffer_id;
   bool is_valid_state;

protected:
   friend class gfx;
   gfx_rt(mws_sp<gfx> i_gi = nullptr);
   void release();
};
