#include "stdafx.h"

#include "gfx-util.hpp"
#include "min.hpp"
#include "gfx-shader.hpp"
#include "gfx.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "ext/gfx-surface.hpp"
#include "pfm-gl.h"
#include <glm/gtx/transform.hpp>


std::unordered_map<std::string, gfx_input::e_data_type> gfx_types;
std::unordered_map<gfx_int, std::string> gl_error_code_list;
std::unordered_map<std::string, std::string> gl_error_list;


void gfx_util::init()
{
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

std::shared_ptr<vx_attribute> gfx_util::parse_attribute(std::string iattribute)
{
   std::shared_ptr<vx_attribute> a;

   if (!mws_str::starts_with(iattribute, "a_"))
   {
      throw ia_exception("invalid iattribute");
   }

   int idx = iattribute.find('_', 2);

   if (idx == std::string::npos)
   {
      throw ia_exception("invalid iattribute2");
   }

   std::string type = iattribute.substr(2, idx - 2);
   std::unordered_map<std::string, gfx_input::e_data_type>::iterator it = gfx_types.find(type);

   if (it == gfx_types.end() || it->second == gfx_input::s2d || it->second == gfx_input::scm)
   {
      throw ia_exception("invalid iattribute3");
   }

   a = std::shared_ptr<vx_attribute>(new vx_attribute(iattribute, it->second));

   return a;
}

std::vector<std::shared_ptr<vx_attribute> > gfx_util::parse_attribute_list(std::string iattr_list)
{
   std::vector<std::shared_ptr<vx_attribute> > v;

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

std::shared_ptr<gfx_uniform> gfx_util::parse_uniform(std::string iuniform)
{
   std::shared_ptr<gfx_uniform> v;

   if (!mws_str::starts_with(iuniform, "u_"))
   {
      throw ia_exception("invalid uniform");
   }

   int idx = iuniform.find('_', 2);

   if (idx == std::string::npos)
   {
      throw ia_exception("invalid uniform2");
   }

   std::string type = iuniform.substr(2, idx - 2);
   std::unordered_map<std::string, gfx_input::e_data_type>::iterator it = gfx_types.find(type);

   if (it == gfx_types.end())
   {
      throw ia_exception("invalid uniform3");
   }

   v = std::shared_ptr<gfx_uniform>(new gfx_uniform(iuniform, it->second));

   return v;
}

void gfx_util::draw_tex(shared_ptr<gfx_tex> itex, float itx, float ity)
{
   draw_tex(itex, itx, ity, itex->get_width(), itex->get_height());
}

void gfx_util::draw_tex(shared_ptr<gfx_tex> itex, float itx, float ity, float iw, float ih)
{
   shared_ptr<gfx_shader> tex_dsp = gfx::shader::get_program_by_name("texture_display");

   if (tex_dsp)
   {
      gfx_tex_params prm;

      prm.internal_format = GL_RGBA8;
      prm.format = GL_RGBA;
      prm.type = GL_UNSIGNED_BYTE;

      int iw = itex->get_width(), ih = itex->get_height();
      shared_ptr<gfx_plane> q2d(new gfx_plane());
      shared_ptr<gfx_shader> current_program = gfx::shader::get_current_program();
      gfx::shader::set_current_program(tex_dsp);
      shared_ptr<gfx_tex> u_s2d_tex = gfx::tex::new_external_tex_2d("u_s2d_tex", itex->get_texture_gl_id(), iw, ih, &prm);

      u_s2d_tex->set_active(0);
      u_s2d_tex->send_uniform(u_s2d_tex->get_name(), 0);

      q2d->scaling = glm::vec3(iw, ih, 1.f);
      q2d->position = glm::vec3(itx, ity, 0.f);
      q2d->render_mesh(shared_ptr<gfx_camera>());

      gfx::shader::set_current_program(current_program);
      mws_report_gfx_errs();
   }
}

const glm::vec4& gfx_util::get_pos_from_tf_mx(const glm::mat4& i_tranform)
{
   return i_tranform[3];
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
      if (gl_error_code_list.find(error_code) != gl_error_code_list.end())
      {
         std::string error_name = gl_error_code_list[error_code];
         std::string error_desc = gl_error_list[error_name];

         mws_print("gl error in file [%s] at line [%d] code [%d / 0x%x] name [%s] desc [%s]\n", i_file, i_line, error_code, error_code, error_name.c_str(), error_desc.c_str());
      }
      else
      {
         mws_print("gl error in file [%s] at line [%d] code [%d / 0x%x]\n", i_file, i_line, error_code, error_code);
      }

      ia_signal_error();
   }

#endif
}
