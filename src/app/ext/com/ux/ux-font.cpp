#include "stdafx.h"

#include "ux-font.hpp"
#include "font-db.hpp"
#include "gfx-color.hpp"
#include "min.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "com/util/unicode/conversions-util.hpp"
#include <glm/vec2.hpp>


class ux_font_impl
{
public:
	ux_font_impl(shared_ptr<ux_font> ifont, const std::string& ifont_path)
	{
		ux_font_ref = ifont;
		color = shared_ptr<gfx_color>(new gfx_color(gfx_color::colors::blue_violet));
		ppath = pfm_path::get_inst(ifont_path);
		font_name = font_db::inst()->get_db_font_name(ppath->get_file_name());
		font_path = ppath->get_full_path();
	}

	weak_ptr<ux_font> ux_font_ref;
	shared_ptr<pfm_path> ppath;
	shared_ptr<std::string> font_name;
	std::string font_path;
	float size;
	shared_ptr<gfx_color> color;
};


shared_ptr<ux_font> ux_font::new_inst(float isize, const std::string& ifont_path)
{
	shared_ptr<ux_font> font(new ux_font());
	std::string font_path = ifont_path.empty() ? "vera.ttf" : ifont_path;

	font->p = shared_ptr<ux_font_impl>(new ux_font_impl(font, font_path));
	font->p->size = isize;

	return font;
}

shared_ptr<ux_font> ux_font::get_inst()
{
	return shared_from_this();
}

const std::string& ux_font::get_file_name()const
{
	return *p->font_name;
}

const std::string& ux_font::get_full_path()const
{
	return p->font_path;
}

std::string ux_font::get_font_name()const
{
	return p->ppath->get_file_stem();
}

std::string ux_font::get_file_extension()const
{
	return p->ppath->get_file_extension();
}

const std::string& ux_font::get_root_directory()const
{
	return p->ppath->get_root_directory();
}

float ux_font::get_size()const
{
	return p->size;
}

float ux_font::get_ascender()
{
	return font_db::inst()->get_ascender(get_inst());
}

float ux_font::get_descender()
{
	return font_db::inst()->get_descender(get_inst());
}

float ux_font::get_height()
{
	return font_db::inst()->get_height(get_inst());
}

glm::vec2 ux_font::get_text_dim(const std::wstring& itext)
{
	return font_db::inst()->get_text_dim(get_inst(), itext);
}

glm::vec2 ux_font::get_text_dim(const std::string& itext)
{
	return get_text_dim(string2wstring(itext));
}

float ux_font::get_text_width(const std::wstring& itext)
{
	return get_text_dim(itext).x;
}

float ux_font::get_text_width(const std::string& itext)
{
	return get_text_width(string2wstring(itext));
}

float ux_font::get_text_height(const std::string& itext)
{
	return get_text_dim(itext).y;
}

const gfx_color& ux_font::get_color()const
{
	return *p->color;
}

void ux_font::set_color(const gfx_color& icolor)
{
	*p->color = icolor;
}


ux_font::ux_font()
{
	//vec4 yellow = { { 1, 1, 0, 1 } };
	//vec4 black = { { 0.0, 0.0, 0.0, 1.0 } };
	//vec4 none = { { 1.0, 1.0, 1.0, 0.0 } };
	//markup_t& m = fntm;

	//m.family = 0;
	//m.size = 24.0;
	//m.bold = 0;
	//m.italic = 0;
	//m.rise = 0.0;
	//m.spacing = 1.0;
	//m.gamma = 1.0;
	//m.foreground_color = yellow;
	//m.background_color = none;
	//m.outline = 0;
	//m.outline_color = black;
	//m.underline = 0;
	//m.underline_color = black;
	//m.overline = 0;
	//m.overline_color = black;
	//m.strikethrough = 0;
	//m.strikethrough_color = black;
	//m.font = 0;
}
