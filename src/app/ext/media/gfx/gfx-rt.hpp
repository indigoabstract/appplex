#pragma once

#include "gfx-obj.hpp"
#include "gfx-tex.hpp"
#include <memory>


class gfx_rt : public gfx_obj
{
public:
   ~gfx_rt();
   virtual gfx_obj::e_gfx_obj_type get_type()const;
   virtual bool is_valid()const;
   int get_width();
   int get_height();
   std::shared_ptr<gfx_tex> get_color_attachment();
   void set_color_attachment(std::shared_ptr<gfx_tex> icolor_att);
   void set_depth_stencil_attachment(std::shared_ptr<gfx_tex> idepth_stencil_att);
   void reload();
   void check_valid_state();

   unsigned int framebuffer;
   std::shared_ptr<gfx_tex> color_att;
   gfx_uint depth_buffer_id;
   bool is_valid_state;

protected:
   friend class gfx;
   gfx_rt(std::shared_ptr<gfx> i_gi = nullptr);
   void release();
};
