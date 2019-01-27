#pragma once

#include "pfm.hpp"
#include <glm/fwd.hpp>


class unit;
struct texture_glyph_t;
class gfx_tex;
class mws_font;
class font_db_impl;


class font_glyph
{
public:
   font_glyph();

   bool is_valid();
   /**
   * Wide character this glyph represents
   */
   wchar_t get_charcode()const;
   /**
   * Glyph id (used for display lists)
   */
   unsigned int get_id()const;
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
   float get_kerning(wchar_t icharcode)const;
   /**
   * Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
   */
   int get_outline_type()const;
   /**
   * Glyph outline thickness
   */
   float get_outline_thickness()const;

private:
   friend class font_db_impl;

   font_glyph(texture_glyph_t* iglyph);

   texture_glyph_t* glyph;
};


class font_db
{
public:
   static mws_sp<font_db> inst();
   void clear_db();
   void resize_db(int ipow_of_two);
   mws_sp<std::string> get_db_font_name(const std::string& ifont_name);
   const std::vector<font_glyph>& get_glyph_vect(mws_sp<mws_font> ifont, const std::string& itext);
   mws_sp<gfx_tex> get_texture_atlas();
   float get_ascender(mws_sp<mws_font> ifont);
   float get_descender(mws_sp<mws_font> ifont);
   float get_height(mws_sp<mws_font> ifont);
   glm::vec2 get_text_dim(mws_sp<mws_font> ifont, const std::string& itext);

private:
   friend class unit;
   font_db();
   void on_frame_start();

   static mws_sp<font_db> instance;
   mws_sp<font_db_impl> p;
};
