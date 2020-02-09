#pragma once

#include "pfm.hxx"
#include <glm/fwd.hpp>


class gfx_color;
class mws_font_impl;
class mws_font_cache;
class mws_font_db_impl;
class mws_dim;


enum class mws_font_rendermode
{
   e_normal,
   e_outline_edge,
   e_outline_positive,
   e_outline_negative,
   e_signed_distance_field
};


struct mws_font_markup
{
   mws_font_rendermode rendermode = mws_font_rendermode::e_normal;
   float outline_thickness = 0.f;
};


class mws_font : public std::enable_shared_from_this<mws_font>
{
public:
   static mws_sp<mws_font> nwi(mws_sp<mws_font> i_fnt, float i_size = 0.f, const mws_font_markup* i_markup = nullptr);
   static mws_sp<mws_font> nwi(float i_size, const std::string& i_font_path = "", const mws_font_markup* i_markup = nullptr);
   static mws_sp<mws_font> nwi(mws_sp<mws_font> i_fnt, const mws_dim& i_height, const mws_font_markup* i_markup = nullptr);
   static mws_sp<mws_font> nwi(const std::string& i_font_path, const mws_dim& i_height, const mws_font_markup* i_markup = nullptr);
   mws_sp<mws_font> get_inst();
   const std::string& filename()const;
   const std::string& string_path()const;
   std::string font_name()const;
   std::string extension()const;
   mws_path directory()const;
   float get_size()const;
   float get_ascender();
   float get_descender();
   float get_height();
   glm::vec2 get_text_dim(const std::string& i_text);
   float get_text_width(const std::string& i_text);
   float get_text_height(const std::string& i_text);
   const gfx_color& get_color()const;
   void set_color(const gfx_color& icolor);
   bool has_markup() const;
   const mws_font_markup& get_markup() const;

private:
   friend class mws_font_db_impl;
   mws_font();

   // cache font data for quick access
   mws_wp<mws_font_cache> fnt_cache;
   mws_sp<mws_font_impl> p;
};


// inherit from this to signal that a class can draw text..(lol!)
class mws_draw_text
{
public:
   virtual ~mws_draw_text() {}
   virtual void draw_text(const std::string& i_text, float i_x, float i_y, const mws_sp<mws_font> i_font = nullptr) = 0;
};
