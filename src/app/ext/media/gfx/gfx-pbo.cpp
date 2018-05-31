#include "stdafx.h"

#include "appplex-conf.hpp"
#include "gfx.hpp"
#include "gfx-pbo.hpp"
#include "gfx-tex.hpp"
#include "gfx-rt.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-util.hpp"
#include "pfm-gl.h"


std::shared_ptr<gfx_readback> gfx_readback::nwi(std::shared_ptr<gfx> i_gi)
{
   return std::shared_ptr<gfx_readback>(new gfx_readback(i_gi));
}

gfx_obj::e_gfx_obj_type gfx_readback::get_type()const
{
   return gfx_obj::e_obj;
}

int gfx_readback::get_pbo_size() const
{
   return width * height * ti->get_bpp();
}

void gfx_readback::rewind()
{
   pbo_index = 0;
   frame_idx = 0;
}

void gfx_readback::set_params(int i_width, int i_height, std::string i_format)
{
   width = i_width;
   height = i_height;
   ti = gfx_util::get_tex_info(i_format);
   pbo_data_size = get_pbo_size();
   //pbo_data.resize(pbo_data_size);

   for (uint32 k = 0; k < pbo_count; k++)
   {
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[k]);
      glBufferData(GL_PIXEL_PACK_BUFFER, pbo_data_size, 0, GL_STREAM_READ);
   }

   glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
   rewind();
}

void gfx_readback::update()
{
   mws_report_gfx_errs();
   glReadBuffer(GL_COLOR_ATTACHMENT0);

   // copy pixels from framebuffer to PBO and use offset instead of pointer.
   // OpenGL should perform async DMA transfer, so glReadPixels() will return immediately.
   glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[pbo_index]);
   mws_report_gfx_errs();
   glPixelStorei(GL_PACK_ALIGNMENT, ti->get_bpp());
   mws_report_gfx_errs();
   glReadPixels(0, 0, width, height, ti->get_format(), ti->get_type(), 0);

   mws_report_gfx_errs();
   if (frame_idx >= pbo_count - 1)
   {
      // pbo_next_index is used to process pixels in the other PBO
      int pbo_next_index = (pbo_index + 1) % pbo_count;

      // map the PBO containing the framebuffer pixels before processing it
      glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[pbo_next_index]);
      // glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, pbo_data.size(), pbo_data.data());
      GLubyte* src = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pbo_data_size, GL_MAP_READ_BIT);

      mws_report_gfx_errs();
      if (src)
      {
         if (on_data_recv_handler)
         {
            on_data_recv_handler(src, pbo_data_size);
         }
      }

      // release pointer to the mapped buffer
      glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
      mws_report_gfx_errs();
      glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
      mws_report_gfx_errs();
   }

   // increment current index first then get the next index
   // pbo_index is used to read pixels from a framebuffer to a PBO
   pbo_index = (pbo_index + 1) % pbo_count;
   frame_idx++;
}

gfx_readback::gfx_readback(std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi)
{
   set_pbo_count(2);
}

void gfx_readback::set_dimensions(int i_width, int i_height)
{
   mws_throw ia_exception("n/a");
}

void gfx_readback::set_pbo_count(int i_pbo_count)
{
   pbo_count = i_pbo_count;
   pbo_id_vect.resize(pbo_count);
   glGenBuffers(pbo_count, pbo_id_vect.data());
}


mws_pbo_bundle::mws_pbo_bundle(mws_sp<gfx> i_gi, int i_width, int i_height, std::string i_format)
{
   gfx_tex_params prm;

   prm.set_format_id(i_format);
   prm.set_rt_params();
   rt = i_gi->rt.new_rt();
   rt_tex = i_gi->tex.nwi(gfx_tex::gen_id(), i_width, i_height, &prm);
   rt->set_color_attachment(rt_tex);
   readback = gfx_readback::nwi(i_gi);
   readback->set_params(i_width, i_height, i_format);
   rt_quad = gfx_quad_2d::nwi(i_gi);
   rt_quad->set_dimensions(1.f, 1.f);
   rt_quad->set_scale((float)i_width, (float)i_height);
}

void mws_pbo_bundle::set_on_data_recv_handler(std::function<void(gfx_ubyte* i_data, int i_size)> i_handler)
{
   readback->on_data_recv_handler = i_handler;
}

void mws_pbo_bundle::set_tex(std::shared_ptr<gfx_tex> i_tex)
{
   (*rt_quad)["u_s2d_tex"][MP_TEXTURE_INST] = i_tex;
}

void mws_pbo_bundle::update(std::shared_ptr<gfx_camera> i_cam)
{
   gfx::i()->rt.set_current_render_target(rt);
   rt_quad->draw_out_of_sync(i_cam);
   //gfx::i()->rt.get_render_target_pixels<uint8>(rt_y, pixels_y_tex);
   //helper::read_pixels_helper(pbo_supported, rt_y_tex, y_pbo_ids[pbo_index], y_pbo_ids[pbo_next_index], pixels_y_tex);
   readback->update();
   mws_report_gfx_errs();

   gfx::i()->rt.set_current_render_target();
}
