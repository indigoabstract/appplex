#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_VECTOR_FONTS

#include "text-vxo.hpp"
#include "com/mws/mws-font.hpp"
#include "com/mws/font-db.hpp"
#include "gfx.hpp"
#include "gfx-color.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-tex.hpp"
#include "gfx-state.hpp"
#include "gfx-vxo.hpp"
#include "pfm-gl.h"
#include <freetype-gl/vertex-buffer.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


struct vertex_t
{
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
	float shift, gamma;
};


class text_vxo_impl
{
public:
	text_vxo_impl(shared_ptr<text_vxo> inst)
	{
      mIs3D = false;
		text_vxo& ti = *inst;
		ti[MP_SHADER_NAME][MP_VSH_NAME] = "v3f-t2f-c4f.vert";
		ti[MP_SHADER_NAME][MP_FSH_NAME] = "v3f-t2f-c4f.frag";
		ti[MP_BLENDING] = MV_ALPHA;
      ti[MP_CULL_BACK] = true;
      ti[MP_CULL_FRONT] = false;
      ti[MP_DEPTH_WRITE] = false;
		ti[MP_DEPTH_TEST] = false;

		vbuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f,ashift:1f,agamma:1f");
      update_projection_mx();
      
      mws_report_gfx_errs();
	}

	~text_vxo_impl()
	{
		vertex_buffer_delete(vbuffer);
		vbuffer = nullptr;
	}

	void clear_text()
	{
		vertex_buffer_clear(vbuffer);
	}

	void add_text(const std::string& itext, const glm::vec2& ipos, const shared_ptr<mws_font> ifont)
	{
		auto& glyphs = font_db::inst()->get_glyph_vect(ifont->get_inst(), itext);
		glm::vec2 pen(ipos.x, ipos.y + ifont->get_ascender());

		add_text_2d_impl(vbuffer, glyphs, itext, pen, (float)gi()->rt.get_render_target_height(), ifont);
      //std::string text = wstring2string(itext);
      //add_text_2d(text, pen, ifont);
   }

	void render_mesh(shared_ptr<text_vxo> inst, shared_ptr<gfx_camera> icamera, const glm::vec3& ipos)
	{
		if (vbuffer->vertices->size == 0)
		{
			return;
		}

		gfx_material& mat = *inst->get_material();
		shared_ptr<gfx_tex> atlas = font_db::inst()->get_texture_atlas();

		if (atlas && atlas->is_valid())
		{
			shared_ptr<gfx_shader> shader = mat.get_shader();

			mat["texture"][MP_TEXTURE_INST] = atlas;
			inst->push_material_params(inst->get_material());
			icamera->update_glp_params(inst, shader);

         //model[3][0] = ipos.x;
         //model[3][1] = -ipos.y;

         if (mIs3D)
         {
            model[3][0] = mPosition.x;
            model[3][1] = mPosition.y;
            model[3][2] = mPosition.z;
         }
         else
         {
            model[3][0] = ipos.x;// mPosition.x;
            model[3][1] = -ipos.y;// gi()->rt.get_render_target_height() - mPosition.y;// -mFontHeight;
         }

         if (rt_width != gi()->rt.get_render_target_width() || rt_height != gi()->rt.get_render_target_height())
         {
            update_projection_mx();
         }

         shader->update_uniform("model", glm::value_ptr(model));
			shader->update_uniform("view", glm::value_ptr(view));
         shader->update_uniform("projection", glm::value_ptr(projection));

			vertex_buffer_render(vbuffer, GL_TRIANGLES);
		}

		mws_report_gfx_errs();
	}

   void add_text_2d(const std::string& Text, const glm::vec2& Pos, const std::shared_ptr<mws_font> Fnt)
   {
      auto& glyphs = font_db::inst()->get_glyph_vect(Fnt->get_inst(), Text);

      if (mIs3D)
      {
         clear_text();
         mIs3D = false;
      }

      mPosition = glm::vec3(Pos, 0);
      mFontHeight = Fnt->get_height();
      AddTextImpl(vbuffer, glyphs, Text, glm::vec3(1, 0, 0), glm::vec3(0, 1, 0), Fnt, 1.f);
   }

   void add_text_3d(const std::string& Text, const glm::vec3& position, const glm::vec3& HDir, const glm::vec3& VDir, std::shared_ptr<mws_font> Fnt)
   {
      auto& glyphs = font_db::inst()->get_glyph_vect(Fnt->get_inst(), Text);
      glm::vec2 pen(0.f);

      if (!mIs3D)
      {
         clear_text();
         mIs3D = true;
      }

      mPosition = position;
      mFontHeight = Fnt->get_height();
      AddTextImpl(vbuffer, glyphs, Text, HDir, VDir, Fnt, 0.05f);
   }

   void AddTextImpl(vertex_buffer_t* Buffer, const std::vector<font_glyph>& Glyphs, const std::string& Text,
      const glm::vec3& HDir, const glm::vec3& VDir, const std::shared_ptr<mws_font> Fnt, float Scale)
   {
      //auto crt_ctx = GraphicsContext::GetCurrentGraphicsContext();
      //UpdateParams(crt_ctx->GetWidth(), crt_ctx->GetHeight());

      int len = glm::min(Text.length(), Glyphs.size());
      glm::vec4 c = Fnt->get_color().to_vec4();
      float r = c.r, g = c.g, b = c.b, a = c.a;
      glm::vec2 pen(0.f);

      for (int i = 0; i < len; ++i)
      {
         font_glyph glyph = Glyphs[i];

         if (glyph.is_valid())
         {
            char ch = Text[i];

            // ignore carriage returns
            if (ch < ' ')
            {
               if (ch == '\n')
               {
                  pen.x = 0.f;
                  pen.y -= Fnt->get_height();
               }
               else if (ch == '\t')
               {
                  pen.x += 2 * Fnt->get_height();
               }
            }
            // normal character
            else
            {
               float kerning = 0.0f;

               if (i > 0)
               {
                  kerning = glyph.get_kerning(Text[i - 1]);
               }

               pen.x += kerning;
               glm::vec3 x0 = HDir * float(pen.x + glyph.get_offset_x()) * Scale;
               glm::vec3 y0 = VDir * float(pen.y + glyph.get_offset_y()) * Scale;
               glm::vec3 x1 = HDir * float(pen.x + glyph.get_offset_x() + glyph.get_width()) * Scale;
               glm::vec3 y1 = VDir * float(pen.y + glyph.get_offset_y() - glyph.get_height()) * Scale;
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

               vertex_buffer_push_back(Buffer, vertices, 4, indices, 6);
               pen.x += glyph.get_advance_x();
            }
         }
      }
   }
   
   void add_text_2d_impl(vertex_buffer_t* buffer, const std::vector<font_glyph>& glyphs, const std::string& text,
		const glm::vec2& ipen, float irt_height, const shared_ptr<mws_font> ifont)
	{
		int len = glm::min(text.length(), glyphs.size());
		glm::vec4 c = ifont->get_color().to_vec4();
		float r = c.r, g = c.g, b = c.b, a = c.a;
		glm::vec2 pen = ipen;

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
						pen.x = ipen.x;
						pen.y += ifont->get_height();
					}
					else if (ch == '\t')
					{
						pen.x += 2 * ifont->get_height();
					}
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
					float y0 = (float)(irt_height - pen.y + glyph.get_offset_y());
					float x1 = (float)(x0 + glyph.get_width());
					float y1 = (float)(y0 - glyph.get_height());
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
	}

   void update_projection_mx()
   {
      rt_width = gi()->rt.get_render_target_width();
      rt_height = gi()->rt.get_render_target_height();

      float left = 0;
      float right = (float)rt_width;
      float bottom = (float)rt_height;
      float top = 0;

      projection = glm::ortho(left, right, top, bottom, -100.f, 100.f);
   }

	vertex_buffer_t* vbuffer;
   glm::mat4 model;
   glm::mat4 view;
   glm::mat4 projection;
   int rt_width;
   int rt_height;
   glm::vec3 mPosition;
   float mFontHeight;
   bool mIs3D;
};


shared_ptr<text_vxo> text_vxo::new_inst()
{
	shared_ptr<text_vxo> inst(new text_vxo());
	inst->p = std::make_shared<text_vxo_impl>(inst);
	return inst;
}

void text_vxo::clear_text()
{
	p->clear_text();
}

void text_vxo::add_text(const std::string& itext, const glm::vec2& ipos, const shared_ptr<mws_font> ifont)
{
	p->add_text(itext, ipos, ifont);
}

void text_vxo::render_mesh(shared_ptr<gfx_camera> icamera)
{
	if (!visible)
	{
		return;
	}

	p->render_mesh(static_pointer_cast<text_vxo>(get_shared_ptr()), icamera, position());
}

text_vxo::text_vxo() : gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_v4_color, a_v1_shift, a_v1_gamma"))
{
}

#endif
