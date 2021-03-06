#pragma once

#include "pfm-def.h"
#include "gfx-obj.hxx"
#include <memory>
#include <string>
#include <unordered_map>

class gfx_shader;
class gfx_shader_impl;
class mws_any;


// fast shortcuts for standard/predefined uniforms
enum class gfx_std_uni
{
   u_m4_model,
   u_m4_model_view,
   u_m4_model_view_proj,
   u_m4_projection,
   u_m4_view,
   u_m4_view_inv,
   u_s2d_tex,
   u_scm_tex,
   u_v4_color,
};


const std::string FS_EXT = ".fsh";
const std::string VS_EXT = ".vsh";


class gfx_input
{
public:
   enum e_input_type
   {
      e_attribute,
      e_uniform,
   };

   enum e_data_type
   {
      e_invalid,
      bvec1,
      bvec2,
      bvec3,
      bvec4,
      ivec1,
      ivec2,
      ivec3,
      ivec4,
      uvec1,
      uvec2,
      uvec3,
      uvec4,
      vec1,
      vec2,
      vec3,
      vec3_array,
      vec4,
      mat2,
      mat3,
      mat4,
      mat2x3,
      mat2x4,
      mat3x2,
      mat3x4,
      mat4x2,
      mat4x3,
      s2d,
      s3d,
      scm,
      text,
   };

   gfx_input()
   {
      array_size = 1;
      location = -1;
   }

   gfx_input(const std::string& i_name, e_input_type i_input_type, e_data_type i_data_type, gfx_int i_array_size, gfx_int i_location)
   {
      name = i_name;
      input_type = i_input_type;
      data_type = i_data_type;
      array_size = i_array_size;
      location = i_location;
   }

   const std::string& get_name()const
   {
      return name;
   }

   e_input_type get_input_type()
   {
      return input_type;
   }

   e_data_type get_data_type()const
   {
      return data_type;
   }

   gfx_int get_array_size()
   {
      return array_size;
   }

   gfx_int get_location()
   {
      return location;
   }

   static e_data_type from_gl_data_type(gfx_enum i_gl_data_type);

protected:
   std::string name;
   e_data_type data_type;
   e_input_type input_type;
   gfx_int array_size;
   gfx_int location;
};


class gfx_shader_listener
{
public:
   virtual const std::string on_before_submit_vsh_source(mws_sp<gfx_shader> i_gp, const std::string& i_shader_src) = 0;
   virtual const std::string on_before_submit_fsh_source(mws_sp<gfx_shader> i_gp, const std::string& i_shader_src) = 0;
};


class gfx_shader : public gfx_obj
{
public:
   ~gfx_shader();
   static mws_sp<gfx_shader> nwi
   (
      const std::string& i_prg_name, const std::string& i_shader_name,
      mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false, mws_sp<gfx> i_gi = nullptr
   );
   static mws_sp<gfx_shader> nwi
   (
      const std::string& i_prg_name, const std::string& i_vertex_shader_name, const std::string& i_fragment_shader_name,
      mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false, mws_sp<gfx> i_gi = nullptr
   );
   static mws_sp<gfx_shader> new_inst_inline
   (
      const std::string& i_prg_name, const std::string& i_vs_shader_src, const std::string& i_fs_shader_src,
      mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false, mws_sp<gfx> i_gi = nullptr
   );

   static std::string create_shader_id(const std::string& i_vertex_shader_name, const std::string& i_fragment_shader_name);
   virtual gfx_obj::e_gfx_obj_type get_type()const;
   virtual bool is_valid()const;
   mws_sp<gfx_shader> get_inst();
   const std::string& get_program_name();
   const std::string& get_shader_id();
   const std::string& get_fragment_shader_file_name();
   const std::string& get_fragment_shader_name();
   const std::string& get_vertex_shader_file_name();
   const std::string& get_vertex_shader_name();
   unsigned int get_program_id();
   void update_uniform(gfx_std_uni i_std_uni, const mws_any* i_val);
   // SLOWEST method for updating uniforms!
   void update_uniform(std::string i_uni_name, const void* i_val);
   void update_uniform(std::string i_uni_name, const mws_any* i_val);
   mws_sp<gfx_input> get_param(std::string i_key);
   mws_sp<gfx_input> remove_param(std::string i_key);
   gfx_int get_param_location(std::string i_key);
   bool contains_param(std::string i_param_name);
   void reload();
   void reload_on_modifications();
   void set_listener(mws_sp<gfx_shader_listener> i_listener);

private:
   friend class gfx;

   gfx_shader(const std::string& i_prg_name, mws_sp<gfx> i_gi = nullptr);
   void release();
   bool make_current();
   void update_uniform(mws_sp<gfx_input> i_input, const mws_any* i_val);

   mws_sp<gfx_shader_impl> p;
};
