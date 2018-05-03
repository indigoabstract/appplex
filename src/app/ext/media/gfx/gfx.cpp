#include "stdafx.h"

#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-tex.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include "min.hpp"
#include "pfm-gl.h"
#include <unordered_map>


gfx_int gfx::default_framebuffer_id = 0;
mws_sp<gfx> gfx::main_instance;


struct tex_info
{
   const char* id;
   gfx_enum internal_format;
   gfx_enum format;
   gfx_enum type;
};

tex_info tex_info_tab[] =
{
   { "R8", GL_R8, GL_RED, GL_UNSIGNED_BYTE, },
   { "R8_SNORM", GL_R8_SNORM, GL_RED, GL_BYTE, },
   { "R16F", GL_R16F, GL_RED, GL_HALF_FLOAT, },
   { "R32F", GL_R32F, GL_RED, GL_FLOAT, },
   { "R8UI", GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE, },
   { "R8I", GL_R8I, GL_RED_INTEGER, GL_BYTE, },
   { "R16UI", GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, },
   { "R16I", GL_R16I, GL_RED_INTEGER, GL_SHORT, },
   { "R32UI", GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT, },
   { "R32I", GL_R32I, GL_RED_INTEGER, GL_INT, },
   { "RG8", GL_RG8, GL_RG, GL_UNSIGNED_BYTE, },
   { "RG8_SNORM", GL_RG8_SNORM, GL_RG, GL_BYTE, },
   { "RG16F", GL_RG16F, GL_RG, GL_HALF_FLOAT, },
   { "RG32F", GL_RG32F, GL_RG, GL_FLOAT, },
   { "RG8UI", GL_RG8UI, GL_RG_INTEGER, GL_UNSIGNED_BYTE, },
   { "RG8I", GL_RG8I, GL_RG_INTEGER, GL_BYTE, },
   { "RG16UI", GL_RG16UI, GL_RG_INTEGER, GL_UNSIGNED_SHORT, },
   { "RG16I", GL_RG16I, GL_RG_INTEGER, GL_SHORT, },
   { "RG32UI", GL_RG32UI, GL_RG_INTEGER, GL_UNSIGNED_INT, },
   { "RG32I", GL_RG32I, GL_RG_INTEGER, GL_INT, },
   { "RGB8", GL_RGB8, GL_RGB, GL_UNSIGNED_BYTE, },
   { "SRGB8", GL_SRGB8, GL_RGB, GL_UNSIGNED_BYTE, },
   { "RGB565", GL_RGB565, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, },
   { "RGB8_SNORM", GL_RGB8_SNORM, GL_RGB, GL_BYTE, },
   { "R11F_G11F_B10F", GL_R11F_G11F_B10F, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, },
   { "RGB9_E5", GL_RGB9_E5, GL_RGB, GL_UNSIGNED_INT_5_9_9_9_REV, },
   { "RGB16F", GL_RGB16F, GL_RGB, GL_HALF_FLOAT, },
   { "RGB32F", GL_RGB32F, GL_RGB, GL_FLOAT, },
   { "RGB8UI", GL_RGB8UI, GL_RGB_INTEGER, GL_UNSIGNED_BYTE, },
   { "RGB8I", GL_RGB8I, GL_RGB_INTEGER, GL_BYTE, },
   { "RGB16UI", GL_RGB16UI, GL_RGB_INTEGER, GL_UNSIGNED_SHORT, },
   { "RGB16I", GL_RGB16I, GL_RGB_INTEGER, GL_SHORT, },
   { "RGB32UI", GL_RGB32UI, GL_RGB_INTEGER, GL_UNSIGNED_INT, },
   { "RGB32I", GL_RGB32I, GL_RGB_INTEGER, GL_INT, },
   { "RGBA8", GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, },
   { "SRGB8_ALPHA8", GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE, },
   { "RGBA8_SNORM", GL_RGBA8_SNORM, GL_RGBA, GL_BYTE, },
   { "RGB5_A1", GL_RGB5_A1, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, },
   { "RGBA4", GL_RGBA4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4, },
   { "RGB10_A2", GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, },
   { "RGBA16F", GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, },
   { "RGBA32F", GL_RGBA32F, GL_RGBA, GL_FLOAT, },
   { "RGBA8UI", GL_RGBA8UI, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, },
   { "RGBA8I", GL_RGBA8I, GL_RGBA_INTEGER, GL_BYTE, },
   { "RGB10_A2UI", GL_RGB10_A2UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, },
   { "RGBA16UI", GL_RGBA16UI, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, },
   { "RGBA16I", GL_RGBA16I, GL_RGBA_INTEGER, GL_SHORT, },
   { "RGBA32I", GL_RGBA32I, GL_RGBA_INTEGER, GL_INT, },
   { "RGBA32UI", GL_RGBA32UI, GL_RGBA_INTEGER, GL_UNSIGNED_INT, },

   { "DEPTH_COMPONENT16", GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, },
   { "DEPTH_COMPONENT24", GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, },
   { "DEPTH_COMPONENT32F", GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT, },
   { "DEPTH24_STENCIL8", GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, },
   { "DEPTH32F_STENCIL8", GL_DEPTH32F_STENCIL8, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, },
};
int tex_info_tab_length = sizeof(tex_info_tab) / sizeof(tex_info);
std::unordered_map<std::string, tex_info*> tex_info_ht;

void mws_tex_img_2d(GLenum target, GLint mipmap_count, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void* pixels)
{
#ifdef USES_GL_TEX_STORAGE_2D

   glTexStorage2D(target, mipmap_count, internalformat, width, height);

#else

   glTexImage2D(target, 0, internalformat, width, height, border, format, type, pixels);

#endif
}


void gfx::global_init()
{
   if (main_instance)
   {
      mws_print("gl_ctrl::init: this method must only be called once\n");
      //reload();
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer_id);
      //throw mws_exception("this method must only be called once");
      return;
   }

   bool print_extensions = false;
   int gl_extension_count = -1;
   int gl_major_version = -1;
   int gl_minor_version = -1;
   const gfx_ubyte* version = glGetString(GL_VERSION);
   const gfx_ubyte* renderer = glGetString(GL_RENDERER);
   const gfx_ubyte* vendor = glGetString(GL_VENDOR);

   glGetIntegerv(GL_NUM_EXTENSIONS, &gl_extension_count);
   glGetIntegerv(GL_MAJOR_VERSION, &gl_major_version);
   glGetIntegerv(GL_MINOR_VERSION, &gl_minor_version);

   mws_print("gl-version [%d.%d] / [%s]\ngl-renderer [%s]\ngl-vendor [%s]\n", gl_major_version, gl_minor_version, version, renderer, vendor);

#ifdef USES_GL_STRINGI

   if (gl_extension_count > 0)
   {
      std::vector<const gfx_ubyte*> extensions;

      mws_print("[%d] gl-extensions found.\n", gl_extension_count);

      if (print_extensions)
      {
         extensions.resize(gl_extension_count);

         for (int k = 0; k < gl_extension_count; k++)
         {
            extensions[k] = glGetStringi(GL_EXTENSIONS, k);
            mws_print("%s\n", extensions[k]);
         }
      }

      mws_print("\n");
   }

#else

   if (print_extensions)
   {
      const GLubyte* extensions = glGetString(GL_EXTENSIONS);
      mws_print("gl-extensions: %s\n", extensions);
   }

#endif

   // color/depth/stencil buffer info
   {
      GLint red_bits = 0;
      GLint green_bits = 0;
      GLint blue_bits = 0;
      GLint alpha_bits = 0;
      GLint depth_bits = 0;
      GLint stencil_bits = 0;

      glGetIntegerv(GL_RED_BITS, &red_bits);
      glGetIntegerv(GL_GREEN_BITS, &green_bits);
      glGetIntegerv(GL_BLUE_BITS, &blue_bits);
      glGetIntegerv(GL_ALPHA_BITS, &alpha_bits);
      glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
      glGetIntegerv(GL_STENCIL_BITS, &stencil_bits);

      mws_print("red_bits [%d] green_bits [%d] blue_bits [%d] alpha_bits [%d] depth_bits [%d] stencil_bits [%d]\n",
         red_bits, green_bits, blue_bits, alpha_bits, depth_bits, stencil_bits);
   }

   // samples
   {
      GLint samples = 0, max_samples = 0;
      glGetIntegerv(GL_SAMPLES, &samples);
      glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
      mws_print("sample coverage: %d, max samples: %d\n", samples, max_samples);
   }

   //inst = shared_ptr<gfx>(new gfx());

   for (int k = 0; k < tex_info_tab_length; k++)
   {
      tex_info& e = tex_info_tab[k];
      tex_info_ht[e.id] = &e;
   }

   glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer_id);

   //int ms = 8;
   //GLuint mTextureFramebuffer = 0;
   //glGenRenderbuffers(1, &mTextureFramebuffer);
   //glBindRenderbuffer(GL_RENDERBUFFER, mTextureFramebuffer);
   //glRenderbufferStorageMultisample(GL_RENDERBUFFER, /* Number of samples */ ms, GL_R8, rt::get_screen_width(), rt::get_screen_height());
   //GLenum err = glGetError();

   //if (err != GL_NO_ERROR)
   //{
   //   bool MultisampleSupported = false;
   //   mws_print("MultisampleSupported %d false\n", ms);
   //}
   //else
   //{
   //   mws_print("MultisampleSupported %d true\n", ms);
   //}

   gfx_util::init();
   gfx_material::init();

   main_instance = std::shared_ptr<gfx>(new gfx());
   main_instance->init(main_instance);
}

std::shared_ptr<gfx> gfx::new_inst()
{
   auto inst = std::shared_ptr<gfx>(new gfx());
   inst->init(inst);
   return inst;
}

shared_ptr<gfx_state> gfx::get_gfx_state()
{
   return gfx_state_inst;
}

void gfx::reload()
{
   for (auto it = rt_list.begin(); it != rt_list.end(); it++)
   {
      shared_ptr<gfx_rt> rt = it->lock();

      rt->reload();
   }

   for (auto it = shader_list.begin(); it != shader_list.end(); it++)
   {
      shared_ptr<gfx_shader> prg = it->lock();

      prg->reload();
   }

   for (auto it = tex_list.begin(); it != tex_list.end(); it++)
   {
      shared_ptr<gfx_tex> tex = it->lock();

      tex->reload();
   }

   mws_report_gfx_errs();
}

std::shared_ptr<gfx_rt> gfx::ic_rt::new_rt()
{
   std::shared_ptr<gfx_rt> rt = std::shared_ptr<gfx_rt>(new gfx_rt(gi()));
   gi()->rt_list.push_back(rt);

   return rt;
}

int gfx::ic_rt::get_screen_width()
{
   return pfm::screen::get_width();
}

int gfx::ic_rt::get_screen_height()
{
   return pfm::screen::get_height();
}

int gfx::ic_rt::get_render_target_width()
{
   return (gi()->active_rt) ? gi()->active_rt->get_width() : get_screen_width();
}

int gfx::ic_rt::get_render_target_height()
{
   return (gi()->active_rt) ? gi()->active_rt->get_height() : get_screen_height();
}

std::shared_ptr<gfx_rt> gfx::ic_rt::get_current_render_target()
{
   return gi()->active_rt;
}

void gfx::ic_rt::set_current_render_target(std::shared_ptr<gfx_rt> irdt)
{
   int width = 0, height = 0;

   mws_report_gfx_errs();

   if (irdt)
   {
      glBindFramebuffer(GL_FRAMEBUFFER, irdt->framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, irdt->color_att->get_texture_gl_id(), 0);
      // attach a renderbuffer to depth attachment point
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, irdt->depth_buffer_id);
      width = irdt->color_att->get_width();
      height = irdt->color_att->get_height();

#ifdef MWS_REPORT_GL_ERRORS

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
         trx("glerror - gl frame buffer status != frame buffer complete");
      }

      mws_report_gfx_errs();

#endif // MWS_REPORT_GL_ERRORS
   }
   else
   {
      if (gi()->active_rt)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);
         gi()->active_rt->color_att->texture_updated = true;
      }

      width = get_screen_width();
      height = get_screen_height();
      mws_report_gfx_errs();
   }

   gi()->active_rt = irdt;
   glViewport(0, 0, width, height);
   mws_report_gfx_errs();
}

bool gfx::ic_shader::reload_shader_on_modify()
{
   return true;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::new_program_from_src
(
   const std::string& iprg_name, std::shared_ptr<std::string> ivs_shader_src, std::shared_ptr<std::string> ifs_shader_src, std::shared_ptr<gfx_shader_listener> ilistener
)
{
   std::shared_ptr<gfx_shader> prg = get_program_by_name(iprg_name);

   if (!prg)
   {
      prg = gfx_shader::new_inst_inline(iprg_name, ivs_shader_src, ifs_shader_src, ilistener, gi());
      gi()->shader_list.push_back(prg);
   }

   return prg;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::new_program(const std::string& ishader_name, std::shared_ptr<gfx_shader_listener> ilistener)
{
   std::string shader_id = gfx_shader::create_shader_id(ishader_name, ishader_name);

   return gi()->shader.new_program(shader_id, ishader_name, ilistener);
}

std::shared_ptr<gfx_shader> gfx::ic_shader::new_program(const std::string& iprg_name, const std::string& ishader_name, std::shared_ptr<gfx_shader_listener> ilistener)
{
   std::string shader_id = gfx_shader::create_shader_id(ishader_name, ishader_name);
   std::shared_ptr<gfx_shader> prg = get_program_by_shader_id(shader_id);

   if (!prg)
   {
      prg = gfx_shader::new_inst(iprg_name, ishader_name, ilistener, gi());
      gi()->shader_list.push_back(prg);
   }

   return prg;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::new_program
(
   const std::string& iprg_name, const std::string& ivertex_shader, const std::string& ifragment_shader, std::shared_ptr<gfx_shader_listener> ilistener
)
{
   std::string shader_id = gfx_shader::create_shader_id(ivertex_shader, ifragment_shader);
   std::shared_ptr<gfx_shader> prg = get_program_by_shader_id(shader_id);

   if (!prg)
   {
      prg = gfx_shader::new_inst(iprg_name, ivertex_shader, ifragment_shader, ilistener, gi());
      gi()->shader_list.push_back(prg);
   }

   return prg;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::get_program_by_shader_id(std::string ishader_id)
{
   std::shared_ptr<gfx_shader> glp;

   for (auto it = gi()->shader_list.begin(); it != gi()->shader_list.end(); it++)
   {
      std::shared_ptr<gfx_shader> prg = it->lock();

      if (prg->get_shader_id() == ishader_id)
      {
         glp = prg;
         break;
      }
   }

   return glp;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::get_program_by_name(std::string iprg_name)
{
   std::shared_ptr<gfx_shader> glp;

   for (auto it = gi()->shader_list.begin(); it != gi()->shader_list.end(); it++)
   {
      std::shared_ptr<gfx_shader> prg = it->lock();

      if (prg->get_program_name() == iprg_name)
      {
         glp = prg;
         break;
      }
   }

   return glp;
}

std::shared_ptr<gfx_shader> gfx::ic_shader::get_current_program()
{
   return gi()->active_shader;
}

void gfx::ic_shader::set_current_program(std::shared_ptr<gfx_shader> iglp, bool force)
{
   mws_report_gfx_errs();

   if (iglp)
   {
      bool change = !gi()->active_shader || (gi()->active_shader->get_program_id() != iglp->get_program_id()) || force;

      if (change)
      {
         if (iglp->make_current())
         {
            gi()->active_shader = iglp;
         }
         else
         {
            gi()->active_shader = get_program_by_name("black-shader");
         }
      }
   }
   else
   {
      mws_signal_error("gl_ctrl::set_current_program - trying to set an invalid program");
   }

   mws_report_gfx_errs();
}

std::shared_ptr<gfx_tex> gfx::ic_tex::new_tex_2d(std::string iuni_tex_name, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      mws_throw ia_exception("texture name already exists");
   }

   tex = std::shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, i_prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

std::shared_ptr<gfx_tex> gfx::ic_tex::new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      mws_throw ia_exception("texture name already exists");
   }

   tex = std::shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, iwith, iheight, gfx_tex::TEX_2D, i_prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex> gfx::ic_tex::new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, std::string iformat, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      mws_throw ia_exception("texture name already exists");
   }

   tex_info* ti = tex_info_ht[iformat];
   gfx_tex_params prm;

   if (i_prm)
   {
      prm = *i_prm;
   }

   prm.internal_format = ti->internal_format;
   prm.format = ti->format;
   prm.type = ti->type;
   tex = std::shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, iwith, iheight, gfx_tex::TEX_2D, &prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

std::shared_ptr<gfx_tex> gfx::ic_tex::new_external_tex_2d(std::string iuni_tex_name, int itexture_id, int iwith, int iheight, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      mws_throw ia_exception("texture name already exists");
   }

   tex = std::shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, itexture_id, iwith, iheight, gfx_tex::TEX_2D, i_prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex_cube_map> gfx::ic_tex::get_tex_cube_map(std::string itex_name, bool iforce_new_inst)
{
   shared_ptr<gfx_tex> tex = get_texture_by_name(itex_name);
   shared_ptr<gfx_tex_cube_map> tex_cube_map;
   bool new_inst = false;

   if (tex)
   {
      if (iforce_new_inst || (tex->get_tex_type() != gfx_tex::TEX_CUBE_MAP))
      {
         new_inst = true;
      }
      else
      {
         tex_cube_map = static_pointer_cast<gfx_tex_cube_map>(tex);
      }
   }
   else
   {
      new_inst = true;
   }

   if (new_inst)
   {
      tex_cube_map = shared_ptr<gfx_tex_cube_map>(new gfx_tex_cube_map(itex_name));
      gi()->tex_list.push_back(tex_cube_map);
   }

   return tex_cube_map;
}

shared_ptr<gfx_tex_cube_map> gfx::ic_tex::new_tex_cube_map(uint32 isize)
{
   shared_ptr<gfx_tex_cube_map> tex_cube_map = shared_ptr<gfx_tex_cube_map>(new gfx_tex_cube_map(isize));

   gi()->tex_list.push_back(tex_cube_map);

   return tex_cube_map;
}

std::shared_ptr<gfx_tex> gfx::ic_tex::get_texture_by_name(std::string iname)
{
   std::shared_ptr<gfx_tex> tex;

   for (auto it = gi()->tex_list.begin(); it != gi()->tex_list.end(); it++)
   {
      std::shared_ptr<gfx_tex> t = it->lock();

      if (t->get_name() == iname)
      {
         tex = t;
         break;
      }
   }

   return tex;
}

void gfx::init(mws_sp<gfx> i_new_inst)
{
   rt.g = i_new_inst;
   shader.g = i_new_inst;
   tex.g = i_new_inst;

   gfx_state_inst = shared_ptr<gfx_state>(new gfx_state());
   rt.set_current_render_target(nullptr);

   // black shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
         uniform mat4 u_m4_model_view_proj;
      attribute vec3 a_v3_position;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )));

      auto fsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
#ifdef GL_ES\n
         precision lowp float; \n
#endif\n
         void main()
      {
         gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
      }
      )));

      black_shader = shader.new_program_from_src("black-shader", vsh, fsh);
      //vprint("fsh %s\n", fsh.c_str());
   }

   // wireframe shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
         uniform mat4 u_m4_model_view_proj;
      attribute vec3 a_v3_position;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
         gl_Position.z -= 0.001;
      }
      )));

      auto fsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
#ifdef GL_ES\n
         precision lowp float; \n
#endif\n
         void main()
      {
         gl_FragColor = vec4(0.5, 0.0, 1.0, 1.0);
      }
      )));

      wireframe_shader = shader.new_program_from_src("wireframe-shader", vsh, fsh);
   }

   // basic-tex shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
         attribute vec3 a_v3_position;
      attribute vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      varying vec2 v_v2_tex_coord;

      void main()
      {
         v_v2_tex_coord = a_v2_tex_coord;

         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )));

      auto fsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
#ifdef GL_ES\n
         precision lowp float; \n
#endif\n

         varying vec2 v_v2_tex_coord;
      uniform sampler2D u_s2d_tex;

      void main()
      {
         gl_FragColor = texture2D(u_s2d_tex, v_v2_tex_coord);
      }
      )));

      basic_tex_shader = shader.new_program_from_src("basic-tex-shader", vsh, fsh);
   }

   // c-o shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
         attribute vec3 a_v3_position;

      uniform mat4 u_m4_model_view_proj;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )));

      auto fsh = shared_ptr<std::string>(new std::string(GLSL_SRC(
#ifdef GL_ES\n
         precision lowp float; \n
#endif\n

         uniform vec4 u_v4_color;

      void main()
      {
         gl_FragColor = u_v4_color;
      }
      )));

      c_o_shader = shader.new_program_from_src("c-o-shader", vsh, fsh);
   }
}

void gfx::get_render_target_pixels_impl(shared_ptr<gfx_rt> irt, void* ivect)
{
   if (irt && rt.get_current_render_target())
   {
      glReadBuffer(GL_COLOR_ATTACHMENT0);
   }
   else
   {
      glReadBuffer(GL_NONE);
   }

   if (rt.get_current_render_target())
   {
      shared_ptr<gfx_tex> att = rt.get_current_render_target()->color_att;
      auto& prm = att->get_params();

      glReadPixels(0, 0, rt.get_render_target_width(), rt.get_render_target_height(), prm.format, prm.type, ivect);
   }
   else
   {
      glReadPixels(0, 0, rt.get_render_target_width(), rt.get_render_target_height(), GL_RGBA, GL_UNSIGNED_BYTE, ivect);
   }
}

void gfx::remove_gfx_obj(const gfx_obj* iobj)
{
   switch (iobj->get_type())
   {
   case gfx_obj::e_rt:
   {
      struct pred
      {
         bool operator()(std::weak_ptr<gfx_rt> wp) { return wp.expired(); }
      };
      auto it = std::find_if(rt_list.begin(), rt_list.end(), pred());
      rt_list.erase(it);
      break;
   }

   case gfx_obj::e_shader:
   {
      struct pred
      {
         bool operator()(std::weak_ptr<gfx_shader> wp) { return wp.expired(); }
      };
      auto it = std::find_if(shader_list.begin(), shader_list.end(), pred());
      shader_list.erase(it);
      break;
   }

   case gfx_obj::e_tex:
   {
      struct pred
      {
         bool operator()(std::weak_ptr<gfx_tex> wp) { return wp.expired(); }
      };
      auto it = std::find_if(tex_list.begin(), tex_list.end(), pred());
      tex_list.erase(it);
      break;
   }
   }
}

gfx::gfx()
{
}
