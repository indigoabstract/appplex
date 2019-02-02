#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_FONTS

#include "font-db.hpp"
#include "mws-font.hpp"
#include "min.hpp"
#include <glm/inc.hpp>

mws_sp<font_db> font_db::instance;

mws_sp<font_db> font_db::inst()
{
   if (!instance)
   {
      instance = mws_sp<font_db>(new font_db());
   }

   return instance;
}

#if defined MOD_VECTOR_FONTS

#include "gfx.hpp"
#include "gfx-tex.hpp"
#include <freetype-gl/freetype-gl.h>


font_glyph::font_glyph()
{
   glyph = nullptr;
}

bool font_glyph::is_valid()
{
   return glyph != nullptr;
}

wchar_t font_glyph::get_charcode()const
{
   return glyph->charcode;
}

unsigned int font_glyph::get_id()const
{
   return glyph->id;
}

size_t font_glyph::get_width()const
{
   return glyph->width;
}

size_t font_glyph::get_height()const
{
   return glyph->height;
}

int font_glyph::get_offset_x()const
{
   return glyph->offset_x;
}

int font_glyph::get_offset_y()const
{
   return glyph->offset_y;
}

float font_glyph::get_advance_x()const
{
   return glyph->advance_x;
}

float font_glyph::get_advance_y()const
{
   return glyph->advance_y;
}

float font_glyph::get_s0()const
{
   return glyph->s0;
}

float font_glyph::get_t0()const
{
   return glyph->t0;
}

float font_glyph::get_s1()const
{
   return glyph->s1;
}

float font_glyph::get_t1()const
{
   return glyph->t1;
}

float font_glyph::get_kerning(wchar_t icharcode)const
{
   return texture_glyph_get_kerning(glyph, icharcode);
}

int font_glyph::get_outline_type()const
{
   return glyph->outline_type;
}

float font_glyph::get_outline_thickness()const
{
   return glyph->outline_thickness;
}

font_glyph::font_glyph(texture_glyph_t* iglyph)
{
   glyph = iglyph;
}


class font_cache
{
public:
   const int inf_lim = 32;
   const int sup_lim = 126;

   font_cache(texture_font_t* itex_font, mws_sp<std::vector<uint8> > ifont_mem_data)
   {
      tex_font = itex_font;
      font_mem_data = ifont_mem_data;
      glyph_vect.resize(sup_lim - inf_lim + 1);
   }

   ~font_cache()
   {
      if (tex_font)
      {
         texture_font_delete(tex_font);
         tex_font = nullptr;
      }
   }

   texture_font_t* get_tex_font()
   {
      return tex_font;
   }

   mws_sp<std::vector<uint8> >get_font_data()
   {
      return font_mem_data;
   }

   font_glyph get_glyph_at(int idx)
   {
      if (idx >= inf_lim && idx <= sup_lim)
      {
         return glyph_vect[idx - inf_lim];
      }

      return glyph_ht[idx];
   }

   void set_glyph_at(font_glyph iglyph, int idx)
   {
      if (idx >= inf_lim && idx <= sup_lim)
      {
         glyph_vect[idx - inf_lim] = iglyph;
      }

      glyph_ht[idx] = iglyph;
   }

private:
   texture_font_t * tex_font;
   mws_sp<std::vector<uint8> > font_mem_data;
   // the glyphs get destroyed in texture_font_delete()
   std::vector<font_glyph> glyph_vect;
   std::unordered_map<int, font_glyph> glyph_ht;
};


class font_db_impl
{
public:
   font_db_impl()
   {
      // text shader
      auto vsh = mws_sp<std::string>(new std::string(
         R"(
      uniform mat4 model;
      uniform mat4 view;
      uniform mat4 projection;

      attribute vec3 vertex;
      attribute vec2 tex_coord;
      attribute vec4 color;

      varying vec2 v_v2_tex_coord;
      varying vec4 v_v4_color;

      void main()
      {
	      v_v2_tex_coord = tex_coord;
	      v_v4_color = color;
            gl_Position = projection*(view*(model*vec4(vertex, 1.0)));
      }
      )"
      ));

      auto fsh = mws_sp<std::string>(new std::string(
         R"(
#ifdef GL_ES
   	precision lowp float;
#endif
      uniform sampler2D texture;
      varying vec2 v_v2_tex_coord;
      varying vec4 v_v4_color;

      void main()
      {
          float v1_a = texture2D(texture, v_v2_tex_coord).r;
          gl_FragColor = vec4(v_v4_color.rgb, v_v4_color.a * v1_a);
      }
      )"
      ));

      text_shader = gfx::i()->shader.new_program_from_src("text-shader", vsh, fsh);
      clear_db();
   }

   void clear_db()
   {
      int size = 1 << pow_of_two;

      font_size_ht.clear();

      if (tex_atlas)
      {
         texture_atlas_delete(tex_atlas);
      }

      tex_atlas = texture_atlas_new(size, size, 1);
      ext_tex_atlas_ref = nullptr;
      reload_atlas = false;
      mws_print("font texture atlas size: [%d]\n", size);
   }

   uint64 get_key(mws_sp<mws_font> ifont)
   {
      uint32* x = (uint32*)&ifont->get_file_name();
      float t = ifont->get_size();
      uint32* y = (uint32*)&t;
      uint64 x64 = *x & 0xffffffff;
      uint64 y64 = *y & 0xffffffff;
      uint64 r = (x64 << 32) | y64;

      return r;
   }

   mws_sp<std::string> get_db_font_name(const std::string& ifont_name)
   {
      mws_sp<std::string> fn = font_name_ht[ifont_name];

      if (!fn)
      {
         fn = font_name_ht[ifont_name] = mws_sp<std::string>(new std::string(ifont_name));
      }

      return fn;
   }

   mws_sp<font_cache> get_font_cache(mws_sp<mws_font> ifont)
   {
      mws_sp<font_cache> fnt_cache = ifont->fnt_cache.lock();

      if (!fnt_cache)
      {
         uint64 key = get_key(ifont);
         fnt_cache = font_size_ht[key];

         if (!fnt_cache)
         {
            const std::string& fn = ifont->get_full_path();
            float font_size = ifont->get_size();
            auto res = font_data_by_path_ht[fn].lock();

            if (!res)
            {
               res = pfm::filesystem::load_res_byte_vect(fn);
               font_data_by_path_ht[fn] = res;
            }

            if (!res)
            {
               mws_print("cannot load font [%s]", fn.c_str());
            }
            else
            {
               texture_font_t* tex_font = texture_font_new_from_memory(tex_atlas, font_size, begin_ptr(*res), res->size());
               fnt_cache = std::make_shared<font_cache>(tex_font, res);

               if (!tex_font)
               {
                  if (reload_atlas == false)
                  {
                     pow_of_two++;
                     reload_atlas = true;
                  }

                  return nullptr;
               }
            }

            font_size_ht[key] = fnt_cache;
         }

         ifont->fnt_cache = fnt_cache;
      }

      return fnt_cache;
   }

   const std::vector<font_glyph>& get_glyph_vect(mws_sp<mws_font> ifont, const std::string& itext)
   {
      mws_sp<font_cache> fnt_cache = get_font_cache(ifont);
      glyph_vect.clear();
      marked_for_loading.clear();

      if (fnt_cache)
      {
         int len = itext.length();
         texture_font_t* tex_font = fnt_cache->get_tex_font();
         glyphs_to_load.clear();

         for (int k = 0; k < len; k++)
         {
            wchar_t c = itext[k];
            font_glyph glyph = fnt_cache->get_glyph_at(c);

            if (!glyph.is_valid() && !marked_for_loading[c])
            {
               glyphs_to_load.push_back(c);
               marked_for_loading[c] = true;
            }
         }

         if (!glyphs_to_load.empty())
         {
            glyphs_to_load.push_back(0);
            int missed_chars = texture_font_load_glyphs(tex_font, begin_ptr(glyphs_to_load));

            if (missed_chars != 0)
            {
               if (reload_atlas == false)
               {
                  pow_of_two++;
                  reload_atlas = true;
               }
            }
         }

         for (int k = 0; k < len; k++)
         {
            wchar_t c = itext[k];
            font_glyph glyph = fnt_cache->get_glyph_at(c);

            if (!glyph.is_valid())
            {
               glyph = texture_font_get_glyph(tex_font, c);
               fnt_cache->set_glyph_at(glyph, c);
            }

            glyph_vect.push_back(glyph);
         }

         if (!ext_tex_atlas_ref)
         {
            ext_tex_atlas_ref = gfx::i()->tex.nwi_external("texture-atlas-" + gfx_tex::gen_id(), tex_atlas->id, "R8");
            ext_tex_atlas_ref->set_dim(tex_atlas->width, tex_atlas->height);
         }
      }

      return glyph_vect;
   }

public:
   texture_atlas_t * tex_atlas = nullptr;
   mws_sp<gfx_shader> text_shader;
   mws_sp<gfx_tex> ext_tex_atlas_ref;
   std::vector<font_glyph> glyph_vect;
   std::unordered_map<uint64, mws_sp<font_cache> > font_size_ht;
   std::unordered_map<std::string, mws_sp<std::string> > font_name_ht;
   // hold weak ref to font data so the same font with different sizes can use the same font data.
   // this way, when all sizes/instances of particular font are deleted, the common font data is also deleted.
   std::unordered_map<std::string, mws_wp<std::vector<uint8> > > font_data_by_path_ht;
   std::vector<wchar_t> glyphs_to_load;
   std::unordered_map<wchar_t, bool> marked_for_loading;
   bool reload_atlas = false;
   int pow_of_two = 9;
};


void font_db::clear_db()
{
   p->reload_atlas = true;
}

void font_db::resize_db(int ipow_of_two)
{
   if (ipow_of_two < 6 || ipow_of_two > 13)
   {
      mws_throw mws_exception("6 <= ipow_of_two <= 13");
   }

   p->pow_of_two = ipow_of_two;
   p->reload_atlas = true;
}

mws_sp<std::string> font_db::get_db_font_name(const std::string& ifont_name)
{
   return p->get_db_font_name(ifont_name);
}

const std::vector<font_glyph>& font_db::get_glyph_vect(mws_sp<mws_font> ifont, const std::string& i_text)
{
   return p->get_glyph_vect(ifont, i_text);
}

mws_sp<gfx_tex> font_db::get_texture_atlas()
{
   return p->ext_tex_atlas_ref;
}

float font_db::get_ascender(mws_sp<mws_font> ifont)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(ifont);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->ascender;
   }

   return 0;
}

float font_db::get_descender(mws_sp<mws_font> ifont)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(ifont);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->descender;
   }

   return 0;
}

float font_db::get_height(mws_sp<mws_font> ifont)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(ifont);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->height;
   }

   return 0;
}

glm::vec2 font_db::get_text_dim(mws_sp<mws_font> ifont, const std::string& itext)
{
   auto glyphs = p->get_glyph_vect(ifont, itext);
   int len = glm::min(itext.length(), glyphs.size());
   glm::vec2 pen(0, get_height(ifont));
   float kerning = 0.0f;

   for (int i = 0; i < len; ++i)
   {
      font_glyph glyph = glyphs[i];

      char ch = itext[i];
      // ignore carriage returns
      if (ch < ' ')
      {
         if (ch == '\n')
         {
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
         if (glyph.is_valid())
         {
            if (i > 0)
            {
               kerning = glyph.get_kerning(itext[i - 1]);
            }
            else
            {
               kerning = 0;
            }

            pen.x += kerning;
            //int x0 = (int)(pen->x + glyph->offset_x);
            //int y0 = (int)(pen->y + glyph->offset_y);
            //int x1 = (int)(x0 + glyph->width);
            //int y1 = (int)(y0 - glyph->height);
            pen.x += glyph.get_advance_x();
         }
      }
   }

   return pen;
}

font_db::font_db()
{
   p = mws_sp<font_db_impl>(new font_db_impl());
}

void font_db::on_frame_start()
{
   if (p->reload_atlas)
   {
      p->clear_db();
   }
}

#elif defined MOD_BITMAP_FONTS

font_db::font_db()
{
}

void font_db::on_frame_start()
{
}

mws_sp<std::string> font_db::get_db_font_name(const std::string& ifont_name)
{
   std::mws_sp<std::string> fn(new std::string("n/a"));

   return fn;
}

float font_db::get_ascender(mws_sp<mws_font> ifont)
{
   return 0;
}

float font_db::get_descender(mws_sp<mws_font> ifont)
{
   return 0;
}

float font_db::get_height(mws_sp<mws_font> ifont)
{
   return 20;
}

glm::vec2 font_db::get_text_dim(mws_sp<mws_font> ifont, const std::string& itext)
{
   return glm::vec2(itext.length() * get_height(ifont) / 2, get_height(ifont));
}

#endif

#endif
