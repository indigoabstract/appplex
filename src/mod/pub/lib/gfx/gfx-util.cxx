#include "stdafx.hxx"

#include "gfx-util.hxx"
#include "gfx-shader.hxx"
#include "gfx.hxx"
#include "gfx-vxo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "pfm-gl.h"
#include <glm/inc.hpp>


gfx_tex_info::gfx_tex_info(std::string i_id, gfx_enum i_internal_format, gfx_enum i_format, gfx_enum i_type, int i_bpp)
{
   id = i_id;
   internal_format = i_internal_format;
   format = i_format;
   type = i_type;
   bpp = i_bpp;
}

gfx_tex_info::gfx_tex_info(const gfx_tex_info& i_obj)
{
   id = i_obj.id;
   internal_format = i_obj.internal_format;
   format = i_obj.format;
   type = i_obj.type;
   bpp = i_obj.bpp;
}

const std::string& gfx_tex_info::get_id() const
{
   return id;
}

gfx_enum gfx_tex_info::get_internal_format() const
{
   return internal_format;
}

gfx_enum gfx_tex_info::get_format() const
{
   return format;
}

gfx_enum gfx_tex_info::get_type() const
{
   return type;
}

int gfx_tex_info::get_bpp() const
{
   return bpp;
}

gfx_tex_info tex_info_tab[] =
{
   { "R8", GL_R8, GL_RED, GL_UNSIGNED_BYTE, 1, },
{ "R8_SNORM", GL_R8_SNORM, GL_RED, GL_BYTE, 1, },
{ "R16F", GL_R16F, GL_RED, GL_HALF_FLOAT, 2, },
{ "R32F", GL_R32F, GL_RED, GL_FLOAT, 4, },
{ "R8UI", GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, 1, },
{ "R8I", GL_R8I, GL_RED_INTEGER, GL_BYTE, 1, },
{ "R16UI", GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, 2, },
{ "R16I", GL_R16I, GL_RED_INTEGER, GL_SHORT, 2, },
{ "R32UI", GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, 4, },
{ "R32I", GL_R32I, GL_RED_INTEGER, GL_INT, 4, },
{ "RG8", GL_RG8, GL_RG, GL_UNSIGNED_BYTE, 2, },
{ "RG8_SNORM", GL_RG8_SNORM, GL_RG, GL_BYTE, 2, },
{ "RG16F", GL_RG16F, GL_RG, GL_HALF_FLOAT, 4, },
{ "RG32F", GL_RG32F, GL_RG, GL_FLOAT, 8, },
{ "RG8UI", GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, 2, },
{ "RG8I", GL_RG8I, GL_RG_INTEGER, GL_BYTE, 2, },
{ "RG16UI", GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, 4, },
{ "RG16I", GL_RG16I, GL_RG_INTEGER, GL_SHORT, 4, },
{ "RG32UI", GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, 8, },
{ "RG32I", GL_RG32I, GL_RG_INTEGER, GL_INT, 8, },
{ "RGB8", GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, 3, },
{ "SRGB8", GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE, 3, },
{ "RGB565", GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 2, },
{ "RGB8_SNORM", GL_RGB8_SNORM, GL_RGB, GL_BYTE, 3, },
{ "R11F_G11F_B10F", GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, 4, },
{ "RGB9_E5", GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, 4, },
{ "RGB16F", GL_RGB16F, GL_RGB, GL_HALF_FLOAT, 6, },
{ "RGB32F", GL_RGB32F, GL_RGB, GL_FLOAT, 12, },
{ "RGB8UI", GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, 3, },
{ "RGB8I", GL_RGB8I, GL_RGB_INTEGER, GL_BYTE, 3, },
{ "RGB16UI", GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, 6, },
{ "RGB16I", GL_RGB16I, GL_RGB_INTEGER, GL_SHORT, 6, },
{ "RGB32UI", GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, 12, },
{ "RGB32I", GL_RGB32I, GL_RGB_INTEGER, GL_INT, 12, },
{ "RGBA8", GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, },
{ "SRGB8_ALPHA8", GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, 4, },
{ "RGBA8_SNORM", GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, 4, },
{ "RGB5_A1", GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 2, },
{ "RGBA4", GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, 2, },
{ "RGB10_A2", GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, 4, },
{ "RGBA16F", GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, 8, },
{ "RGBA32F", GL_RGBA32F, GL_RGBA, GL_FLOAT, 16, },
{ "RGBA8UI", GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, 4, },
{ "RGBA8I", GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, 4, },
{ "RGB10_A2UI", GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, 4, },
{ "RGBA16UI", GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, 8, },
{ "RGBA16I", GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, 8, },
{ "RGBA32I", GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, 16, },
{ "RGBA32UI", GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, 16, },

{ "DEPTH_COMPONENT16", GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, 2, },
{ "DEPTH_COMPONENT24", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, 3, },
{ "DEPTH_COMPONENT32F", GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, 4, },
{ "DEPTH24_STENCIL8", GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, 4, },
{ "DEPTH32F_STENCIL8", GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, 5, },
};
int tex_info_tab_length = sizeof(tex_info_tab) / sizeof(gfx_tex_info);
std::unordered_map<std::string, mws_sp<gfx_tex_info> > tex_info_ht;
std::unordered_map<std::string, gfx_input::e_data_type> gfx_types;
std::unordered_map<gfx_int, std::string> gl_error_code_list;
std::unordered_map<std::string, std::string> gl_error_list;


mws_sp<gfx_tex_info> gfx_util::get_tex_info(std::string i_format)
{
   return tex_info_ht[i_format];
}

void gfx_util::init()
{
   for (int k = 0; k < tex_info_tab_length; k++)
   {
      gfx_tex_info& e = tex_info_tab[k];
      tex_info_ht[e.get_id()] = mws_sp<gfx_tex_info>(new gfx_tex_info(e));
   }

   gfx_types["bv1"] = gfx_input::bvec1;
   gfx_types["bv2"] = gfx_input::bvec2;
   gfx_types["bv3"] = gfx_input::bvec3;
   gfx_types["bv4"] = gfx_input::bvec4;
   gfx_types["iv1"] = gfx_input::ivec1;
   gfx_types["iv2"] = gfx_input::ivec2;
   gfx_types["iv3"] = gfx_input::ivec3;
   gfx_types["iv4"] = gfx_input::ivec4;
   gfx_types["v1"] = gfx_input::vec1;
   gfx_types["v2"] = gfx_input::vec2;
   gfx_types["v3"] = gfx_input::vec3;
   gfx_types["v4"] = gfx_input::vec4;
   gfx_types["m2"] = gfx_input::mat2;
   gfx_types["m3"] = gfx_input::mat3;
   gfx_types["m4"] = gfx_input::mat4;
   gfx_types["s2d"] = gfx_input::s2d;
   gfx_types["scm"] = gfx_input::scm;

   gl_error_code_list[GL_INVALID_ENUM] = "GL_INVALID_ENUM";
   gl_error_code_list[GL_INVALID_VALUE] = "GL_INVALID_VALUE";
   gl_error_code_list[GL_INVALID_OPERATION] = "GL_INVALID_OPERATION";
   gl_error_code_list[GL_OUT_OF_MEMORY] = "GL_OUT_OF_MEMORY";
   gl_error_code_list[GL_INVALID_FRAMEBUFFER_OPERATION] = "GL_INVALID_FRAMEBUFFER_OPERATION";

   gl_error_list["GL_INVALID_ENUM"] = "Given when an enumeration parameter is not a legal\
 enumeration for that function.This is given only for local problems; if the spec allows the\
 enumeration in certain circumstances, where other parameters or state dictate those circumstances,\
 then GL_INVALID_OPERATION is the result instead.";

   gl_error_list["GL_INVALID_VALUE"] = "Given when a value parameter is not a legal value for\
 that function.This is only given for local problems; if the spec allows the value in certain\
 circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION\
 is the result instead.";

   gl_error_list["GL_INVALID_OPERATION"] = "Given when the set of state for a command is not\
 legal for the parameters given to that command.It is also given for commands where combinations\
 of parameters define what the legal parameters are.";

   gl_error_list["GL_OUT_OF_MEMORY"] = "Given when performing an operation that can allocate\
 memory, and the memory cannot be allocated.The results of OpenGL functions that return this\
 error are undefined; it is allowable for partial operations to happen.";

   gl_error_list["GL_INVALID_FRAMEBUFFER_OPERATION"] = "Given when doing anything that would\
 attempt to read from or write / render to a framebuffer that is not complete.";
}

mws_sp<vx_attribute> gfx_util::parse_attribute(std::string iattribute)
{
   mws_sp<vx_attribute> a;

   if (!mws_str::starts_with(iattribute, "a_"))
   {
      mws_throw mws_exception("invalid iattribute");
   }

   int idx = iattribute.find('_', 2);

   if (idx == std::string::npos)
   {
      mws_throw mws_exception("invalid iattribute2");
   }

   std::string type = iattribute.substr(2, idx - 2);
   std::unordered_map<std::string, gfx_input::e_data_type>::iterator it = gfx_types.find(type);

   if (it == gfx_types.end() || it->second == gfx_input::s2d || it->second == gfx_input::scm)
   {
      mws_throw mws_exception("invalid iattribute3");
   }

   a = mws_sp<vx_attribute>(new vx_attribute(iattribute, it->second));

   return a;
}

std::vector<mws_sp<vx_attribute> > gfx_util::parse_attribute_list(std::string iattr_list)
{
   std::vector<mws_sp<vx_attribute> > v;

   int current_pos = 0;
   int size = iattr_list.length();

   while (current_pos < size)
   {
      int idx = iattr_list.find(',', current_pos);

      if (idx != std::string::npos)
      {
         std::string attr = iattr_list.substr(current_pos, idx - current_pos);

         attr = trim(attr);
         v.push_back(parse_attribute(attr));
      }
      else
      {
         std::string attr = iattr_list.substr(current_pos, iattr_list.length() - current_pos);

         attr = trim(attr);
         v.push_back(parse_attribute(attr));
         break;
      }

      current_pos = idx + 1;
   }

   return v;
}

mws_sp<gfx_uniform> gfx_util::parse_uniform(std::string iuniform)
{
   mws_sp<gfx_uniform> v;

   if (!mws_str::starts_with(iuniform, "u_"))
   {
      mws_throw mws_exception("invalid uniform");
   }

   int idx = iuniform.find('_', 2);

   if (idx == std::string::npos)
   {
      mws_throw mws_exception("invalid uniform2");
   }

   std::string type = iuniform.substr(2, idx - 2);
   std::unordered_map<std::string, gfx_input::e_data_type>::iterator it = gfx_types.find(type);

   if (it == gfx_types.end())
   {
      mws_throw mws_exception("invalid uniform3");
   }

   v = mws_sp<gfx_uniform>(new gfx_uniform(iuniform, it->second));

   return v;
}

void gfx_util::draw_tex(mws_sp<gfx_camera> i_cam, mws_sp<gfx_tex> itex, float itx, float ity)
{
   draw_tex(i_cam, itex, itx, ity, itex->get_width(), itex->get_height());
}

void gfx_util::draw_tex(mws_sp<gfx_camera> i_cam, mws_sp<gfx_tex> itex, float itx, float ity, float iw, float ih)
{
   auto q2d = gfx_quad_2d::nwi();

   q2d->set_dimensions(1, 1);
   q2d->set_translation(itx, ity);
   q2d->set_scale(iw, ih);
   (*q2d)[MP_SHADER_NAME] = "basic-tex-shader";
   (*q2d)["u_s2d_tex"][MP_TEXTURE_INST] = itex;
   (*q2d)[MP_DEPTH_TEST] = false;
   (*q2d)[MP_DEPTH_WRITE] = true;
   (*q2d)[MP_CULL_BACK] = false;
   //q2d->set_v_flip(true);
   //q2d->set_h_flip(true);
   q2d->draw_out_of_sync(i_cam);

   mws_report_gfx_errs();
}

const glm::vec4& gfx_util::get_pos_from_tf_mx(const glm::mat4& i_tranform)
{
   return i_tranform[3];
}

const glm::quat gfx_util::get_orientation_from_tf_mx(const glm::mat4& i_tranform)
{
   return glm::quat(1.f, 0.f, 0.f, 0.f);
}

const glm::vec3 gfx_util::get_scale_from_tf_mx(const glm::mat4& i_tranform)
{
   glm::vec3 scale(i_tranform[0].x, i_tranform[1].y, i_tranform[2].z);

   return glm::abs(scale);
}

glm::mat4 gfx_util::mat4_from_trs(const glm::vec3& translate, const glm::quat& rotate, const glm::vec3& scale)
{
   return glm::translate(translate) * glm::toMat4(rotate) * glm::scale(scale);
}

// http://lolengine.net/blog/2014/02/24/quaternion-from-two-vectors-final
// Build a unit quaternion representing the rotation from u to v. The input vectors need not be normalised.
glm::quat gfx_util::quat_from_two_vectors(glm::vec3 u, glm::vec3 v)
{
   float norm_u_norm_v = glm::sqrt(glm::dot(u, u) * glm::dot(v, v));
   float real_part = norm_u_norm_v + glm::dot(u, v);
   glm::vec3 w;

   if (real_part < 1.e-6f * norm_u_norm_v)
   {
      // If u and v are exactly opposite, rotate 180 degrees
      // around an arbitrary orthogonal axis. Axis normalisation
      // can happen later, when we normalise the quaternion.
      real_part = 0.0f;
      w = glm::abs(u.x) > glm::abs(u.z) ? glm::vec3(-u.y, u.x, 0.f) : glm::vec3(0.f, -u.z, u.y);
   }
   else
   {
      // Otherwise, build quaternion the standard way.
      w = glm::cross(u, v);
   }

   return glm::normalize(glm::quat(real_part, w.x, w.y, w.z));
}

// Returns a quaternion that will make your object looking towards 'direction'.
// Similar to RotationBetweenVectors, but also controls the vertical orientation.
// This assumes that at rest, the object faces +Z.
// Beware, the first parameter is a direction, not the target point !
glm::quat gfx_util::look_at(glm::vec3 direction, glm::vec3 desiredUp)
{
   direction = glm::normalize(direction);
   // Recompute desiredUp so that it's perpendicular to the direction
   // You can skip that part if you really want to force desiredUp
   glm::vec3 right = glm::cross(direction, desiredUp);
   desiredUp = glm::normalize(glm::cross(right, direction));

   // Find the rotation between the front of the object (that we assume towards +Z,
   // but this depends on your model) and the desired direction
   // glm::quat rot1 = glm::rotation(glm::vec3(0.0f, 0.0f, -1.0f), direction);
   glm::quat rot1 = quat_from_two_vectors(glm::vec3(0.0f, 0.0f, -1.0f), direction);
   // Because of the 1rst rotation, the up is probably completely screwed up. 
   // Find the rotation between the "up" of the rotated object, and the desired up
   glm::vec3 newUp = glm::normalize(rot1 * glm::vec3(0.0f, 1.0f, 0.0f));
   glm::quat rot2 = glm::rotation(newUp, desiredUp);

   // Apply them
   return rot2 * rot1; // remember, in reverse order.
}

int gfx_util::get_tex_2d_mipmap_count(int iwidth, int iheight)
{
   return int(floor(log2(std::max(iwidth, iheight))) + 1);
}

uint32 gfx_util::next_power_of_2(uint32 in)
{
   in -= 1;

   in |= in >> 16;
   in |= in >> 8;
   in |= in >> 4;
   in |= in >> 2;
   in |= in >> 1;

   return in + 1;
}

void mws_report_gfx_errs_impl(const char* i_file, uint32 i_line)
{
#if defined MWS_REPORT_GL_ERRORS

   int error_code = glGetError();

   if (error_code != 0)
   {
      if (!gl_error_code_list.empty() && (gl_error_code_list.find(error_code) != gl_error_code_list.end()))
      {
         std::string error_name = gl_error_code_list[error_code];
         std::string error_desc = gl_error_list[error_name];

         mws_print("gl error in file [%s] at line [%d] code [%d / 0x%x] name [%s] desc [%s]\n", i_file, i_line, error_code, error_code, error_name.c_str(), error_desc.c_str());
      }
      else
      {
         mws_print("gl error in file [%s] at line [%d] code [%d / 0x%x]\n", i_file, i_line, error_code, error_code);
      }

      mws_signal_error("gl-error");
   }

#endif
}
