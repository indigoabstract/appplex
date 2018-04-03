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
shared_ptr<gfx_shader> gfx::active_shader;
shared_ptr<gfx_rt> gfx::active_rt;
shared_ptr<gfx_state> gfx::gfx_state_inst;
std::vector<weak_ptr<gfx_rt> > gfx::rt_list;
std::vector<weak_ptr<gfx_shader> > gfx::shader_list;
std::vector<weak_ptr<gfx_tex> > gfx::tex_list;
shared_ptr<gfx> gfx::inst;

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

bool gfx::is_init()
{
   return inst != shared_ptr<gfx>();
}

void gfx::init()
{
   if (!is_init())
   {
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

      vprint("gl-version [%d.%d] / [%s]\ngl-renderer [%s]\ngl-vendor [%s]\n", gl_major_version, gl_minor_version, version, renderer, vendor);

#ifdef USES_GL_STRINGI

      if (gl_extension_count > 0)
      {
         std::vector<const gfx_ubyte*> extensions;

         vprint("[%d] gl-extensions found.\n", gl_extension_count);

         if (print_extensions)
         {
            extensions.resize(gl_extension_count);

            for (int k = 0; k < gl_extension_count; k++)
            {
               extensions[k] = glGetStringi(GL_EXTENSIONS, k);
               vprint("%s\n", extensions[k]);
            }
         }

         vprint("\n");
      }

#else

      if (print_extensions)
      {
         const GLubyte* extensions = glGetString(GL_EXTENSIONS);
         vprint("gl-extensions: %s\n", extensions);
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

         vprint("red_bits [%d] green_bits [%d] blue_bits [%d] alpha_bits [%d] depth_bits [%d] stencil_bits [%d]\n",
            red_bits, green_bits, blue_bits, alpha_bits, depth_bits, stencil_bits);
      }

      // samples
      {
         GLint samples = 0, max_samples = 0;
         glGetIntegerv(GL_SAMPLES, &samples);
         glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
         vprint("sample coverage: %d, max samples: %d\n", samples, max_samples);
      }

      inst = shared_ptr<gfx>(new gfx());

      for (int k = 0; k < tex_info_tab_length; k++)
      {
         tex_info& e = tex_info_tab[k];
         tex_info_ht[e.id] = &e;
      }

      //int ms = 8;
      //GLuint mTextureFramebuffer = 0;
      //glGenRenderbuffers(1, &mTextureFramebuffer);
      //glBindRenderbuffer(GL_RENDERBUFFER, mTextureFramebuffer);
      //glRenderbufferStorageMultisample(GL_RENDERBUFFER, /* Number of samples */ ms, GL_R8, rt::get_screen_width(), rt::get_screen_height());
      //GLenum err = glGetError();

      //if (err != GL_NO_ERROR)
      //{
      //   bool MultisampleSupported = false;
      //   vprint("MultisampleSupported %d false\n", ms);
      //}
      //else
      //{
      //   vprint("MultisampleSupported %d true\n", ms);
      //}

      gfx_state_inst = shared_ptr<gfx_state>(new gfx_state());
      gfx_util::init();
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer_id);
      gfx_shader::init();
      gfx_material::init();
      rt::set_current_render_target(shared_ptr<gfx_rt>());
   }
   else
   {
      vprint("gl_ctrl::init: this method must only be called once\n");
      //reload();
      glGetIntegerv(GL_FRAMEBUFFER_BINDING, &default_framebuffer_id);
      //throw ia_exception("this method must only be called once");
   }
}

shared_ptr<gfx_state> gfx::get_gfx_state()
{
   return gfx_state_inst;
}

void gfx::reload()
{
   if (inst)
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
}

shared_ptr<gfx_rt> gfx::rt::new_rt()
{
   check_init();
   shared_ptr<gfx_rt> rt = shared_ptr<gfx_rt>(new gfx_rt());
   rt_list.push_back(rt);

   return rt;
}

int gfx::rt::get_screen_width()
{
   return pfm::screen::get_width();
}

int gfx::rt::get_screen_height()
{
   return pfm::screen::get_height();
}

int gfx::rt::get_render_target_width()
{
   return (active_rt) ? active_rt->get_width() : get_screen_width();
}

int gfx::rt::get_render_target_height()
{
   return (active_rt) ? active_rt->get_height() : get_screen_height();
}

shared_ptr<gfx_rt> gfx::rt::get_current_render_target()
{
   return active_rt;
}

void gfx::rt::set_current_render_target(shared_ptr<gfx_rt> irdt)
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

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
         trx("glerror - gl frame buffer status != frame buffer complete");
      }

      mws_report_gfx_errs();
   }
   else
   {
      if (active_rt)
      {
         glBindFramebuffer(GL_FRAMEBUFFER, default_framebuffer_id);
         active_rt->color_att->texture_updated = true;
      }

      width = get_screen_width();
      height = get_screen_height();
      mws_report_gfx_errs();
   }

   active_rt = irdt;
   glViewport(0, 0, width, height);
   mws_report_gfx_errs();
}

bool gfx::shader::reload_shader_on_modify()
{
   return true;
}

shared_ptr<gfx_shader> gfx::shader::new_program_from_src
(
   const std::string& iprg_name, shared_ptr<std::string> ivs_shader_src, shared_ptr<std::string> ifs_shader_src, shared_ptr<gfx_shader_listener> ilistener
)
{
   check_init();
   shared_ptr<gfx_shader> prg = get_program_by_name(iprg_name);

   if (!prg)
   {
      prg = gfx_shader::new_inst_inline(iprg_name, ivs_shader_src, ifs_shader_src, ilistener);
      shader_list.push_back(prg);
   }

   return prg;
}

shared_ptr<gfx_shader> gfx::shader::new_program(const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener)
{
   std::string shader_id = gfx_shader::create_shader_id(ishader_name, ishader_name);

   return gfx::shader::new_program(shader_id, ishader_name, ilistener);
}

shared_ptr<gfx_shader> gfx::shader::new_program(const std::string& iprg_name, const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener)
{
   check_init();
   std::string shader_id = gfx_shader::create_shader_id(ishader_name, ishader_name);
   shared_ptr<gfx_shader> prg = get_program_by_shader_id(shader_id);

   if (!prg)
   {
      prg = gfx_shader::new_inst(iprg_name, ishader_name, ilistener);
      shader_list.push_back(prg);
   }

   return prg;
}

shared_ptr<gfx_shader> gfx::shader::new_program
(
   const std::string& iprg_name, const std::string& ivertex_shader, const std::string& ifragment_shader, shared_ptr<gfx_shader_listener> ilistener
)
{
   check_init();
   std::string shader_id = gfx_shader::create_shader_id(ivertex_shader, ifragment_shader);
   shared_ptr<gfx_shader> prg = get_program_by_shader_id(shader_id);

   if (!prg)
   {
      prg = gfx_shader::new_inst(iprg_name, ivertex_shader, ifragment_shader, ilistener);
      shader_list.push_back(prg);
   }

   return prg;
}

shared_ptr<gfx_shader> gfx::shader::get_program_by_shader_id(std::string ishader_id)
{
   shared_ptr<gfx_shader> glp;

   check_init();

   for (auto it = shader_list.begin(); it != shader_list.end(); it++)
   {
      shared_ptr<gfx_shader> prg = it->lock();

      if (prg->get_shader_id() == ishader_id)
      {
         glp = prg;
         break;
      }
   }

   return glp;
}

shared_ptr<gfx_shader> gfx::shader::get_program_by_name(std::string iprg_name)
{
   shared_ptr<gfx_shader> glp;

   check_init();

   for (auto it = shader_list.begin(); it != shader_list.end(); it++)
   {
      shared_ptr<gfx_shader> prg = it->lock();

      if (prg->get_program_name() == iprg_name)
      {
         glp = prg;
         break;
      }
   }

   return glp;
}

shared_ptr<gfx_shader> gfx::shader::get_current_program()
{
   return active_shader;
}

void gfx::shader::set_current_program(shared_ptr<gfx_shader> iglp)
{
   mws_report_gfx_errs();

   if (iglp)
   {
      bool change = !active_shader || (active_shader->get_program_id() != iglp->get_program_id());

      if (change)
      {
         if (iglp->make_current())
         {
            active_shader = iglp;
         }
         else
         {
            active_shader = get_program_by_name("black_shader");
         }
      }
   }
   else
   {
      trx("gl_ctrl::set_current_program - trying to set an invalid program");
      ia_signal_error();
   }

   mws_report_gfx_errs();
}

shared_ptr<gfx_tex> gfx::tex::new_tex_2d(std::string iuni_tex_name, const gfx_tex_params* i_prm)
{
   check_init();
   shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      throw ia_exception("texture name already exists");
   }

   tex = shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, i_prm));
   tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex> gfx::tex::new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, const gfx_tex_params* i_prm)
{
   check_init();
   shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      throw ia_exception("texture name already exists");
   }

   tex = shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, iwith, iheight, gfx_tex::TEX_2D, i_prm));
   tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex> gfx::tex::new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, std::string iformat, const gfx_tex_params* i_prm)
{
   check_init();
   shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      throw ia_exception("texture name already exists");
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
   tex = shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, iwith, iheight, gfx_tex::TEX_2D, &prm));
   tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex> gfx::tex::new_external_tex_2d(std::string iuni_tex_name, int itexture_id, int iwith, int iheight, const gfx_tex_params* i_prm)
{
   check_init();
   shared_ptr<gfx_tex> tex = get_texture_by_name(iuni_tex_name);

   if (tex)
   {
      throw ia_exception("texture name already exists");
   }

   tex = shared_ptr<gfx_tex>(new gfx_tex(iuni_tex_name, itexture_id, iwith, iheight, gfx_tex::TEX_2D, i_prm));
   tex_list.push_back(tex);

   return tex;
}

shared_ptr<gfx_tex_cube_map> gfx::tex::get_tex_cube_map(std::string itex_name, bool iforce_new_inst)
{
   check_init();
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
      tex_list.push_back(tex_cube_map);
   }

   return tex_cube_map;
}

shared_ptr<gfx_tex_cube_map> gfx::tex::new_tex_cube_map(uint32 isize)
{
   check_init();
   shared_ptr<gfx_tex_cube_map> tex_cube_map = shared_ptr<gfx_tex_cube_map>(new gfx_tex_cube_map(isize));

   tex_list.push_back(tex_cube_map);

   return tex_cube_map;
}

shared_ptr<gfx_tex> gfx::tex::get_texture_by_name(std::string iname)
{
   shared_ptr<gfx_tex> tex;

   check_init();

   for (auto it = tex_list.begin(); it != tex_list.end(); it++)
   {
      shared_ptr<gfx_tex> t = it->lock();

      if (t->get_name() == iname)
      {
         tex = t;
         break;
      }
   }

   return tex;
}

void gfx::check_init()
{
   if (!inst)
   {
      throw ia_exception("gl context is not yet created!");
   }

   mws_report_gfx_errs();
}

void gfx::get_render_target_pixels_impl(shared_ptr<gfx_rt> irt, void* ivect)
{
   if (irt && rt::get_current_render_target())
   {
      glReadBuffer(GL_COLOR_ATTACHMENT0);
   }
   else
   {
      glReadBuffer(GL_NONE);
   }

   if (rt::get_current_render_target())
   {
      shared_ptr<gfx_tex> att = rt::get_current_render_target()->color_att;
      auto& prm = att->get_params();

      glReadPixels(0, 0, rt::get_render_target_width(), rt::get_render_target_height(), prm.format, prm.type, ivect);
   }
   else
   {
      glReadPixels(0, 0, rt::get_render_target_width(), rt::get_render_target_height(), GL_RGBA, GL_UNSIGNED_BYTE, ivect);
   }
}

void gfx::remove_gfx_obj(const gfx_obj* iobj)
{
    switch (iobj->get_type())
    {
    case gfx_obj::e_gfx_rt:
    {
        struct pred
        {
            bool operator()(std::weak_ptr<gfx_rt> wp) { return wp.expired(); }
        };
        auto it = std::find_if(rt_list.begin(), rt_list.end(), pred());
        rt_list.erase(it);
        break;
    }

    case gfx_obj::e_gfx_shader:
    {
        struct pred
        {
            bool operator()(std::weak_ptr<gfx_shader> wp) { return wp.expired(); }
        };
        auto it = std::find_if(shader_list.begin(), shader_list.end(), pred());
        shader_list.erase(it);
        break;
    }

    case gfx_obj::e_gfx_tex:
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
