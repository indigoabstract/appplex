#pragma once

#include "pfm.hxx"
#include "mws-font.hxx"
#include <glm/fwd.hpp>
#include <utility>


namespace ftgl
{
   struct texture_glyph_t;
}
class mws_mod;
class gfx_tex;
class mws_font_db_impl;
class mws_dim;
class mws_pt;
class mws_px;


class font_glyph
{
public:
   font_glyph();

   bool is_valid() const;
   /**
   * character this glyph represents
   */
   uint32_t get_charcode()const;
   /**
   * Glyph's width in pixels.
   */
   size_t get_width()const;
   /**
   * Glyph's height in pixels.
   */
   size_t get_height()const;
   /**
   * Glyph's left bearing expressed in integer pixels.
   */
   int get_offset_x()const;
   /**
   * Glyphs's top bearing expressed in integer pixels.
   *
   * Remember that this is the distance from the baseline to the top-most
   * glyph scanline, upwards y coordinates being positive.
   */
   int get_offset_y()const;
   /**
   * For horizontal text layouts, this is the horizontal distance (in
   * fractional pixels) used to increment the pen position when the glyph is
   * drawn as part of a string of text.
   */
   float get_advance_x()const;
   /**
   * For vertical text layouts, this is the vertical distance (in fractional
   * pixels) used to increment the pen position when the glyph is drawn as
   * part of a string of text.
   */
   float get_advance_y()const;
   /**
   * First normalized texture coordinate (x) of top-left corner
   */
   float get_s0()const;
   /**
   * Second normalized texture coordinate (y) of top-left corner
   */
   float get_t0()const;
   /**
   * First normalized texture coordinate (x) of bottom-right corner
   */
   float get_s1()const;
   /**
   * Second normalized texture coordinate (y) of bottom-right corner
   */
   float get_t1()const;
   /**
   * A vector of kerning pairs relative to this glyph.
   */
   float get_kerning(char i_charcode)const;
   /**
   * get rendermode: RENDER_NORMAL, RENDER_OUTLINE_EDGE, RENDER_OUTLINE_POSITIVE, RENDER_OUTLINE_NEGATIVE, RENDER_SIGNED_DISTANCE_FIELD
   */
   mws_font_rendermode get_rendermode()const;
   /**
   * Glyph outline thickness
   */
   float get_outline_thickness()const;

private:
   friend class mws_font_db_impl;

   font_glyph(ftgl::texture_glyph_t* i_glyph);

   ftgl::texture_glyph_t* glyph;
};


class mws_font_db
{
public:
   static const std::string default_font_name;
   static mws_sp<mws_font_db> nwi_inex(uint32 i_pow_of_two_db_size);
   static mws_sp<mws_font_db> inst();
   void clear_db();
   void resize_db(uint32 i_pow_of_two);
   const mws_sp<mws_font> get_global_font() const;
   void set_global_font(const std::string& i_font_name, float i_size = 0.f);
   void set_global_font(mws_sp<mws_font> i_font);
   mws_sp<std::string> get_db_font_name(const std::string& i_font_name);
   const std::vector<font_glyph>& get_glyph_vect(mws_sp<mws_font> i_font, const std::string& i_text);
   mws_sp<gfx_tex> get_texture_atlas();
   float get_ascender(mws_sp<mws_font> i_font);
   float get_descender(mws_sp<mws_font> i_font);
   float get_height(mws_sp<mws_font> i_font);
   glm::vec2 get_text_dim(mws_sp<mws_font> i_font, const std::string& i_text);
   void store_font_metrix(const std::string& i_font_path, const mws_pt& i_min_height_pt, const mws_px& i_min_height_px,
      const mws_pt& i_max_height_pt, const mws_px& i_max_height_px, const std::pair<float, float>* i_pixels_to_points_data, uint32 i_data_elem_count);
   mws_sp<mws_font> load_font_by_metrix(const std::string& i_font_path, const mws_dim& i_height, const mws_font_markup* i_markup = nullptr);

private:
   friend class mws_mod;
   mws_font_db(uint32 i_pow_of_two);
   void on_frame_start();

   static mws_sp<mws_font_db> instance;
   mws_sp<mws_font_db_impl> p;
};
