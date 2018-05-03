#pragma once

#include "pfm.hpp"
#include "gfx-obj.hpp"
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


// avoid using this if shader contains preprocessing macros
// on android, line s containing #ifdef GL_ES seem to get left out,
// which means the shader will FAIL to compile
#define GLSL_SRC(src) "\n" #src
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

   gfx_input(const std::string& iname, e_input_type iinput_type, e_data_type idata_type, gfx_int iarray_size, gfx_int ilocation)
   {
      name = iname;
      input_type = iinput_type;
      data_type = idata_type;
      array_size = iarray_size;
      location = ilocation;
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

   static e_data_type from_gl_data_type(gfx_enum gl_data_type);

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
   virtual const std::shared_ptr<std::string> on_before_submit_vsh_source
   (std::shared_ptr<gfx_shader> gp, const std::shared_ptr<std::string> ishader_src) = 0;
   virtual const std::shared_ptr<std::string> on_before_submit_fsh_source
   (std::shared_ptr<gfx_shader> gp, const std::shared_ptr<std::string> ishader_src) = 0;
};


class gfx_shader : public gfx_obj
{
public:
   ~gfx_shader();
   static std::shared_ptr<gfx_shader> new_inst
   (
      const std::string& iprg_name, const std::string& ishader_name,
      std::shared_ptr<gfx_shader_listener> ilistener = nullptr, std::shared_ptr<gfx> i_gi = nullptr
   );
   static std::shared_ptr<gfx_shader> new_inst
   (
      const std::string& iprg_name, const std::string& ivertex_shader_name, const std::string& ifragment_shader_name,
      std::shared_ptr<gfx_shader_listener> ilistener = nullptr, std::shared_ptr<gfx> i_gi = nullptr
   );
   static std::shared_ptr<gfx_shader> new_inst_inline
   (
      const std::string& iprg_name, const std::shared_ptr<std::string> ivs_shader_src, const std::shared_ptr<std::string> ifs_shader_src,
      std::shared_ptr<gfx_shader_listener> ilistener = nullptr, std::shared_ptr<gfx> i_gi = nullptr
   );

   static std::string create_shader_id(std::string ivertex_shader_name, std::string ifragment_shader_name);
   virtual gfx_obj::e_gfx_obj_type get_type()const;
   virtual bool is_valid()const;
   std::shared_ptr<gfx_shader> get_inst();
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
   std::shared_ptr<gfx_input> get_param(std::string ikey);
   std::shared_ptr<gfx_input> remove_param(std::string ikey);
   gfx_int get_param_location(std::string ikey);
   bool contains_param(std::string iparam_name);
   void reload();
   void reload_on_modifications();
   void set_listener(std::shared_ptr<gfx_shader_listener> ilistener);

private:
   friend class gfx;

   gfx_shader(const std::string& iprg_name, std::shared_ptr<gfx> i_gi = nullptr);
   void release();
   bool make_current();
   void update_uniform(std::shared_ptr<gfx_input> i_input, const mws_any* i_val);

   std::shared_ptr<gfx_shader_impl> p;
};
