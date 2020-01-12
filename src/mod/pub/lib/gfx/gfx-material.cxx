#include "stdafx.hxx"

#include "gfx-vxo.hxx"
#include "gfx.hxx"
#include "gfx-util.hxx"
#include "gfx-shader.hxx"
#include "gfx-camera.hxx"
#include "gfx-state.hxx"
#include "pfm-gl.h"
#include <glm/inc.hpp>

static std::string depth_func_name_list[] = { MV_NEVER, MV_LESS, MV_EQUAL, MV_LESS_OR_EQUAL, MV_GREATER, MV_NOT_EQUAL, MV_GREATER_OR_EQUAL, MV_ALWAYS };
static gfx_uint depth_func_list[] = { gl::NEVER_GL, gl::LESS_GL, gl::EQUAL_GL, gl::LEQUAL_GL, gl::GREATER_GL, gl::NOTEQUAL_GL, gl::GEQUAL_GL, gl::ALWAYS_GL };
static int depth_func_list_length = sizeof(depth_func_list) / sizeof(gfx_uint);

static std::string blending_name_list[] = { MV_NONE, MV_ALPHA, MV_ADD, MV_ADD_COLOR, MV_MUL };
static gfx_uint blending_list[] = { gfx_material::e_none, gfx_material::e_alpha, gfx_material::e_add, gfx_material::e_add_color, gfx_material::e_multiply };
static int blending_list_length = sizeof(blending_list) / sizeof(gfx_uint);


mws_sp<gfx_material_entry> gfx_material_entry::nwi(std::string i_name, mws_sp<gfx_material> imaterial_inst, mws_sp<gfx_material_entry> i_parent)
{
   return mws_sp<gfx_material_entry>(new gfx_material_entry(i_name, imaterial_inst, i_parent));
}

mws_sp<gfx_material_entry> gfx_material_entry::get_inst()
{
   return shared_from_this();
}

gfx_material_entry::gfx_material_entry(std::string i_name, mws_sp<gfx_material> imaterial_inst, mws_sp<gfx_material_entry> i_parent)
{
   root = imaterial_inst;
   parent = i_parent;
   enabled = true;
   name = i_name;
   value.clear();
   value_type = gfx_input::e_invalid;

   if (name == MP_COLOR_WRITE)
   {
      value = glm::bvec4(true);
      value_type = gfx_input::bvec4;
   }
   else if (name == MP_CULL_BACK)
   {
      value = true;
      value_type = gfx_input::bvec1;
   }
   else if (name == MP_CULL_FRONT)
   {
      value = false;
      value_type = gfx_input::bvec1;
   }
   else if (name == MP_DEPTH_TEST)
   {
      value = true;
      value_type = gfx_input::bvec1;
   }
   else if (name == MP_DEPTH_WRITE)
   {
      value = true;
      value_type = gfx_input::bvec1;
   }
   else if (name == MP_SCISSOR_ENABLED)
   {
      value = false;
      value_type = gfx_input::bvec1;
   }
   else if (name == MP_BLENDING)
   {
      value = gfx_material::e_none;
      value_type = gfx_input::ivec1;
   }
   else if (name == MP_DEPTH_FUNCTION)
   {
      value = gl::LESS_GL;
      value_type = gfx_input::ivec1;
   }
   else if (name == MP_WIREFRAME_MODE)
   {
      value = MV_WF_NONE;
      value_type = gfx_input::ivec1;
   }
}

gfx_material_entry::~gfx_material_entry()
{
   value.clear();
}

gfx_material_entry& gfx_material_entry::operator[] (const std::string i_name)
{
   if (!entries[i_name])
   {
      entries[i_name] = nwi(i_name, get_material(), get_inst());
   }

   return *entries[i_name];
}

gfx_material_entry& gfx_material_entry::operator=(const int ivalue)
{
   value_type = gfx_input::ivec1;
   value = (int)ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const float ivalue)
{
   value_type = gfx_input::vec1;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec2& ivalue)
{
   value_type = gfx_input::vec2;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec3& ivalue)
{
   value_type = gfx_input::vec3;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const std::vector<glm::vec3>& ivalue)
{
   value_type = gfx_input::vec3_array;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::vec4& ivalue)
{
   value_type = gfx_input::vec4;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::bvec4& ivalue)
{
   value_type = gfx_input::bvec4;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat2& ivalue)
{
   value_type = gfx_input::mat2;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat3& ivalue)
{
   value_type = gfx_input::mat3;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const glm::mat4& ivalue)
{
   value_type = gfx_input::mat4;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const mws_sp<gfx_tex> ivalue)
{
   parent.lock()->value_type = gfx_input::s2d;
   value_type = gfx_input::s2d;
   value = ivalue;

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(mws_sp<gfx_shader> ivalue)
{
   value_type = gfx_input::e_invalid;
   value = ivalue;

   if (name == MP_SHADER_INST)
   {
      get_material()->shader = ivalue;
   }

   return *this;
}

gfx_material_entry& gfx_material_entry::operator=(const std::string& ivalue)
{
   value = ivalue;

   if (gfx_material::is_std_param(name))
   {
      value_type = gfx_input::text;
      if (name == MP_SHADER_NAME)
      {
         entries[MP_FSH_NAME] = nwi(MP_FSH_NAME, get_material(), get_inst());
         entries[MP_VSH_NAME] = nwi(MP_VSH_NAME, get_material(), get_inst());
         *entries[MP_FSH_NAME] = ivalue;
         *entries[MP_VSH_NAME] = ivalue;

         get_material()->shader = nullptr;
      }
      else if (name == MP_FSH_NAME)
      {
         get_material()->shader = nullptr;
      }
      else if (name == MP_VSH_NAME)
      {
         get_material()->shader = nullptr;
      }
      else if (name == MP_DEPTH_FUNCTION)
      {
         value_type = gfx_input::ivec1;
         int val = gl::LESS_GL;

         for (int k = 0; k < depth_func_list_length; k++)
         {
            if (ivalue == depth_func_name_list[k])
            {
               val = depth_func_list[k];
               break;
            }
         }

         value = val;
      }
      else if (name == MP_BLENDING)
      {
         value_type = gfx_input::ivec1;
         int val = gfx_material::e_none;

         for (int k = 0; k < blending_list_length; k++)
         {
            if (ivalue == blending_name_list[k])
            {
               val = blending_list[k];
               break;
            }
         }

         value = val;
      }
   }
   else
      // custom parameter
   {
      mws_sp<gfx_shader> sh = get_material()->shader;
      mws_sp<gfx_input> param = sh->get_param(name);

      value_type = gfx_input::e_invalid;

      if (param && param->get_input_type() == gfx_input::e_uniform)
      {
         switch (param->get_data_type())
         {
         case gfx_input::s2d:
         {
            value_type = gfx_input::s2d;
            entries[MP_TEXTURE_NAME] = nwi(MP_TEXTURE_NAME, get_material(), get_inst());
            // the texture will be loaded by the scene 'update' method
            entries[MP_TEXTURE_INST] = nwi(MP_TEXTURE_INST, get_material(), get_inst());
            entries[MP_TEX_FILTER] = nwi(MP_TEX_FILTER, get_material(), get_inst());
            entries[MP_TEX_ADDRU] = nwi(MP_TEX_ADDRU, get_material(), get_inst());
            entries[MP_TEX_ADDRV] = nwi(MP_TEX_ADDRV, get_material(), get_inst());
            *entries[MP_TEXTURE_NAME] = ivalue;
            *entries[MP_TEX_FILTER] = MV_MIN_MAG_MIP_LINEAR;
            *entries[MP_TEX_ADDRU] = MV_WRAP;
            *entries[MP_TEX_ADDRV] = MV_WRAP;
            break;
         }

         case gfx_input::s3d:
         {
            value_type = gfx_input::s3d;
            mws_throw mws_exception("s3d not implemented");
            break;
         }

         case gfx_input::scm:
         {
            value_type = gfx_input::scm;
            entries[MP_TEXTURE_NAME] = nwi(MP_TEXTURE_NAME, get_material(), get_inst());
            // the texture will be loaded by the scene 'update' method
            entries[MP_TEXTURE_INST] = nwi(MP_TEXTURE_INST, get_material(), get_inst());
            entries[MP_TEX_FILTER] = nwi(MP_TEX_FILTER, get_material(), get_inst());
            entries[MP_TEX_ADDRU] = nwi(MP_TEX_ADDRU, get_material(), get_inst());
            entries[MP_TEX_ADDRV] = nwi(MP_TEX_ADDRV, get_material(), get_inst());
            *entries[MP_TEXTURE_NAME] = ivalue;
            *entries[MP_TEX_FILTER] = MV_MIN_MAG_MIP_LINEAR;
            *entries[MP_TEX_ADDRU] = MV_WRAP;
            *entries[MP_TEX_ADDRV] = MV_WRAP;
            break;
         }
         }
      }
   }

   return *this;
}

mws_sp<gfx_material> gfx_material_entry::get_material()
{
   return root.lock();
}

bool gfx_material_entry::empty_value()
{
   bool empty = value.empty();

   return empty;
}

gfx_input::e_data_type gfx_material_entry::get_value_type()
{
   return value_type;
}

void gfx_material_entry::debug_print()
{
   std::unordered_map<std::string, mws_sp<gfx_material_entry> >::iterator it = entries.begin();

   mws_print("[");
   for (; it != entries.end(); ++it)
   {
      mws_print("[name[%s] ", it->first.c_str());
      it->second->debug_print();
      mws_print("], ");
   }

   if (value_type != -1)
   {
      mws_print(" vt %d", value_type);

      if (value_type == gfx_input::text)
      {
         //std::string* s = (std::string*)value;
         auto& s = get_value<std::string>();
         mws_print(" text [%s]", s.c_str());
      }
      else if (value_type == gfx_input::vec3)
      {
         //glm::vec3* v = (glm::vec3*)value;
         auto& v = get_value<glm::vec3>();
         mws_print(" vec3 [%f, %f, %f]", v.x, v.y, v.z);
      }
   }
   else
   {
      mws_print(" null");
   }

   mws_print("]");
}

std::unordered_map<std::string, mws_sp<gfx_material_entry> > gfx_material::static_std_param;

gfx_material::gfx_material(mws_sp<gfx> i_gi)
{
   shader_compile_time = 0;
   fsh_last_write = 0;
   vsh_last_write = 0;
}

mws_sp<gfx_material> gfx_material::nwi(mws_sp<gfx> i_gi)
{
   mws_sp<gfx_material> m(new gfx_material(i_gi));
   gfx_material& inst = *m;

   return m;
}

mws_sp<gfx_material> gfx_material::get_inst()
{
   return shared_from_this();
}

gfx_material_entry& gfx_material::operator[] (const std::string i_name)
{
   if (is_std_param(i_name))
      // if it's a standard parameter
   {
      mws_sp<gfx_material_entry> me = std_params[i_name];

      if (!me)
      {
         me = std_params[i_name] = gfx_material_entry::nwi(i_name, get_inst(), nullptr);
      }

      return *me;
   }

   // this is a custom parameter. shader needs to be loaded to check for the parameter's type
   if (!shader)
   {
      load_shader();
   }

   if (!other_params[i_name])
   {
      other_params[i_name] = gfx_material_entry::nwi(i_name, get_inst(), nullptr);
   }

   return *other_params[i_name];
}

mws_sp<gfx_shader> gfx_material::get_shader()
{
   return load_shader();
}

void gfx_material::set_mesh(mws_sp<gfx_vxo> imesh)
{
   mesh = imesh;
}

void gfx_material::clear_entries()
{
   other_params.clear();
}

void gfx_material::debug_print()
{
   std::unordered_map<std::string, mws_sp<gfx_material_entry> >::iterator it = other_params.begin();

   mws_print("[\n");

   for (; it != other_params.end(); ++it)
   {
      mws_print("[%s\t", it->first.c_str());
      it->second->debug_print();
      mws_print("]\n");
   }

   mws_print("\n]");
}

bool gfx_material::is_std_param(const std::string& i_param_name)
{
   return static_std_param.find(i_param_name) != static_std_param.end();
}

mws_sp<gfx_shader> gfx_material::load_shader()
{
   if (!shader)
   {
      gfx_material& inst = *this;
      gfx_material_entry& fsh = inst[MP_SHADER_NAME][MP_FSH_NAME];
      gfx_material_entry& vsh = inst[MP_SHADER_NAME][MP_VSH_NAME];

      if (!fsh.empty_value() && !vsh.empty_value())
      {
         std::string fsh_name = fsh.get_value<std::string>();
         std::string vsh_name = vsh.get_value<std::string>();
         std::string shader_id;

         if (fsh_name == vsh_name)
         {
            shader = mesh.lock()->gi()->shader.get_program_by_name(vsh_name);
         }
         else
         {
            shader_id = gfx_shader::create_shader_id(vsh_name, fsh_name);
            shader = mesh.lock()->gi()->shader.get_program_by_shader_id(shader_id);
         }

         if (!shader)
         {
            shader = mesh.lock()->gi()->shader.new_program(shader_id, vsh_name, fsh_name);
            shader_compile_time = pfm::time::get_time_millis();
         }
      }
      else
      {
         mws_print("gl_material::load_shader(): failed to load shader. switching to default\n");
         return mesh.lock()->gi()->shader.get_program_by_name(gfx::black_sh_id);
      }
   }
   else
   {
#if defined PLATFORM_WINDOWS_PC && defined MWS_DEBUG_BUILD
      shader->reload_on_modifications();
#endif
   }

   return shader;
}

void gfx_material::init()
{
   mws_sp<gfx_material> mi;

   static_std_param[MP_BLENDING] = gfx_material_entry::nwi(MP_BLENDING, mi, nullptr);
   static_std_param[MP_COLOR_WRITE] = gfx_material_entry::nwi(MP_COLOR_WRITE, mi, nullptr);
   static_std_param[MP_CULL_BACK] = gfx_material_entry::nwi(MP_CULL_BACK, mi, nullptr);
   static_std_param[MP_CULL_FRONT] = gfx_material_entry::nwi(MP_CULL_FRONT, mi, nullptr);
   static_std_param[MP_DEPTH_FUNCTION] = gfx_material_entry::nwi(MP_DEPTH_FUNCTION, mi, nullptr);
   static_std_param[MP_DEPTH_TEST] = gfx_material_entry::nwi(MP_DEPTH_TEST, mi, nullptr);
   static_std_param[MP_DEPTH_WRITE] = gfx_material_entry::nwi(MP_DEPTH_WRITE, mi, nullptr);
   static_std_param[MP_SHADER_INST] = gfx_material_entry::nwi(MP_SHADER_INST, mi, nullptr);
   static_std_param[MP_SHADER_NAME] = gfx_material_entry::nwi(MP_SHADER_NAME, mi, nullptr);
   static_std_param[MP_FSH_NAME] = gfx_material_entry::nwi(MP_FSH_NAME, mi, nullptr);
   static_std_param[MP_VSH_NAME] = gfx_material_entry::nwi(MP_VSH_NAME, mi, nullptr);
   static_std_param[MP_SCISSOR_ENABLED] = gfx_material_entry::nwi(MP_SCISSOR_ENABLED, mi, nullptr);
   static_std_param[MP_SCISSOR_AREA] = gfx_material_entry::nwi(MP_SCISSOR_AREA, mi, nullptr);
   static_std_param[MP_TEXTURE_INST] = gfx_material_entry::nwi(MP_TEXTURE_INST, mi, nullptr);
   static_std_param[MP_TEXTURE_NAME] = gfx_material_entry::nwi(MP_TEXTURE_NAME, mi, nullptr);
   static_std_param[MP_TEX_FILTER] = gfx_material_entry::nwi(MP_TEX_FILTER, mi, nullptr);
   static_std_param[MP_TEX_ADDRU] = gfx_material_entry::nwi(MP_TEX_ADDRU, mi, nullptr);
   static_std_param[MP_TEX_ADDRV] = gfx_material_entry::nwi(MP_TEX_ADDRV, mi, nullptr);
   static_std_param[MP_TRANSPARENT_SORTING] = gfx_material_entry::nwi(MP_TRANSPARENT_SORTING, mi, nullptr);
   static_std_param[MP_WIREFRAME_MODE] = gfx_material_entry::nwi(MP_WIREFRAME_MODE, mi, nullptr);
}
