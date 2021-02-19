#pragma once

#include "pfm-def.h"
#include "gfx-obj.hxx"
#include <string>


class gfx;
class gfx_tex_info;


class gfx_tex_params
{
public:
   enum e_tex_filters
   {
      // texture mag filter
      e_tf_nearest,
      e_tf_linear,
      // texture min filter
      // e_tf_nearest,
      // e_tf_linear,
      e_tf_nearest_mipmap_nearest,
      e_tf_linear_mipmap_nearest,
      e_tf_nearest_mipmap_linear,
      e_tf_linear_mipmap_linear
   };

   enum e_tex_wrap_modes
   {
      e_twm_repeat,
      e_twm_clamp_to_edge,
      e_twm_mirrored_repeat
   };

   gfx_tex_params();
   gfx_int gl_mag_filter();
   gfx_int gl_min_filter();
   gfx_int gl_wrap_r();
   gfx_int gl_wrap_s();
   gfx_int gl_wrap_t();
   bool anisotropy_enabled();
   gfx_enum get_bpp() const;
   gfx_enum get_internal_format() const;
   gfx_enum get_format() const;
   gfx_enum get_type() const;
   const std::string& get_format_id() const;
   void set_format_id(std::string i_format_id);
   // set params to enable this tex to be used as an rt (filtering/wrap/gen mipmaps, etc). use this before creating the texture.
   void set_rt_params();

   e_tex_filters mag_filter;
   e_tex_filters min_filter;
   e_tex_wrap_modes wrap_r;
   e_tex_wrap_modes wrap_s;
   e_tex_wrap_modes wrap_t;
   float max_anisotropy;
   bool gen_mipmaps;
   bool regen_mipmaps;

private:
   mws_sp<gfx_tex_info> ti;
};


class gfx_tex : public gfx_obj
{
public:
   enum gfx_tex_types
   {
      TEX_2D,
      TEX_ARRAY_2D,
      TEX_3D,
      TEX_CUBE_MAP,
   };

   virtual ~gfx_tex();
   static bool mipmaps_supported(gfx_enum i_internal_format);
   static std::string gen_id();
   virtual gfx_obj::e_gfx_obj_type get_type()const;
   const gfx_tex_params& get_params() const;
   virtual bool is_valid()const;
   bool is_external_texture();
   std::string get_name();
   gfx_tex_types get_tex_type();
   gfx_uint get_texture_gl_id();
   void set_texture_gl_id(gfx_uint i_texture_id);
   uint32_t get_width();
   uint32_t get_height();
   void set_dim(uint32_t i_width, uint32_t i_height);
   void send_uniform(const std::string iuniform_name, uint32_t iactive_tex_index);
   void set_active(uint32_t itex_unit_index);
   void update(uint32_t iactive_tex_index, const char* ibb);
   void reload();

protected:
   friend class gfx;

   gfx_tex(const gfx_tex_params* i_prm = nullptr, mws_sp<gfx> i_gi = nullptr);
   gfx_tex(std::string i_uni_tex_name, const gfx_tex_params* i_prm = nullptr, mws_sp<gfx> i_gi = nullptr);
   gfx_tex(std::string i_uni_tex_name, uint32_t i_texture_id, uint32_t i_width, uint32_t i_height,
      gfx_tex_types iuni_tex_type, const gfx_tex_params* i_prm = nullptr, mws_sp<gfx> i_gi = nullptr);
   gfx_tex(std::string i_uni_tex_name, uint32_t i_width, uint32_t i_height, gfx_tex_types iuni_tex_type,
      const gfx_tex_params* i_prm = nullptr, mws_sp<gfx> i_gi = nullptr);

   void set_texture_name(std::string i_uni_tex_name);
   void init_dimensions(uint32_t i_width, uint32_t i_height);
   void set_params(const gfx_tex_params* i_prm);
   gfx_uint gen_texture_gl_id();
   void check_valid_state();
   void release();

   bool is_valid_state = false;
   bool is_external = false;
   gfx_enum gl_tex_target;
   std::string	tex_name;
   gfx_uint texture_gl_id;
   uint32_t width;
   uint32_t height;
   gfx_tex_types uni_tex_type;
   gfx_tex_params prm;
   bool texture_updated = false;
};


class gfx_tex_2d : public gfx_tex
{
public:
   virtual ~gfx_tex_2d();

protected:
   gfx_tex_2d(std::string i_tex_name, const gfx_tex_params* i_prm = nullptr, mws_sp<gfx> i_gi = nullptr);

   friend class gfx;
};


class gfx_tex_3d : public gfx_tex
{
public:
   virtual ~gfx_tex_3d();

protected:
   gfx_tex_3d(std::string i_tex_name, mws_sp<gfx> i_gi = nullptr);

   friend class gfx;
};


class gfx_tex_cube_map : public gfx_tex
{
public:
   virtual ~gfx_tex_cube_map();

protected:
   gfx_tex_cube_map(std::string i_tex_name, mws_sp<gfx> i_gi = nullptr);
   gfx_tex_cube_map(uint32_t i_size, mws_sp<gfx> i_gi = nullptr);

   friend class gfx;
};
