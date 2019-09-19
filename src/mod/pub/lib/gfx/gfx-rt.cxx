#include "stdafx.hxx"

#include "gfx-rt.hxx"
#include "gfx.hxx"
#include "gfx-util.hxx"
#include "pfm.hxx"
#include "pfm-gl.h"


gfx_rt::~gfx_rt()
{
   release();
}

gfx_obj::e_gfx_obj_type gfx_rt::get_type()const
{
   return e_rt;
}

bool gfx_rt::is_valid()const
{
   return is_valid_state;
}

int gfx_rt::get_width()
{
   check_valid_state();
   return color_att->get_width();
}

int gfx_rt::get_height()
{
   check_valid_state();
   return color_att->get_height();
}

mws_sp<gfx_tex> gfx_rt::get_color_attachment()
{
   return color_att;
}

void gfx_rt::set_color_attachment(mws_sp<gfx_tex> icolor_att)
{
   color_att = icolor_att;
   is_valid_state = false;

   if (color_att && color_att->is_valid())
   {
      is_valid_state = true;
      glBindRenderbuffer(GL_RENDERBUFFER, depth_buffer_id);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, get_width(), get_height());
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
   }
}

void gfx_rt::clear_buffers(bool i_clear_color, bool i_clear_depth, bool i_clear_stencil, gfx_color i_clear_color_val)
{
   mws_assert((color_att) ? (gfx::i()->rt.get_current_render_target() == get_inst()) : true);

   gfx_bitfield bf = 0;

   if (i_clear_color)
   {
      glm::vec4 cc = i_clear_color_val.to_vec4();

      bf |= GL_COLOR_BUFFER_BIT;
      glClearColor(cc.r, cc.g, cc.b, cc.a);
   }

   if (i_clear_depth)
   {
      bf |= GL_DEPTH_BUFFER_BIT;
   }

   if (i_clear_stencil)
   {
      bf |= GL_STENCIL_BUFFER_BIT;
   }

   if (bf != 0)
   {
      glClear(bf);
   }
}

void gfx_rt::reload()
{
   glGenFramebuffers(1, &framebuffer);
   mws_report_gfx_errs();
}

void gfx_rt::check_valid_state()
{
   if (!is_valid_state)
   {
      mws_throw mws_exception("the render target is not in a valid state");
   }
}

gfx_rt::gfx_rt(mws_sp<gfx> i_gi) : gfx_obj(i_gi)
{
   glGenFramebuffers(1, &framebuffer);
   is_valid_state = false;
   glGenRenderbuffers(1, &depth_buffer_id);
   mws_report_gfx_errs();
}

void gfx_rt::release()
{
   color_att = nullptr;

   if (framebuffer != 0)
   {
      glDeleteFramebuffers(1, &framebuffer);
      framebuffer = 0;
   }

   if (depth_buffer_id != 0)
   {
      glDeleteRenderbuffers(1, &depth_buffer_id);
      depth_buffer_id = 0;
   }

   gi()->remove_gfx_obj(this);
}
