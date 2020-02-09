#include "stdafx.hxx"

#include "mws-font.hxx"
#include "mws/mws.hxx"
#include "mws-font-db.hxx"
#include "gfx-color.hxx"
#include "min.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include <glm/inc.hpp>


class mws_font_impl
{
public:
   mws_font_impl(mws_sp<mws_font> i_font, const std::string& i_font_path, const mws_font_markup* i_markup)
   {
      mws_font_ref = i_font;
      color = mws_sp<gfx_color>(new gfx_color(gfx_color::colors::white));
      ppath = mws_path(i_font_path);
      font_name = mws_font_db::inst()->get_db_font_name(ppath.filename());
      font_path = ppath.string();

      if (i_markup)
      {
         markup = *i_markup;
         has_markup = true;
      }
   }

   mws_wp<mws_font> mws_font_ref;
   mws_path ppath;
   mws_sp<std::string> font_name;
   std::string font_path;
   float size = 0.f;
   mws_sp<gfx_color> color;
   mws_font_markup markup;
   bool has_markup = false;
};


mws_sp<mws_font> mws_font::nwi(mws_sp<mws_font> i_fnt, float i_size, const mws_font_markup* i_markup)
{
   float size = (i_size > 0.f) ? i_size : i_fnt->p->size;
   return nwi(size, i_fnt->string_path(), i_markup);
}

mws_sp<mws_font> mws_font::nwi(float i_size, const std::string& i_font_path, const mws_font_markup* i_markup)
{
   mws_sp<mws_font> font(new mws_font());
   std::string font_path = i_font_path;

   font->p = mws_sp<mws_font_impl>(new mws_font_impl(font, font_path, i_markup));
   font->p->size = i_size;

   return font;
}

mws_sp<mws_font> mws_font::nwi(mws_sp<mws_font> i_fnt, const mws_dim& i_height, const mws_font_markup* i_markup)
{
   std::string path = i_fnt->string_path();

   if (path.empty())
   {
      path = mws_font_db::inst()->get_global_font()->string_path();
   }

   return nwi(path, i_height, i_markup);
}

mws_sp<mws_font> mws_font::nwi(const std::string& i_font_path, const mws_dim& i_height, const mws_font_markup* i_markup)
{
   mws_assert(i_height.to_pt().val() > 1.f);
   return mws_font_db::inst()->load_font_by_metrix(i_font_path, i_height, i_markup);
}

mws_sp<mws_font> mws_font::get_inst()
{
   return shared_from_this();
}

const std::string& mws_font::filename()const
{
   return *p->font_name;
}

const std::string& mws_font::string_path()const
{
   return p->font_path;
}

std::string mws_font::font_name()const
{
   return p->ppath.stem();
}

std::string mws_font::extension()const
{
   return p->ppath.extension();
}

mws_path mws_font::directory()const
{
   return p->ppath.directory();
}

float mws_font::get_size()const
{
   return p->size;
}

float mws_font::get_ascender()
{
   return mws_font_db::inst()->get_ascender(get_inst());
}

float mws_font::get_descender()
{
   return mws_font_db::inst()->get_descender(get_inst());
}

float mws_font::get_height()
{
   return mws_font_db::inst()->get_height(get_inst());
}

glm::vec2 mws_font::get_text_dim(const std::string& i_text)
{
   return mws_font_db::inst()->get_text_dim(get_inst(), i_text);
}

float mws_font::get_text_width(const std::string& i_text)
{
   return get_text_dim(i_text).x;
}

float mws_font::get_text_height(const std::string& i_text)
{
   return get_text_dim(i_text).y;
}

const gfx_color& mws_font::get_color()const
{
   return *p->color;
}

void mws_font::set_color(const gfx_color& icolor)
{
   *p->color = icolor;
}
bool mws_font::has_markup() const { return p->has_markup; }

const mws_font_markup& mws_font::get_markup() const { return p->markup; }


mws_font::mws_font()
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
