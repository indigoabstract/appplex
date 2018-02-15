#include "stdafx.h"

#include "gfx-rt.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "min.hpp"
#include "pfmgl.h"


gfx_rt::~gfx_rt()
{
   release();
}

gfx_obj::e_gfx_obj_type gfx_rt::get_type()const
{
   return e_gfx_rt;
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

std::shared_ptr<gfx_tex> gfx_rt::get_color_attachment()
{
   return color_att;
}

void gfx_rt::set_color_attachment(std::shared_ptr<gfx_tex> icolor_att)
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

void gfx_rt::reload()
{
   glGenFramebuffers(1, &framebuffer);
   gfx_util::check_gfx_error();
}

void gfx_rt::check_valid_state()
{
   if (!is_valid_state)
   {
      throw ia_exception("the render target is not in a valid state");
   }
}

gfx_rt::gfx_rt(std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   glGenFramebuffers(1, &framebuffer);
   is_valid_state = false;
   glGenRenderbuffers(1, &depth_buffer_id);
   gfx_util::check_gfx_error();
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

   gfx::remove_gfx_obj(this);
}
