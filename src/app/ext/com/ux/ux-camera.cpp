#include "stdafx.h"

#include "ux-camera.hpp"
#include "ux-font.hpp"
#include "text-vxo.hpp"
#include "com/util/unicode/conversions-util.hpp"


namespace ns_ux_camera
{
	class draw_text_op : public draw_op
	{
	public:
		draw_text_op()
		{
			tx_vxo = text_vxo::new_inst();
		}

		void push_data(shared_ptr<rw_sequence> seq, const std::wstring& text, float ix, float iy, const shared_ptr<ux_font> ifont)
		{
			tx = text;
			x = ix;
			y = iy;
			font_idx = fonts.size();
			fonts.push_back(ifont);
			seq->w.write_pointer(this);
			write_data(seq);
		}

		virtual void read_data(shared_ptr<rw_sequence> seq)
		{
			int length = seq->r.read_uint32();
			std::vector<wchar_t> txt(length);
			seq->r.read_int8((int8*)begin_ptr(txt), length * sizeof(wchar_t), 0);
			tx = std::wstring(begin_ptr(txt), length);
			x = seq_util::read_float(seq);
			y = seq_util::read_float(seq);
			font_idx = seq->r.read_uint32();
		}

		virtual void write_data(shared_ptr<rw_sequence> seq)
		{
			seq->w.write_uint32(tx.length());
			seq->w.write_int8((int8*)tx.c_str(), tx.length() * sizeof(wchar_t), 0);
			seq_util::write_float(seq, x);
			seq_util::write_float(seq, y);
			seq->w.write_uint32(font_idx);
		}

		void on_update_start(shared_ptr<draw_context> idc)
		{
			tx_vxo->clear_text();
		}

		void draw(shared_ptr<draw_context> idc)
		{
			if (!tx.empty())
			{
				tx_vxo->add_text(tx, glm::vec2(x, y), fonts[font_idx]);
			}
		}

		void on_update_end(shared_ptr<draw_context> idc)
		{
			tx_vxo->render_mesh(idc->get_cam());
			fonts.clear();
			tx.clear();
		}

		shared_ptr<text_vxo> tx_vxo;
		std::wstring tx;
		float x;
		float y;
		int font_idx;
		// hold refs to the fonts, so they don't get destroyed before they're used for drawing
		std::vector<shared_ptr<ux_font> > fonts;
	};
}
using namespace ns_ux_camera;


class ux_camera_impl
{
public:
	ux_camera_impl()
	{
		font = ux_font::new_inst(24);
		font->set_color(gfx_color::colors::yellow);
	}

	shared_ptr<ux_font> font;
	gfx_color color;
	draw_text_op d_text;
};


shared_ptr<ux_camera> ux_camera::new_inst(std::shared_ptr<gfx> i_gi)
{
	shared_ptr<ux_camera> inst(new ux_camera(i_gi));
	inst->load(inst);
	return inst;
}

void ux_camera::drawImage(shared_ptr<gfx_tex> img, float x, float y, float width, float height)
{
	draw_image(img, x, y, width, height);
}

void ux_camera::drawLine(float i, float j, float k, float l)
{
	draw_line(glm::vec3(i, j, 0.f), glm::vec3(k, l, 0.f), p->color.to_vec4(), 1.f);
}

void ux_camera::drawRect(float x, float y, float width, float height)
{
	draw_line(glm::vec3(x, y, 0.f), glm::vec3(x + width, y, 0.f), p->color.to_vec4(), 1.f);
	draw_line(glm::vec3(x + width, y, 0.f), glm::vec3(x + width, y + height, 0.f), p->color.to_vec4(), 1.f);
	draw_line(glm::vec3(x + width, y + height, 0.f), glm::vec3(x, y + height, 0.f), p->color.to_vec4(), 1.f);
	draw_line(glm::vec3(x, y + height, 0.f), glm::vec3(x, y, 0.f), p->color.to_vec4(), 1.f);
}

void ux_camera::fillRect(float x, float y, float width, float height)
{
	draw_plane(glm::vec3(x + width * 0.5, y + height * 0.5, 0.f), glm::vec3(0, 0, 1), glm::vec2(width, height), p->color.to_vec4());
	//drawRect(x, y, width, height);
	//draw_line(glm::vec3(x, y, 0.f), glm::vec3(x + width, y, 0.f), color.to_vec4(), height / 2);
	//draw_line(glm::vec3(10, 50, 0.f), glm::vec3(100, 50, 0.f), ia_color::colors::blue.to_vec4(), 1.f);
}

shared_ptr<ux_font> ux_camera::get_font()const
{
	return p->font;
}

void ux_camera::set_font(shared_ptr<ux_font> ifont)
{
	p->font = ifont;
}

void ux_camera::drawText(const std::string& text, float x, float y, const shared_ptr<ux_font> ifnt)
{
	if (enabled)
	{
		std::wstring wtx = string2wstring(text);
		const shared_ptr<ux_font> fnt = (ifnt) ? ifnt : p->font;
		p->d_text.push_data(draw_ops, wtx, x, y, fnt);
	}
}

void ux_camera::drawText(const std::wstring& text, float x, float y, const shared_ptr<ux_font> ifnt)
{
	if (enabled)
	{
		const shared_ptr<ux_font> fnt = (ifnt) ? ifnt : p->font;
		p->d_text.push_data(draw_ops, text, x, y, fnt);
	}
}

void ux_camera::setColorf(float r, float g, float b, float a)
{
	p->color.from_float(r, g, b, a);
}

void ux_camera::setColor(uint8 r, uint8 g, uint8 b, uint8 a)
{
	p->color = gfx_color(r, g, b, a);
}

void ux_camera::setColor(int argb)
{
	p->color = gfx_color(argb);
}

void ux_camera::push_transform_state()
{

}

void ux_camera::pop_transform_state()
{

}

void ux_camera::rotate(float angle)
{

}

void ux_camera::scale(float sx, float sy)
{

}

void ux_camera::translate(float tx, float ty)
{

}

ux_camera::ux_camera(std::shared_ptr<gfx> i_gi) : gfx_camera(i_gi)
{
}

void ux_camera::load(shared_ptr<gfx_camera> inst)
{
	gfx_camera::load(inst);
	p = shared_ptr<ux_camera_impl>(new ux_camera_impl());
	projection_type = "orthographic";
	rendering_priority = 0xffff;
}

void ux_camera::update_camera_state()
{
	p->d_text.on_update_start(draw_ctx);
	gfx_camera::update_camera_state();
	p->d_text.on_update_end(draw_ctx);
}
