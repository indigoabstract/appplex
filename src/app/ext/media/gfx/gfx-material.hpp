#pragma once

#include "pfm.hpp"
#include "gfx-scene.hpp"
#include "gfx-util.hpp"
#include "gfx-tex.hpp"
#include "gfx-shader.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class gfx;
class gfx_material;


const std::string MP_BLENDING = "blending";
const std::string MP_COLOR_WRITE = "color-write";
const std::string MP_CULL_BACK = "cull-back";
const std::string MP_CULL_FRONT = "cull-front";
const std::string MP_DEPTH_FUNCTION = "depth-function";
const std::string MP_DEPTH_TEST = "depth-test";
const std::string MP_DEPTH_WRITE = "depth-write";
const std::string MP_SHADER_INST = "shader-inst";
const std::string MP_SHADER_NAME = "shader-name";
const std::string MP_FSH_NAME = "fsh-name";
const std::string MP_VSH_NAME = "vsh-name";
const std::string MP_SCISSOR_ENABLED = "scissor-enabled";
const std::string MP_SCISSOR_AREA = "scissor-area";
const std::string MP_TEXTURE_INST = "texture-inst";
const std::string MP_TEXTURE_NAME = "texture-name";
const std::string MP_TEX_FILTER = "filter";
const std::string MP_TEX_ADDRU = "addr-u";
const std::string MP_TEX_ADDRV = "addr-v";
const std::string MP_TRANSPARENT_SORTING = "transparent-sorting";

const std::string MP_WIREFRAME_MODE = "wireframe-mode";
enum wireframe_mode
{
   MV_WF_NONE,
   MV_WF_OVERLAY,
   MV_WF_WIREFRAME_ONLY,
};

const std::string MV_NONE = "none";
const std::string MV_ALPHA = "alpha";
const std::string MV_ADD = "add";
const std::string MV_MUL = "multiply";

const std::string MV_NEVER = "never";
const std::string MV_LESS = "less";
const std::string MV_EQUAL = "equal";
const std::string MV_LESS_OR_EQUAL = "less-or-equal";
const std::string MV_GREATER = "greater";
const std::string MV_NOT_EQUAL = "not-equal";
const std::string MV_GREATER_OR_EQUAL = "greater-or-equal";
const std::string MV_ALWAYS = "always";

const std::string MV_CLAMP = "clamp";
const std::string MV_WRAP = "wrap";
const std::string MV_MIRROR = "mirror";

const std::string MV_MIN_MAG_MIP_NEAREST = "min-mag-mip-nearest";
const std::string MV_MIN_MAG_NEAREST_MIP_LINEAR = "min-mag-nearest-mip-linear";
const std::string MV_MIN_NEAREST_MAG_LINEAR_MIP_NEAREST = "min-nearest-mag-linear-mip-nearest";
const std::string MV_MIN_NEAREST_MAG_MIP_LINEAR = "min-nearest-mag-mip-linear";
const std::string MV_MIN_LINEAR_MAG_MIP_NEAREST = "min-linear-mag-mip-nearest";
const std::string MV_MIN_LINEAR_MAG_NEAREST_MIP_LINEAR = "min-linear-mag-nearest-mip-linear";
const std::string MV_MIN_MAG_LINEAR_MIP_NEAREST = "min-mag-linear-mip-nearest";
const std::string MV_MIN_MAG_MIP_LINEAR = "min-mag-mip-linear";
const std::string MV_ANISOTROPIC = "anisotropic";


class gfx_material_entry : public std::enable_shared_from_this < gfx_material_entry >
{
public:
   ~gfx_material_entry();
   shared_ptr<gfx_material_entry> get_inst();
   gfx_material_entry& operator[] (const std::string iname);

   //gl_material_entry& operator=(const bool ivalue);
   gfx_material_entry& operator=(const int ivalue);
   gfx_material_entry& operator=(const float ivalue);
   gfx_material_entry& operator=(const glm::vec2& ivalue);
   gfx_material_entry& operator=(const glm::vec3& ivalue);
   gfx_material_entry& operator=(const std::vector<glm::vec3>& ivalue);
   gfx_material_entry& operator=(const glm::vec4& ivalue);
   gfx_material_entry& operator=(const glm::mat2& ivalue);
   gfx_material_entry& operator=(const glm::mat3& ivalue);
   gfx_material_entry& operator=(const glm::mat4& ivalue);
   gfx_material_entry& operator=(const shared_ptr<gfx_tex> ivalue);
   gfx_material_entry& operator=(shared_ptr<gfx_shader> ivalue);
   gfx_material_entry& operator=(const std::string& ivalue);

   shared_ptr<gfx_material> get_material();
   bool empty_value();
   gfx_input::e_data_type get_value_type();
   bool get_bool_value();
   int get_int_value();
   float get_float_value();
   const glm::vec2& get_vec2_value();
   const glm::vec3& get_vec3_value();
   const std::vector<glm::vec3>* get_vec3_array_value();
   const glm::vec4& get_vec4_value();
   const glm::mat2& get_mat2_value();
   const glm::mat3& get_mat3_value();
   const glm::mat4& get_mat4_value();
   const shared_ptr<gfx_tex> get_tex_value();
   const std::string& get_text_value();

   void debug_print();

protected:
   friend class gfx_material;
   static shared_ptr<gfx_material_entry> new_inst(std::string iname, shared_ptr<gfx_material> imaterial_inst, shared_ptr<gfx_material_entry> iparent);
   gfx_material_entry();
   gfx_material_entry(std::string iname, shared_ptr<gfx_material> imaterial_inst, shared_ptr<gfx_material_entry> iparent);
   void delete_value();

   weak_ptr<gfx_material> root;
   weak_ptr<gfx_material_entry> parent;
   bool enabled;
   std::string name;
   gfx_input::e_data_type value_type;
   void* value;
   shared_ptr<gfx_tex> s2d_val;
   std::unordered_map<std::string, shared_ptr<gfx_material_entry> > entries;
};


class gfx_material : public std::enable_shared_from_this < gfx_material >
{
public:
   enum blending_modes
   {
      e_none,
      e_alpha,
      e_add,
      e_multiply,
   };

   gfx_material();
   static shared_ptr<gfx_material> new_inst();
   shared_ptr<gfx_material> get_inst();
   gfx_material_entry& operator[] (const std::string iname);
   shared_ptr<gfx_shader> get_shader();
   void set_mesh(shared_ptr<gfx_vxo> imesh);
   void clear_entries();
   void debug_print();

   weak_ptr<gfx_vxo> mesh;
   std::unordered_map<std::string, shared_ptr<gfx_material_entry> > std_params;
   std::unordered_map<std::string, shared_ptr<gfx_material_entry> > other_params;
   static bool is_std_param(const std::string& iparam_name);

private:
   friend class gfx;
   friend class gfx_material_entry;

   shared_ptr<gfx_shader> load_shader();
   static void init();

   shared_ptr<gfx_shader> shader;
   uint32 shader_compile_time;
   uint64 fsh_last_write;
   uint64 vsh_last_write;
   static std::unordered_map<std::string, shared_ptr<gfx_material_entry> > static_std_param;
};
