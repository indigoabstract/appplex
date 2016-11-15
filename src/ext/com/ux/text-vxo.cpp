#include "stdafx.h"

#include "text-vxo.hpp"
#include "com/ux/ux-font.hpp"
#include "com/ux/font-db.hpp"
#include "com/util/unicode/conversions-util.hpp"
#include "gfx.hpp"
#include "gfx-color.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-tex.hpp"
#include "gfx-state.hpp"
#include "gfx-vxo.hpp"
#include "pfmgl.h"
#include <freetype-gl/mat4.h>
#include <freetype-gl/vertex-buffer.h>
#include <glm/glm.hpp>


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
		text_vxo& ti = *inst;
		ti[MP_SHADER_NAME][MP_VSH_NAME] = "v3f-t2f-c4f.vert";
		ti[MP_SHADER_NAME][MP_FSH_NAME] = "v3f-t2f-c4f.frag";
		ti[MP_BLENDING] = MV_ALPHA;
		ti[MP_CULL_BACK] = true;
		ti[MP_DEPTH_WRITE] = false;
		ti[MP_DEPTH_TEST] = false;

		vbuffer = vertex_buffer_new("vertex:3f,tex_coord:2f,color:4f,ashift:1f,agamma:1f");

		mat4_set_identity(&projection);
		mat4_set_identity(&model);
		mat4_set_identity(&view);

		float left = 0;
		float right = gfx::rt::get_render_target_width();
		float bottom = gfx::rt::get_render_target_height();
		float top = 0;
		mat4_set_orthographic(&projection, 0, right, 0, bottom, -1, 1);

		gfx_util::check_gfx_error();
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

	void add_text(const std::wstring& itext, const glm::vec2& ipos, const shared_ptr<ux_font> ifont)
	{
		auto& glyphs = font_db::inst()->get_glyph_vect(ifont->get_inst(), itext);
		glm::vec2 pen(ipos.x, ipos.y + ifont->get_ascender());

		add_text_impl(vbuffer, glyphs, itext, pen, gfx::rt::get_render_target_height(), ifont);
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
			inst->push_material_params();
			icamera->update_glp_params(inst, shader);

			model.m30 = ipos.x;
			model.m31 = -ipos.y;
			shader->update_uniform("model", model.data);
			shader->update_uniform("view", view.data);
			shader->update_uniform("projection", projection.data);

			vertex_buffer_render(vbuffer, GL_TRIANGLES);
		}

		gfx_util::check_gfx_error();
	}

	void add_text_impl(vertex_buffer_t* buffer, const std::vector<font_glyph>& glyphs, const std::wstring& text,
		const glm::vec2& ipen, float irt_height, const shared_ptr<ux_font> ifont)
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
					float x0 = (int)(pen.x + glyph.get_offset_x());
					float y0 = (int)(irt_height - pen.y + glyph.get_offset_y());
					float x1 = (int)(x0 + glyph.get_width());
					float y1 = (int)(y0 - glyph.get_height());
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

	vertex_buffer_t* vbuffer;
	mat4 model, view, projection;
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

void text_vxo::add_text(const std::string& itext, const glm::vec2& ipos, const shared_ptr<ux_font> ifont)
{
	std::wstring tx = string2wstring(itext);
	p->add_text(tx, ipos, ifont);
}

void text_vxo::add_text(const std::wstring& itext, const glm::vec2& ipos, const shared_ptr<ux_font> ifont)
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

text_vxo::text_vxo() : gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord"))
{
}
