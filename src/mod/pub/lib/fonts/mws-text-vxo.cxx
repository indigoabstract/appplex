#include "stdafx.hxx"

#include "appplex-conf.hxx"

#if defined MOD_VECTOR_FONTS

#include "mws-text-vxo.hxx"
#include "mws-font.hxx"
#include "mws-font-db.hxx"
#include "gfx.hxx"
#include "gfx-color.hxx"
#include "gfx-camera.hxx"
#include "gfx-shader.hxx"
#include "gfx-tex.hxx"
#include "gfx-state.hxx"
#include "gfx-vxo.hxx"
#include "pfm-gl.h"
#include <freetype-gl/vertex-buffer.h>
#include <glm/inc.hpp>


using namespace ftgl;


struct vertex_t
{
   float x, y, z;    // position
   float s, t;       // texture
   float r, g, b, a; // color
};


class mws_text_vxo_impl
{
public:
   mws_text_vxo_impl(mws_sp<mws_text_vxo> i_inst)
   {
      mIs3D = false;
      mws_text_vxo& ti = *i_inst;
      ti[MP_SHADER_NAME] = "text-shader";
      ti[MP_BLENDING] = MV_ALPHA;
      ti[MP_CULL_BACK] = true;
      ti[MP_CULL_FRONT] = false;
      ti[MP_DEPTH_WRITE] = false;
      ti[MP_DEPTH_TEST] = false;

      vbuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f");
      update_projection_mx();

      mws_report_gfx_errs();
   }

   ~mws_text_vxo_impl()
   {
      vertex_buffer_delete(vbuffer);
      vbuffer = nullptr;
   }

   void clear_text()
   {
      vertex_buffer_clear(vbuffer);
   }

   glm::vec4 add_text(const std::string& i_text, const glm::vec2& i_pos, const mws_sp<mws_font> i_font)
   {
      glm::vec2 pen(i_pos.x, i_pos.y + i_font->get_ascender());
      auto& glyphs = mws_font_db::inst()->get_glyph_vect(i_font->get_inst(), i_text);

      return add_text_2d_impl(vbuffer, glyphs, i_text, pen, (float)gfx::i()->rt.get_render_target_height(), i_font);
      //std::string text = wstring2string(i_text);
      //add_text_2d(text, pen, i_font);
   }

   void draw_in_sync(mws_sp<mws_text_vxo> i_inst, mws_sp<gfx_camera> i_camera, const glm::vec3& i_pos)
   {
      if (vbuffer->vertices->size == 0)
      {
         return;
      }

      gfx_material& mat = *i_inst->get_material();
      mws_sp<gfx_tex> atlas = mws_font_db::inst()->get_texture_atlas();

      if (atlas && atlas->is_valid())
      {
         mws_report_gfx_errs();
         mws_sp<gfx_shader> shader = mat.get_shader();

         mat["texture"][MP_TEXTURE_INST] = atlas;
         i_inst->push_material_params(i_inst->get_material());
         i_camera->update_glp_params(i_inst, shader);

         //model[3][0] = i_pos.x;
         //model[3][1] = -i_pos.y;

         if (mIs3D)
         {
            model[3][0] = mPosition.x;
            model[3][1] = mPosition.y;
            model[3][2] = mPosition.z;
         }
         else
         {
            model[3][0] = glm::round(i_pos.x);// mPosition.x;
            model[3][1] = glm::round(-i_pos.y);// gi()->rt.get_render_target_height() - mPosition.y;// -mFontHeight;
         }

         if (rt_width != i_inst->gi()->rt.get_render_target_width() || rt_height != i_inst->gi()->rt.get_render_target_height())
         {
            update_projection_mx();
         }

         shader->update_uniform("model", glm::value_ptr(model));
         shader->update_uniform("view", glm::value_ptr(view));
         shader->update_uniform("projection", glm::value_ptr(projection));
         mws_report_gfx_errs();

         vertex_buffer_render(vbuffer, GL_TRIANGLES);
         mws_report_gfx_errs();
      }
   }

   void add_text_2d(const std::string& i_text, const glm::vec2& i_position, const mws_sp<mws_font> i_font)
   {
      auto& glyphs = mws_font_db::inst()->get_glyph_vect(i_font->get_inst(), i_text);

      if (mIs3D)
      {
         clear_text();
         mIs3D = false;
      }

      mPosition = glm::vec3(i_position, 0);
      mFontHeight = i_font->get_height();
      AddTextImpl(vbuffer, glyphs, i_text, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), i_font, 1.f);
   }

   void add_text_3d(const std::string& i_text, const glm::vec3& i_position, const glm::vec3& i_hdir, const glm::vec3& i_vdir, mws_sp<mws_font> i_font)
   {
      auto& glyphs = mws_font_db::inst()->get_glyph_vect(i_font->get_inst(), i_text);
      glm::vec2 pen(0.f);

      if (!mIs3D)
      {
         clear_text();
         mIs3D = true;
      }

      mPosition = i_position;
      mFontHeight = i_font->get_height();
      AddTextImpl(vbuffer, glyphs, i_text, i_hdir, i_vdir, i_font, 0.05f);
   }

   void AddTextImpl(vertex_buffer_t* i_buffer, const std::vector<font_glyph>& i_glyphs, const std::string& i_text,
      const glm::vec3& i_hdir, const glm::vec3& i_vdir, const mws_sp<mws_font> i_font, float i_scale)
   {
      //auto crt_ctx = GraphicsContext::GetCurrentGraphicsContext();
      //UpdateParams(crt_ctx->GetWidth(), crt_ctx->GetHeight());

      int len = glm::min(i_text.length(), i_glyphs.size());
      glm::vec4 c = i_font->get_color().to_vec4();
      float r = c.r, g = c.g, b = c.b, a = c.a;
      glm::vec2 pen(0.f);

      for (int i = 0; i < len; ++i)
      {
         font_glyph glyph = i_glyphs[i];

         if (glyph.is_valid())
         {
            char ch = i_text[i];

            // ignore carriage returns
            if (ch < ' ')
            {
               if (ch == '\n')
               {
                  pen.x = 0.f;
                  pen.y -= i_font->get_height();
               }
               else if (ch == '\t')
               {
                  pen.x += 2 * i_font->get_height();
               }
            }
            // normal character
            else
            {
               float kerning = 0.0f;

               if (i > 0)
               {
                  kerning = glyph.get_kerning(i_text[i - 1]);
               }

               pen.x += kerning;
               glm::vec3 x0 = i_hdir * float(pen.x + glyph.get_offset_x()) * i_scale;
               glm::vec3 y0 = i_vdir * float(pen.y + glyph.get_offset_y()) * i_scale;
               glm::vec3 x1 = i_hdir * float(pen.x + glyph.get_offset_x() + glyph.get_width()) * i_scale;
               glm::vec3 y1 = i_vdir * float(pen.y + glyph.get_offset_y() - glyph.get_height()) * i_scale;
               float s0 = glyph.get_s0();
               float t0 = glyph.get_t0();
               float s1 = glyph.get_s1();
               float t1 = glyph.get_t1();
               //GLuint indices[6] = { 0, 2, 1, 2, 0, 3 };
               GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
               vertex_t vertices[4] =
               {
                  { (x0 + y0).x, (x0 + y0).y, (x0 + y0).z, s0, t0, r, g, b, a },
                  { (x0 + y1).x, (x0 + y1).y, (x0 + y1).z, s0, t1, r, g, b, a },
                  { (x1 + y1).x, (x1 + y1).y, (x1 + y1).z, s1, t1, r, g, b, a },
                  { (x1 + y0).x, (x1 + y0).y, (x1 + y0).z, s1, t0, r, g, b, a }
               };

               vertex_buffer_push_back(i_buffer, vertices, 4, indices, 6);
               pen.x += glyph.get_advance_x();
            }
         }
      }
   }

   glm::vec4 add_text_2d_impl(vertex_buffer_t* buffer, const std::vector<font_glyph>& glyphs, const std::string& text,
      const glm::vec2& i_pen, float i_rt_height, const mws_sp<mws_font> i_font)
   {
      glm::vec4 ret_val(i_pen, i_pen);
      int len = glm::min(text.length(), glyphs.size());
      glm::vec4 c = i_font->get_color().to_vec4();
      float r = c.r, g = c.g, b = c.b, a = c.a;
      glm::vec2 pen = i_pen;

      for (int i = 0; i < len; ++i)
      {
         font_glyph glyph = glyphs[i];

         if (glyph.is_valid())
         {
            char ch = text[i];

            // ignore carriage returns
            if (ch < ' ')
            {
               if (ch == '\n')
               {
                  pen.x = i_pen.x;
                  pen.y += i_font->get_height();
               }
               else if (ch == '\t')
               {
                  pen.x += 2 * i_font->get_height();
               }

               ret_val.z = glm::max(ret_val.z, pen.x);
            }
            // normal character
            else
            {
               float kerning = 0.0f;
               if (i > 0)
               {
                  kerning = glyph.get_kerning(text[i - 1]);
               }
               pen.x += kerning;
               float x0 = (float)(pen.x + glyph.get_offset_x());
               float y0 = (float)(i_rt_height - pen.y + glyph.get_offset_y());
               float x1 = float((int)(x0 + glyph.get_width())); x0 = float((int)x0);
               float y1 = float((int)(y0 - glyph.get_height())); y0 = float((int)y0);
               float s0 = glyph.get_s0();
               float t0 = glyph.get_t0();
               float s1 = glyph.get_s1();
               float t1 = glyph.get_t1();
               //GLuint indices[6] = { 1, 0, 2, 2, 0, 3 };
               GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
               vertex_t vertices[4] =
               {
                  { x0, y0, 0, s0, t0, r, g, b, a },
                  { x0, y1, 0, s0, t1, r, g, b, a },
                  { x1, y1, 0, s1, t1, r, g, b, a },
                  { x1, y0, 0, s1, t0, r, g, b, a }
               };
               vertex_buffer_push_back(buffer, vertices, 4, indices, 6);
               pen.x += glyph.get_advance_x();
            }
         }
      }

      ret_val.z = glm::max(ret_val.z, pen.x);
      ret_val.w = pen.y;

      return ret_val;
   }

   void update_projection_mx()
   {
      rt_width = gfx::i()->rt.get_render_target_width();
      rt_height = gfx::i()->rt.get_render_target_height();

      float left = 0;
      float right = (float)rt_width;
      float bottom = (float)rt_height;
      float top = 0;

      projection = glm::ortho(left, right, top, bottom, -100.f, 100.f);
   }

   vertex_buffer_t* vbuffer;
   glm::mat4 model = glm::mat4(1.f);
   glm::mat4 view = glm::mat4(1.f);
   glm::mat4 projection = glm::mat4(1.f);
   int rt_width;
   int rt_height;
   glm::vec3 mPosition;
   float mFontHeight;
   bool mIs3D;
};


mws_sp<mws_text_vxo> mws_text_vxo::nwi()
{
   mws_sp<mws_text_vxo> inst(new mws_text_vxo());
   inst->p = std::make_shared<mws_text_vxo_impl>(inst);
   return inst;
}

void mws_text_vxo::draw_text(const std::string& i_text, float i_x, float i_y, const mws_sp<mws_font> i_font)
{
   add_text(i_text, glm::vec2(i_x, i_y), i_font);
}

void mws_text_vxo::clear_text()
{
   p->clear_text();
}

glm::vec4 mws_text_vxo::add_text(const std::string& i_text, const glm::vec2& i_pos, const mws_sp<mws_font> i_font)
{
   return p->add_text(i_text, i_pos, i_font);
}

void mws_text_vxo::draw_in_sync(mws_sp<gfx_camera> i_camera)
{
   if (!visible)
   {
      return;
   }

   p->draw_in_sync(static_pointer_cast<mws_text_vxo>(get_mws_sp()), i_camera, position);
}

mws_text_vxo::mws_text_vxo() : gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_v4_color"))
{
}

#endif
