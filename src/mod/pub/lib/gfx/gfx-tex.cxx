#include "stdafx.hxx"

#include "gfx-tex.hxx"
#include "gfx.hxx"
#include "krn.hxx"
#include "gfx-util.hxx"
#include "gfx-shader.hxx"
#include "pfm-gl.h"
#include "res-ld/res-ld.hxx"


gfx_tex_params::gfx_tex_params()
{
   set_format_id("RGBA8");
   min_filter = e_tf_linear_mipmap_linear;
   mag_filter = e_tf_linear;
   wrap_r = e_twm_clamp_to_edge;
   wrap_s = e_twm_clamp_to_edge;
   wrap_t = e_twm_clamp_to_edge;
   max_anisotropy = 16.f;
   gen_mipmaps = true;
   regen_mipmaps = false;
}

gfx_int gfx_tex_params::gl_mag_filter()
{
   switch (mag_filter)
   {
   case e_tf_nearest:
      return GL_NEAREST;

   case e_tf_linear:
      return GL_LINEAR;

   default:
      return GL_NEAREST;
   }
}

gfx_int gfx_tex_params::gl_min_filter()
{
   switch (min_filter)
   {
   case e_tf_nearest:
      return GL_NEAREST;

   case e_tf_linear:
      return GL_LINEAR;

   case e_tf_nearest_mipmap_nearest:
      return GL_NEAREST_MIPMAP_NEAREST;

   case e_tf_linear_mipmap_nearest:
      return GL_LINEAR_MIPMAP_NEAREST;

   case e_tf_nearest_mipmap_linear:
      return GL_NEAREST_MIPMAP_LINEAR;

   case e_tf_linear_mipmap_linear:
      return GL_LINEAR_MIPMAP_LINEAR;

   default:
      return GL_NEAREST;
   }
}

gfx_int gfx_tex_params::gl_wrap_r()
{
   switch (wrap_r)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirrored_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

gfx_int gfx_tex_params::gl_wrap_s()
{
   switch (wrap_s)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirrored_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

gfx_int gfx_tex_params::gl_wrap_t()
{
   switch (wrap_s)
   {
   case e_twm_repeat:
      return GL_REPEAT;

   case e_twm_clamp_to_edge:
      return GL_CLAMP_TO_EDGE;

   case e_twm_mirrored_repeat:
      return GL_MIRRORED_REPEAT;

   default:
      return GL_CLAMP_TO_EDGE;
   }
}

bool gfx_tex_params::anisotropy_enabled()
{
   return max_anisotropy > 0.f;
}

gfx_enum gfx_tex_params::get_bpp() const
{
   return ti->get_bpp();
}

gfx_enum gfx_tex_params::get_internal_format() const
{
   return ti->get_internal_format();
}

gfx_enum gfx_tex_params::get_format() const
{
   return ti->get_format();
}

gfx_enum gfx_tex_params::get_type() const
{
   return ti->get_type();
}

const std::string& gfx_tex_params::get_format_id() const
{
   return ti->get_id();;
}

void gfx_tex_params::set_format_id(std::string i_format_id)
{
   ti = gfx_util::get_tex_info(i_format_id);
   mws_assert(ti != nullptr);
}

void gfx_tex_params::set_rt_params()
{
   mag_filter = e_tf_linear;
   min_filter = e_tf_linear;
   wrap_s = e_twm_clamp_to_edge;
   wrap_t = e_twm_clamp_to_edge;
   wrap_r = e_twm_clamp_to_edge;
   max_anisotropy = 0.f;
   gen_mipmaps = false;
   regen_mipmaps = false;
}


static uint32_t texture_name_idx = 0;

gfx_tex::~gfx_tex()
{
   release();
}

bool gfx_tex::mipmaps_supported(gfx_enum i_internal_format)
{
   switch (i_internal_format)
   {
   case GL_R32UI:
      return false;
   }

   return true;
}

std::string gfx_tex::gen_id()
{
   uint32_t time = mws::time::get_time_millis();
   std::string name = mws_to_str_fmt("tex-%d-%d", texture_name_idx, time);
   texture_name_idx++;

   return name;
}

gfx_obj::e_gfx_obj_type gfx_tex::get_type()const
{
   return e_tex;
}

const gfx_tex_params& gfx_tex::get_params() const
{
   return prm;
}

bool gfx_tex::is_valid()const
{
   return is_valid_state;
}

bool gfx_tex::is_external_texture()
{
   check_valid_state();
   return is_external;
}

std::string gfx_tex::get_name()
{
   check_valid_state();
   return tex_name;
}

gfx_tex::gfx_tex_types gfx_tex::get_tex_type()
{
   check_valid_state();
   return uni_tex_type;
}

gfx_uint gfx_tex::get_texture_gl_id()
{
   check_valid_state();
   return texture_gl_id;
}

void gfx_tex::set_texture_gl_id(gfx_uint i_texture_id)
{
   check_valid_state();
   // only available for external textures
   assert(is_external);
   texture_gl_id = i_texture_id;
}

uint32_t gfx_tex::get_width()
{
   check_valid_state();
   return width;
}

uint32_t gfx_tex::get_height()
{
   check_valid_state();
   return height;
}

void gfx_tex::set_dim(uint32_t i_width, uint32_t i_height)
{
    check_valid_state();
    // only available for external textures
    assert(is_external);
    init_dimensions(i_width, i_height);
}

void gfx_tex::send_uniform(const std::string iuniform_name, uint32_t i_active_tex_index)
{
   check_valid_state();

   mws_sp<gfx_shader> glp = gi()->shader.get_current_program();
   gfx_int param_location = glp->get_param_location(iuniform_name);

   if (param_location != -1)
   {
      mws_report_gfx_errs();
      glUniform1i(param_location, i_active_tex_index);
      mws_report_gfx_errs();
      set_active(i_active_tex_index);
      mws_report_gfx_errs();
   }
}

void gfx_tex::set_active(uint32_t i_tex_unit_index)
{
   check_valid_state();
   mws_report_gfx_errs();
   glActiveTexture(GL_TEXTURE0 + i_tex_unit_index);
   mws_report_gfx_errs();
   glBindTexture(gl_tex_target, texture_gl_id);

   if (texture_updated)
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

      if (prm.anisotropy_enabled())
      {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
      }

      if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))
      {
         glGenerateMipmap(gl_tex_target);
      }

      texture_updated = false;
   }

   if (prm.regen_mipmaps && (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format())))
   {
      glGenerateMipmap(gl_tex_target);
   }

   mws_report_gfx_errs();
}

void gfx_tex::update(uint32_t i_active_tex_index, const char* i_bb)
{
   check_valid_state();
   set_active(i_active_tex_index);

   if (!is_external && uni_tex_type == TEX_2D)
   {
      glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, prm.get_format(), prm.get_type(), i_bb);

      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
      glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

      if (prm.anisotropy_enabled())
      {
         glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
      }

      if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))
      {
         glGenerateMipmap(gl_tex_target);
      }
   }

   mws_report_gfx_errs();
}

void gfx_tex::reload()
{
   if (is_external)
   {
      set_texture_gl_id(0);
   }
   else
   {
      if (is_valid())
      {
         texture_gl_id = gen_texture_gl_id();
         // setActive(0);
         glBindTexture(gl_tex_target, texture_gl_id);

         if (uni_tex_type == TEX_2D)
         {
            uint32_t mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format())) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

            mws_tex_img_2d(gl_tex_target, mipmap_count, prm.get_internal_format(), width, height, 0, prm.get_format(), prm.get_type(), nullptr);
         }

         mws_report_gfx_errs();

         glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
         glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
         glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
         glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

         if (prm.anisotropy_enabled())
         {
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
         }

         if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))
         {
            glGenerateMipmap(gl_tex_target);
         }

         mws_report_gfx_errs();
      }
   }
}

gfx_tex::gfx_tex(const gfx_tex_params* i_prm, mws_sp<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
}

gfx_tex::gfx_tex(std::string i_tex_name, const gfx_tex_params* i_prm, mws_sp<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   uni_tex_type = TEX_2D;
   set_texture_name(i_tex_name);

   mws_sp<raw_img_data> rid = res_ld::inst()->load_image(tex_name);
   init_dimensions(rid->width, rid->height);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = gen_texture_gl_id();
   glBindTexture(gl_tex_target, texture_gl_id);

   if (uni_tex_type == TEX_2D)
   {
      uint32_t mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format())) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

      mws_tex_img_2d(gl_tex_target, mipmap_count, prm.get_internal_format(), width, height, 0, prm.get_format(), prm.get_type(), nullptr);
      glTexSubImage2D(gl_tex_target, 0, 0, 0, width, height, prm.get_format(), prm.get_type(), rid->data);
   }

   mws_report_gfx_errs();

   glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

   if (prm.anisotropy_enabled())
   {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   }

   if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))
   {
      glGenerateMipmap(gl_tex_target);
   }

   mws_report_gfx_errs();
   is_valid_state = true;
}

gfx_tex::gfx_tex(std::string i_tex_name, uint32_t i_texture_id, uint32_t i_width, uint32_t i_height,
   gfx_tex_types iuni_tex_type, const gfx_tex_params* i_prm, mws_sp<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   is_external = true;
   uni_tex_type = iuni_tex_type;
   set_texture_name(i_tex_name);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = i_texture_id;
   init_dimensions(i_width, i_height);

   mws_report_gfx_errs();
   is_valid_state = true;
}

gfx_tex::gfx_tex(std::string i_tex_name, uint32_t i_width, uint32_t i_height,
   gfx_tex_types iuni_tex_type, const gfx_tex_params* i_prm, mws_sp<gfx> i_gi) : gfx_obj(i_gi)
{
   set_params(i_prm);
   uni_tex_type = iuni_tex_type;
   set_texture_name(i_tex_name);
   init_dimensions(i_width, i_height);

   switch (uni_tex_type)
   {
   case TEX_2D:
      gl_tex_target = GL_TEXTURE_2D;
      break;
   }

   texture_gl_id = gen_texture_gl_id();
   // setActive(0);
   glBindTexture(gl_tex_target, texture_gl_id);

   if (uni_tex_type == TEX_2D)
   {
      uint32_t mipmap_count = (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format())) ? gfx_util::get_tex_2d_mipmap_count(width, height) : 1;

      mws_tex_img_2d(gl_tex_target, mipmap_count, prm.get_internal_format(), width, height, 0, prm.get_format(), prm.get_type(), nullptr);
   }

   mws_report_gfx_errs();

   glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
   glTexParameteri(gl_tex_target, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());

   if (prm.anisotropy_enabled())
   {
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   }

   if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))
   {
      glGenerateMipmap(gl_tex_target);
   }

   mws_report_gfx_errs();
   is_valid_state = true;
}

void gfx_tex::set_texture_name(std::string i_tex_name)
{
   tex_name = i_tex_name;
}

void gfx_tex::init_dimensions(uint32_t i_width, uint32_t i_height)
{
   mws_assert(i_width > 0 && i_height > 0);
   width = i_width;
   height = i_height;
}

void gfx_tex::set_params(const gfx_tex_params* i_prm)
{
   if (i_prm)
   {
      prm = *i_prm;
   }
}

gfx_uint gfx_tex::gen_texture_gl_id()
{
   gfx_uint tex_id;

   glGenTextures(1, &tex_id);

   mws_print("gfx-info: gen tex id [ %d ] name [ %s ]\n", tex_id, tex_name.c_str());
   mws_report_gfx_errs();

   return tex_id;
}

void gfx_tex::check_valid_state()
{
   assert(is_valid_state);
}

void gfx_tex::release()
{
   if (is_valid_state)
   {
      is_valid_state = false;

      if (!is_external)
      {
         glDeleteTextures(1, &texture_gl_id);

         mws_print("gfx-info: released tex name[ %s ] id[ %d ]\n", tex_name.c_str(), texture_gl_id);
      }

      gi()->remove_gfx_obj(this);
   }
}


gfx_tex_2d::~gfx_tex_2d()
{
   release();
}

gfx_tex_2d::gfx_tex_2d(std::string i_tex_name, const gfx_tex_params* i_prm, mws_sp<gfx> i_gi) : gfx_tex(i_tex_name, i_prm, i_gi)
{
}


gfx_tex_3d::~gfx_tex_3d()
{
   release();
}

gfx_tex_3d::gfx_tex_3d(std::string i_tex_name, mws_sp<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
}


gfx_tex_cube_map::~gfx_tex_cube_map()
{
   release();
}

gfx_tex_cube_map::gfx_tex_cube_map(std::string i_tex_name, mws_sp<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
   {
      gfx_tex_params t_prm;

      t_prm.wrap_r = gfx_tex_params::e_twm_clamp_to_edge;
      t_prm.wrap_s = gfx_tex_params::e_twm_clamp_to_edge;
      t_prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
      set_params(&t_prm);
   }

   uni_tex_type = TEX_CUBE_MAP;
   set_texture_name(i_tex_name);
   gl_tex_target = GL_TEXTURE_CUBE_MAP;
   texture_gl_id = gen_texture_gl_id();
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(gl_tex_target, texture_gl_id);
    mws_report_gfx_errs();

   std::string ends[] = { "posx", "negx", "posy", "negy", "posz", "negz" };
   bool is_init = false;

   for (uint32_t k = 0; k < 6; k++)
   {
      std::string img_name = i_tex_name + "-" + ends[k] + ".png";
      mws_sp<raw_img_data> rid = res_ld::inst()->load_image(img_name);

      if (!is_init)
      {
         uint32_t mipmap_count = mipmaps_supported(prm.get_internal_format()) ? gfx_util::get_tex_2d_mipmap_count(rid->width, rid->height) : 1;

         is_init = true;
         init_dimensions(rid->width, rid->height);
         mws_tex_img_2d(GL_TEXTURE_CUBE_MAP, mipmap_count, prm.get_internal_format(), width, height, 0, prm.get_format(), prm.get_type(), nullptr);
          mws_report_gfx_errs();
      }

      glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + k, 0, 0, 0, width, height, prm.get_format(), prm.get_type(), rid->data);
       mws_report_gfx_errs();
   }

   mws_report_gfx_errs();

   if (mipmaps_supported(prm.get_internal_format()))
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, prm.gl_min_filter());
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, prm.gl_mag_filter());
   }
   else
   {
      glTexParameteri(gl_tex_target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(gl_tex_target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   }

   //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   //glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, prm.gl_wrap_s());
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, prm.gl_wrap_t());
   glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, prm.gl_wrap_r());
   if (prm.anisotropy_enabled())glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, prm.max_anisotropy);
   if (prm.gen_mipmaps && mipmaps_supported(prm.get_internal_format()))glGenerateMipmap(gl_tex_target);

   mws_report_gfx_errs();
   is_valid_state = true;
}

gfx_tex_cube_map::gfx_tex_cube_map(uint32_t i_size, mws_sp<gfx> i_gi) : gfx_tex(nullptr, i_gi)
{
}
