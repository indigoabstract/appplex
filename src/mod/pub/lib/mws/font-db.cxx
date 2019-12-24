#include "stdafx.hxx"

#include "appplex-conf.hxx"

#if defined MOD_FONTS

#include "font-db.hxx"
#include "mws.hxx"
#include "mws-font.hxx"
#include "min.hxx"
#include <glm/inc.hpp>


#if defined MOD_VECTOR_FONTS

#include "gfx.hxx"
#include "gfx-tex.hxx"
#include <freetype-gl/freetype-gl.h>
// default font binary data
#include "font-def.inl"


font_glyph::font_glyph()
{
   glyph = nullptr;
}

bool font_glyph::is_valid() const
{
   return glyph != nullptr;
}

uint32_t font_glyph::get_charcode()const
{
   return glyph->codepoint;
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

float font_glyph::get_kerning(char i_charcode)const
{
   char c[2] = { i_charcode, 0 };
   return texture_glyph_get_kerning(glyph, c);
}

mws_font_rendermode font_glyph::get_rendermode()const
{
   return static_cast<mws_font_rendermode>(glyph->rendermode);
}

float font_glyph::get_outline_thickness()const
{
   return glyph->outline_thickness;
}

font_glyph::font_glyph(texture_glyph_t* i_glyph)
{
   glyph = i_glyph;
}


class font_cache
{
public:
   const int inf_lim = 32;
   const int sup_lim = 126;
   std::string font_file_name;
   float font_size = 0.f;

   font_cache(std::string i_font_file_name, float i_font_size, texture_font_t* itex_font, mws_sp<std::vector<uint8> > i_font_mem_data)
   {
      mws_assert(!i_font_file_name.empty());
      font_file_name = i_font_file_name;
      font_size = i_font_size;
      tex_font = itex_font;
      font_mem_data = i_font_mem_data;
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

   mws_sp<std::vector<uint8>> get_font_data()
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
   texture_font_t* tex_font = nullptr;
   mws_sp<std::vector<uint8> > font_mem_data;
   // the glyphs get destroyed in texture_font_delete()
   std::vector<font_glyph> glyph_vect;
   std::unordered_map<int, font_glyph> glyph_ht;
};


class font_db_impl
{
public:
   font_db_impl(uint32 i_pow_of_two) : pow_of_two(i_pow_of_two)
   {
      // text shader
      const std::string text_shader_name = "text-shader";
      text_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(text_shader_name, true);

      if (!text_shader)
      {
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

         text_shader = gfx::i()->shader.new_program_from_src(text_shader_name, vsh, fsh);
      }

      clear_db();
   }

   void clear_db()
   {
      int size = 1 << pow_of_two;

      font_size_ht.clear();
      reload_atlas = false;

      if (tex_atlas)
      {
         texture_atlas_delete(tex_atlas);
      }

      {
         gfx_tex_params prm;

         prm.set_format_id("R8");
         prm.set_rt_params();
         prm.mag_filter = prm.e_tf_nearest;
         prm.min_filter = prm.e_tf_nearest;
         tex_atlas = texture_atlas_new(size, size, 1);
         glyph_atlas = gfx::i()->tex.nwi("texture-atlas-" + gfx_tex::gen_id(), size, size, &prm);
         mws_print("font texture atlas size: [%d]\n", size);
      }
   }

   std::string get_key(const std::string& i_font_file_name, float i_size)
   {
      return i_font_file_name + mws_to_str_fmt("%.2f", i_size);
   }

   mws_sp<mws_font> get_global_font() const
   {
      return global_font;
   }

   void set_global_font(mws_sp<mws_font> i_font)
   {
      if (i_font->get_full_path().empty())
      {
         global_font = mws_font::nwi(i_font->get_size(), global_font->get_full_path());
      }
      else
      {
         global_font = i_font;
      }
   }

   mws_sp<std::string> get_db_font_name(const std::string& i_font_name)
   {
      mws_sp<std::string> fn;
      auto it = font_name_ht.find(i_font_name);

      if (it != font_name_ht.end())
      {
         fn = it->second.font_name;
      }
      else
      {
         font_info fi;
         fn = fi.font_name = std::make_shared<std::string>(i_font_name);
         font_name_ht[i_font_name] = fi;
      }

      return fn;
   }

   mws_sp<font_cache> get_font_cache(mws_sp<mws_font> i_font)
   {
      mws_sp<font_cache> fnt_cache = i_font->fnt_cache.lock();
      const std::string& font_file_name_0 = i_font->get_file_name();

      // fonts having empty font_file_name are global fonts. if the global font has changed, then invalidate the font cache
      if (font_file_name_0.empty() && fnt_cache)
      {
         if (fnt_cache->font_file_name != global_font->get_file_name())
         {
            fnt_cache = nullptr;
         }
      }

      if (!fnt_cache)
      {
         const std::string& font_file_name = (font_file_name_0.empty()) ? global_font->get_file_name() : font_file_name_0;
         float font_size = i_font->get_size();
         bool has_markup = i_font->has_markup();
         std::string key = (has_markup) ? mws_to_str_fmt("markup-font-%d", markup_idx++) : get_key(font_file_name, font_size);
         fnt_cache = font_size_ht[key];

         if (!fnt_cache)
         {
            auto res = font_data_by_path_ht[font_file_name].lock();

            if (!res)
            {
               if (font_file_name == font_db::default_font_name)
               {
                  res = std::make_shared<std::vector<uint8>>(mws_def_font_data, mws_def_font_data + mws_def_font_data_size);
               }
               else
               {
                  res = pfm::filesystem::load_res_byte_vect(font_file_name);
               }

               if (!res)
               {
                  mws_println("ERROR[ cannot load font [%s] ]", font_file_name.c_str());
                  return nullptr;
               }

               font_data_by_path_ht[font_file_name] = res;
            }

            texture_font_t* tex_font = texture_font_new_from_memory(tex_atlas, font_size, begin_ptr(*res), res->size());
            fnt_cache = std::make_shared<font_cache>(font_file_name, font_size, tex_font, res);

            if (!tex_font)
            {
               if (reload_atlas == false)
               {
                  pow_of_two++;
                  reload_atlas = true;
               }

               return nullptr;
            }
            else
            {
               tex_font->padding = 0;
            }

            font_size_ht[key] = fnt_cache;

            if (has_markup)
            {
               const mws_font_markup& mk = i_font->get_markup();
               tex_font->rendermode = static_cast<ftgl::rendermode_t>(mk.rendermode);
               tex_font->outline_thickness = mk.outline_thickness;
            }
         }

         i_font->fnt_cache = fnt_cache;
      }

      return fnt_cache;
   }

   const std::vector<font_glyph>& get_glyph_vect(mws_sp<mws_font> i_font, const std::string& i_text)
   {
      mws_sp<font_cache> fnt_cache = get_font_cache(i_font);
      glyph_vect.clear();
      marked_for_loading.clear();

      if (fnt_cache)
      {
         int len = i_text.length();
         texture_font_t* tex_font = fnt_cache->get_tex_font();
         glyphs_to_load.clear();

         for (int k = 0; k < len; k++)
         {
            char c = i_text[k];
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
            int missed_chars = texture_font_load_glyphs(tex_font, glyphs_to_load.data());

            if (missed_chars != 0)
            {
               if (reload_atlas == false)
               {
                  pow_of_two++;
                  reload_atlas = true;
                  mws_println("WARNING: font tex too small. reloading atlas with new pot size [ %d ]", pow_of_two);
               }
            }

            glyph_atlas->update(0, (const char*)tex_font->atlas->data);
         }

         if (!reload_atlas)
         {
            for (int k = 0; k < len; k++)
            {
               char c[2] = { i_text[k], 0 };
               font_glyph glyph = fnt_cache->get_glyph_at(c[0]);

               if (!glyph.is_valid())
               {
                  glyph = texture_font_get_glyph(tex_font, c);
                  fnt_cache->set_glyph_at(glyph, c[0]);
               }

               glyph_vect.push_back(glyph);
            }
         }
      }

      return glyph_vect;
   }

public:
   struct font_info
   {
      mws_sp<std::string> font_name;
      struct mix_f { float operator()(const float& i_c0, const float& i_c1, float i_f) { return (1.f - i_f) * i_c0 + i_f * i_c1; } };
      // height is in pixels(px), size is in points(pt)
      // this maps max font height in pixels(which is the difference from bottom pixel to the top pixel) to font size in points
      using px_to_pt_mixer = mws_val_mixer<float, mix_f>;
      px_to_pt_mixer px_to_pt_mix;
      mws_pt min_height_pt;
      mws_px min_height_px;
      mws_pt max_height_pt;
      mws_px max_height_px;
   };

   texture_atlas_t* tex_atlas = nullptr;
   mws_sp<gfx_shader> text_shader;
   mws_sp<gfx_tex> glyph_atlas;
   std::vector<font_glyph> glyph_vect;
   std::unordered_map<std::string, mws_sp<font_cache>> font_size_ht;
   std::unordered_map<std::string, font_info> font_name_ht;
   // hold weak ref to font data so the same font with different sizes can use the same font data.
   // this way, when all sizes/instances of particular font are deleted, the common font data is also deleted.
   std::unordered_map<std::string, mws_wp<std::vector<uint8>>> font_data_by_path_ht;
   std::vector<char> glyphs_to_load;
   std::unordered_map<uint32_t, bool> marked_for_loading;
   mws_sp<mws_font> global_font;
   bool reload_atlas = false;
   uint32 pow_of_two = 0;
   static inline uint32 markup_idx = 0;
};


mws_sp<font_db> font_db::nwi_inex(uint32 i_pow_of_two_db_size)
{
   mws_assert(!instance);
   instance = mws_sp<font_db>(new font_db(i_pow_of_two_db_size));
   instance->p->global_font = mws_font::nwi(font_db::default_font_name, mws_cm(0.4f));
   return instance;
}

mws_sp<font_db> font_db::inst()
{
   return instance;
}

void font_db::clear_db()
{
   p->reload_atlas = true;
}

void font_db::resize_db(uint32 i_pow_of_two)
{
   if (i_pow_of_two < 6 || i_pow_of_two > 13)
   {
      mws_throw mws_exception("6 <= i_pow_of_two <= 13");
   }

   p->pow_of_two = i_pow_of_two;
   p->reload_atlas = true;
}

mws_sp<mws_font> font_db::get_global_font() const
{
   return p->get_global_font();
}

void font_db::set_global_font(const std::string& i_font_name, float i_size)
{
   float size = (i_size > 0.f) ? i_size : 40.f;
   auto font = mws_font::nwi(size, i_font_name);
   set_global_font(font);
}

void font_db::set_global_font(mws_sp<mws_font> i_font)
{
   p->set_global_font(i_font);
}

mws_sp<std::string> font_db::get_db_font_name(const std::string& i_font_name)
{
   return p->get_db_font_name(i_font_name);
}

const std::vector<font_glyph>& font_db::get_glyph_vect(mws_sp<mws_font> i_font, const std::string& i_text)
{
   return p->get_glyph_vect(i_font, i_text);
}

mws_sp<gfx_tex> font_db::get_texture_atlas()
{
   return p->glyph_atlas;
}

float font_db::get_ascender(mws_sp<mws_font> i_font)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(i_font);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->ascender;
   }

   return 0;
}

float font_db::get_descender(mws_sp<mws_font> i_font)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(i_font);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->descender;
   }

   return 0;
}

float font_db::get_height(mws_sp<mws_font> i_font)
{
   mws_sp<font_cache> fnt_cache = p->get_font_cache(i_font);

   if (fnt_cache)
   {
      return fnt_cache->get_tex_font()->height;
   }

   return 0;
}

glm::vec2 font_db::get_text_dim(mws_sp<mws_font> i_font, const std::string& i_text)
{
   auto glyphs = p->get_glyph_vect(i_font, i_text);
   int len = glm::min(i_text.length(), glyphs.size());
   glm::vec2 pen(0, get_height(i_font));
   float kerning = 0.0f;

   for (int i = 0; i < len; ++i)
   {
      font_glyph glyph = glyphs[i];

      char ch = i_text[i];
      // ignore carriage returns
      if (ch < ' ')
      {
         if (ch == '\n')
         {
            pen.y += i_font->get_height();
         }
         else if (ch == '\t')
         {
            pen.x += 2 * i_font->get_height();
         }
      }
      // normal character
      else
      {
         if (glyph.is_valid())
         {
            if (i > 0)
            {
               kerning = glyph.get_kerning(i_text[i - 1]);
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

void font_db::store_font_metrix(const std::string& i_font_path, const mws_pt& i_min_height_pt, const mws_px& i_min_height_px,
   const mws_pt& i_max_height_pt, const mws_px& i_max_height_px, const std::pair<float, float>* i_pixels_to_points_data, uint32 i_data_elem_count)
{
   std::string font_name = mws_util::path::get_filename_from_path(i_font_path);
   auto& ht = p->font_name_ht;
   auto it = ht.find(font_name);
   font_db_impl::font_info tmp_fi;
   font_db_impl::font_info& fi = (it != ht.end()) ? it->second : tmp_fi;
   std::vector<font_db_impl::font_info::px_to_pt_mixer::pos_val> val_vect(i_data_elem_count);
   std::pair<float, float>* dest = (std::pair<float, float>*)val_vect.data();

   fi.min_height_pt = i_min_height_pt;
   fi.min_height_px = i_min_height_px;
   fi.max_height_pt = i_max_height_pt;
   fi.max_height_px = i_max_height_px;
   std::copy(i_pixels_to_points_data, i_pixels_to_points_data + i_data_elem_count, dest);
   fi.px_to_pt_mix.set_values(val_vect);

   if (it == ht.end())
   {
      fi.font_name = std::make_shared<std::string>(font_name);
      ht[font_name] = fi;
   }
}

mws_sp<mws_font> font_db::load_font_by_metrix(const std::string& i_font_path, const mws_dim& i_height, const mws_font_markup* i_markup)
{
   std::string font_name = mws_util::path::get_filename_from_path(i_font_path);
   auto& ht = p->font_name_ht;
   auto it = ht.find(font_name);

   if (it != ht.end())
   {
      font_db_impl::font_info& fi = it->second;

      if (fi.min_height_pt.val() > 0.f && fi.min_height_px.int_val() > 0 && fi.max_height_pt.val() > 0.f && fi.max_height_px.int_val() > 0)
      {
         float px_count = i_height.to_px().val();
         mws_assert(px_count > 0);
         float px_idx = float(px_count - fi.min_height_px.val()) / (fi.max_height_px.val() - fi.min_height_px.val());
         float font_size_pt = 0.f;

         if (px_count < fi.min_height_px.val() || px_count > fi.max_height_px.val())
         {
            float pos = (px_count - fi.min_height_px.val()) / (fi.max_height_px.val() - fi.min_height_px.val());
            font_size_pt = glm::mix(fi.min_height_pt.val(), fi.max_height_pt.val(), pos);
         }
         else
         {
            font_size_pt = fi.px_to_pt_mix.get_val_at(px_idx);
         }

         return mws_font::nwi(font_size_pt, i_font_path, i_markup);
      }
   }

   return mws_font::nwi(i_height.to_pt().val(), i_font_path, i_markup);
}

font_db::font_db(uint32 i_pow_of_two)
{
   p = mws_sp<font_db_impl>(new font_db_impl(i_pow_of_two));
   store_font_metrix(font_db::default_font_name, mws_pt(min_height_pt), mws_px(min_height_px),
      mws_pt(max_height_pt), mws_px(max_height_px), mws_def_font_data_metrix, mws_def_font_data_metrix_size);
}

void font_db::on_frame_start()
{
   if (p->reload_atlas)
   {
      p->clear_db();
   }
}

#elif defined MOD_BITMAP_FONTS

font_db::font_db(uint32 i_pow_of_two)
{
}

mws_sp<font_db> font_db::inst()
{
   if (!instance)
   {
      instance = mws_sp<font_db>(new font_db());
   }

   return instance;
}

mws_sp<mws_font> font_db::get_global_font() const
{
   return nullptr;
}

void font_db::on_frame_start()
{
}

mws_sp<std::string> font_db::get_db_font_name(const std::string& i_font_name)
{
   mws_sp<std::string> font_path(new std::string("n/a"));

   return font_path;
}

float font_db::get_ascender(mws_sp<mws_font> i_font)
{
   return 0;
}

float font_db::get_descender(mws_sp<mws_font> i_font)
{
   return 0;
}

float font_db::get_height(mws_sp<mws_font> i_font)
{
   return 20;
}

glm::vec2 font_db::get_text_dim(mws_sp<mws_font> i_font, const std::string& i_text)
{
   return glm::vec2(i_text.length() * get_height(i_font) / 2, get_height(i_font));
}

void font_db::store_font_metrix(const std::string& i_font_path, const mws_pt& i_min_height_pt, const mws_px& i_min_height_px,
   const mws_pt& i_max_height_pt, const mws_px& i_max_height_px, const std::pair<float, float>* i_pixels_to_points_data, uint32 i_data_elem_count) {}
mws_sp<mws_font> font_db::load_font_by_metrix(const std::string& i_font_path, const mws_dim& i_height) { return nullptr; }

#endif


const std::string font_db::default_font_name = "mws-def-font";
mws_sp<font_db> font_db::instance;

#endif
