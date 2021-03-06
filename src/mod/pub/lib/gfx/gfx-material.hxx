#pragma once

#include "pfm-def.h"
#include "krn.hxx"
#include "gfx-scene.hxx"
#include "gfx-util.hxx"
#include "gfx-tex.hxx"
#include "gfx-shader.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
#include <string>
#include <vector>
#include <unordered_map>
mws_pop_disable_all_warnings

class gfx;
class gfx_material;
class mws_any;


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
const std::string MV_ADD_COLOR = "add-color";
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
   virtual ~gfx_material_entry();
   mws_sp<gfx_material_entry> get_inst();
   gfx_material_entry& operator[] (const std::string i_name);

   gfx_material_entry& operator=(const int ivalue);
   gfx_material_entry& operator=(const float ivalue);
   gfx_material_entry& operator=(const glm::vec2& ivalue);
   gfx_material_entry& operator=(const glm::vec3& ivalue);
   gfx_material_entry& operator=(const std::vector<glm::vec3>& ivalue);
   gfx_material_entry& operator=(const glm::vec4& ivalue);
   gfx_material_entry& operator=(const glm::bvec4& i_value);
   gfx_material_entry& operator=(const glm::mat2& ivalue);
   gfx_material_entry& operator=(const glm::mat3& ivalue);
   gfx_material_entry& operator=(const glm::mat4& ivalue);
   gfx_material_entry& operator=(const mws_sp<gfx_tex> ivalue);
   gfx_material_entry& operator=(mws_sp<gfx_shader> ivalue);
   gfx_material_entry& operator=(const std::string& ivalue);

   mws_sp<gfx_material> get_material();
   bool empty_value();
   gfx_input::e_data_type get_value_type();
   template <typename T> T& get_value() { return mws_any_cast<T>(value); }
   mws_any* get_any() { return &value; }

   void debug_print();

protected:
   friend class gfx_material;
   static mws_sp<gfx_material_entry> nwi(std::string i_name, mws_sp<gfx_material> imaterial_inst, mws_sp<gfx_material_entry> i_parent);
   gfx_material_entry(std::string i_name, mws_sp<gfx_material> imaterial_inst, mws_sp<gfx_material_entry> i_parent);

   mws_wp<gfx_material> root;
   mws_wp<gfx_material_entry> parent;
   bool enabled;
   std::string name;
   gfx_input::e_data_type value_type;
   mws_any value;
   std::unordered_map<std::string, mws_sp<gfx_material_entry> > entries;
};


class gfx_material : public std::enable_shared_from_this < gfx_material >
{
public:
   enum blending_modes
   {
      e_none,
      e_alpha,
      e_add,
      e_add_color,
      e_multiply,
   };

   gfx_material(mws_sp<gfx> i_gi = nullptr);
   virtual ~gfx_material() {}
   static mws_sp<gfx_material> nwi(mws_sp<gfx> i_gi = nullptr);
   mws_sp<gfx_material> get_inst();
   gfx_material_entry& operator[] (const std::string i_name);
   mws_sp<gfx_shader> get_shader();
   void set_mesh(mws_sp<gfx_vxo> imesh);
   void clear_entries();
   void debug_print();

   mws_wp<gfx_vxo> mesh;
   std::unordered_map<std::string, mws_sp<gfx_material_entry> > std_params;
   std::unordered_map<std::string, mws_sp<gfx_material_entry> > other_params;
   static bool is_std_param(const std::string& i_param_name);

private:
   friend class gfx;
   friend class gfx_material_entry;

   mws_sp<gfx_shader> load_shader();
   static void init();

   mws_sp<gfx_shader> shader;
   uint32_t shader_compile_time;
   uint64_t fsh_last_write;
   uint64_t vsh_last_write;
   static std::unordered_map<std::string, mws_sp<gfx_material_entry> > static_std_param;
};
