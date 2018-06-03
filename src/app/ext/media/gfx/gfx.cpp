#include "stdafx.h"

#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-tex.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include "media/res-ld/res-ld.hpp"
#include "min.hpp"
#include "pfm-gl.h"
#include <glm/gtc/type_ptr.hpp>
#include <unordered_map>


glm::vec4 default_viewport_dim;
gfx_int gfx::default_framebuffer_id = 0;
mws_sp<gfx> gfx::main_instance;



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
      //glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer_id);
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

   {
      int framebuffer_id = -1;

      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer_id);
      default_framebuffer_id = framebuffer_id;
      glGetFloatv(GL_VIEWPORT, glm::value_ptr(default_viewport_dim));
   }

   main_instance = std::shared_ptr<gfx>(new gfx());
   main_instance->init(main_instance);
}

std::shared_ptr<gfx> gfx::nwi()
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

void gfx::ic_rt::set_current_render_target(std::shared_ptr<gfx_rt> irdt, bool i_force_binding)
{
   mws_report_gfx_errs();

   if (irdt)
   {
      int width = 0, height = 0;

      glBindFramebuffer(GL_FRAMEBUFFER, irdt->framebuffer);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, irdt->color_att->get_texture_gl_id(), 0);
      // attach a renderbuffer to depth attachment point
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, irdt->depth_buffer_id);
      width = irdt->color_att->get_width();
      height = irdt->color_att->get_height();
      glViewport(0, 0, width, height);

#ifdef MWS_REPORT_GL_ERRORS

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
         mws_print("glerror - gl frame buffer status != frame buffer complete");
      }

      mws_report_gfx_errs();

#endif // MWS_REPORT_GL_ERRORS
   }
   else
   {
      if (gi()->active_rt || i_force_binding)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);

         if (gi()->active_rt)
         {
            gi()->active_rt->color_att->texture_updated = true;
         }
      }

      glViewport(default_viewport_dim.x, default_viewport_dim.y, default_viewport_dim.z, default_viewport_dim.w);
      mws_report_gfx_errs();
   }

   gi()->active_rt = irdt;
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
      prg = gfx_shader::nwi(iprg_name, ishader_name, ilistener, gi());
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
      prg = gfx_shader::nwi(iprg_name, ivertex_shader, ifragment_shader, ilistener, gi());
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


mws_sp<gfx_tex> gfx::ic_tex::nwi(std::string i_filename, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(i_filename);
   mws_assert(!tex);

   tex = std::shared_ptr<gfx_tex>(new gfx_tex(i_filename, i_prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

mws_sp<gfx_tex> gfx::ic_tex::nwi(std::string i_tex_id, int i_width, int i_height, const gfx_tex_params* i_prm)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(i_tex_id);
   mws_assert(!tex);

   tex = std::shared_ptr<gfx_tex>(new gfx_tex(i_tex_id, i_width, i_height, gfx_tex::TEX_2D, i_prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

mws_sp<gfx_tex> gfx::ic_tex::nwi(std::string i_tex_id, int i_width, int i_height, std::string i_format_id)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(i_tex_id);
   mws_assert(!tex);

   gfx_tex_params prm;

   prm.set_format_id(i_format_id);
   tex = std::shared_ptr<gfx_tex>(new gfx_tex(i_tex_id, i_width, i_height, gfx_tex::TEX_2D, &prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

mws_sp<gfx_tex> gfx::ic_tex::nwi_external(std::string i_tex_id, int i_gl_id, std::string i_format_id)
{
   std::shared_ptr<gfx_tex> tex = get_texture_by_name(i_tex_id);
   mws_assert(!tex);

   gfx_tex_params prm;

   prm.set_format_id(i_format_id);
   tex = std::shared_ptr<gfx_tex>(new gfx_tex(i_tex_id, i_gl_id, 1, 1, gfx_tex::TEX_2D, &prm, gi()));
   gi()->tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex_cube_map> gfx::ic_tex::get_tex_cube_map(std::string itex_name, bool iforce_new_inst)
{
   shared_ptr<gfx_tex> tex = get_texture_by_name(itex_name);
   shared_ptr<gfx_tex_cube_map> tex_cube_map;
   bool new_tex = false;

   if (tex)
   {
      if (iforce_new_inst || (tex->get_tex_type() != gfx_tex::TEX_CUBE_MAP))
      {
         new_tex = true;
      }
      else
      {
         tex_cube_map = static_pointer_cast<gfx_tex_cube_map>(tex);
      }
   }
   else
   {
      new_tex = true;
   }

   if (new_tex)
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

shared_ptr<gfx_tex> gfx::ic_tex::load_tex(std::string i_filename)
{
   auto tex = get_texture_by_name(i_filename);

   if (!tex)
   {
      tex = res_ld::inst()->load_tex(i_filename);
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
      auto vsh = shared_ptr<std::string>(new std::string(
         R"(
      uniform mat4 u_m4_model_view_proj;
      attribute vec3 a_v3_position;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = shared_ptr<std::string>(new std::string(
         R"(
#ifdef GL_ES
         precision lowp float;
#endif
         void main()
      {
         gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
      }
      )"
      ));

      black_shader = shader.new_program_from_src("black-shader", vsh, fsh);
      //vprint("fsh %s\n", fsh.c_str());
   }

   // wireframe shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(
         R"(
      uniform mat4 u_m4_model_view_proj;
      attribute vec3 a_v3_position;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
         gl_Position.z -= 0.001;
      }
      )"
      ));

      auto fsh = shared_ptr<std::string>(new std::string(
         R"(
#ifdef GL_ES
         precision lowp float;
#endif
         void main()
      {
         gl_FragColor = vec4(0.5, 0.0, 1.0, 1.0);
      }
      )"
      ));

      wireframe_shader = shader.new_program_from_src("wireframe-shader", vsh, fsh);
   }

   // basic-tex shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(
         R"(
      attribute vec3 a_v3_position;
      attribute vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      varying vec2 v_v2_tex_coord;

      void main()
      {
         v_v2_tex_coord = a_v2_tex_coord;

         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = shared_ptr<std::string>(new std::string(
         R"(
#ifdef GL_ES
      precision lowp float;
#endif

      uniform sampler2D u_s2d_tex;

      varying vec2 v_v2_tex_coord;

      void main()
      {
         vec4 v4_color = texture2D(u_s2d_tex, v_v2_tex_coord);

         gl_FragColor = v4_color;
      }
      )"
      ));

      basic_tex_shader = shader.new_program_from_src("basic-tex-shader", vsh, fsh);
   }

   // c-o shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(
         R"(
      attribute vec3 a_v3_position;

      uniform mat4 u_m4_model_view_proj;

      void main()
      {
         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = shared_ptr<std::string>(new std::string(
         R"(
#ifdef GL_ES
         precision lowp float;
#endif

         uniform vec4 u_v4_color;

      void main()
      {
         gl_FragColor = u_v4_color;
      }
      )"
      ));

      c_o_shader = shader.new_program_from_src("c-o-shader", vsh, fsh);
   }

   // mws shader
   {
      auto vsh = shared_ptr<std::string>(new std::string(
         R"(
      attribute vec3 a_v3_position;
      attribute vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      varying vec2 v_v2_tex_coord;

      void main()
      {
         v_v2_tex_coord = a_v2_tex_coord;

         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = shared_ptr<std::string>(new std::string(
         R"(
#ifdef GL_ES
      precision lowp float;
#endif

      uniform float u_v1_is_enabled;
      uniform vec4 u_v4_color;
      uniform float u_v1_has_tex;
      uniform sampler2D u_s2d_tex;
      uniform float u_v1_has_alpha;

      varying vec2 v_v2_tex_coord;

      void main()
      {
         vec4 v4_color;

         if(u_v1_has_tex == 1.)
         {
            v4_color = texture2D(u_s2d_tex, v_v2_tex_coord);
         }
         else
         {
            v4_color = u_v4_color;
         }

         if(u_v1_is_enabled == 0.)
         {
            float v1_gray = dot(v4_color.rgb, vec3(0.299, 0.587, 0.114));
            v4_color.rgb = vec3(v1_gray);
         }

         gl_FragColor = v4_color;
      }
      )"
      ));

      mws_shader = shader.new_program_from_src("mws-shader", vsh, fsh);
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

      glPixelStorei(GL_PACK_ALIGNMENT, prm.get_bpp());
      glReadPixels(0, 0, rt.get_render_target_width(), rt.get_render_target_height(), prm.get_format(), prm.get_type(), ivect);
   }
   else
   {
      glPixelStorei(GL_PACK_ALIGNMENT, 4);
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
