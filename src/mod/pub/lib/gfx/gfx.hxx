#pragma once

#include <string>
#include <vector>
#include "pfm-def.h"

class gfx_obj;
class gfx_rt;
class gfx_shader_listener;
class gfx_shader;
class gfx_tex_params;
class gfx_tex;
class gfx_tex_cube_map;
class gfx_state;
class mws_mod_ctrl;


class gfx
{
public:
   static const inline std::string black_sh_id = "black-shader";
   static const inline std::string wireframe_sh_id = "wireframe-shader";
   static const inline std::string basic_tex_sh_id = "basic-tex-shader";
   static const inline std::string c_o_sh_id = "c-o-shader";
   static const inline std::string mws_sh_id = "mws-shader";

   static mws_sp<gfx> nwi();
   static mws_sp<gfx> i() { return main_instance; }
   mws_sp<gfx_state> get_gfx_state();
   void reload();

   struct ic_rt
   {
      mws_sp<gfx> gi() { return g.lock(); }
      mws_sp<gfx_rt> new_rt();
      int get_screen_width();
      int get_screen_height();
      int get_render_target_width();
      int get_render_target_height();
      mws_sp<gfx_rt> get_current_render_target();
       // i_rdt - new framebuffer to bind, null for default
       // i_force_binding - force a rebinding to the default FB, even if it was already bound. useful for resync of opengl state with outside libs
      void set_current_render_target(mws_sp<gfx_rt> i_rdt = nullptr, bool i_force_binding = false);

      template<typename T> mws_sp<std::vector<T> > get_render_target_pixels(mws_sp<gfx_rt> irt = nullptr)
      {
         mws_sp<std::vector<T> > vect(new std::vector<T>(get_render_target_width() * get_render_target_height()));

         get_render_target_pixels<T>(irt, vect);

         return vect;
      }

      template<typename T> void get_render_target_pixels(mws_sp<gfx_rt> irt, mws_sp<std::vector<T> > ivect)
      {
         g.lock()->get_render_target_pixels_impl(irt, begin_ptr(ivect));
      }

   private:
      friend class gfx;
      mws_wp<gfx> g;
   };
   ic_rt rt;

   struct ic_shader
   {
      mws_sp<gfx> gi() { return g.lock(); }
      bool reload_shader_on_modify();
      mws_sp<gfx_shader> new_program_from_src
      (
         const std::string& iprg_name, mws_sp<std::string> ivs_shader_src, mws_sp<std::string> ifs_shader_src,
         mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> new_program(const std::string& ishader_name, mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false);
      mws_sp<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ishader_name, mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ivertex_shader, const std::string& ifragment_shader,
         mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> get_program_by_shader_id(std::string ishader_id);
      mws_sp<gfx_shader> get_program_by_name(std::string iprg_name);
      // check for a shader named i_shader_root_name, return it if not null, otherwise,
      // check if i_shader_root_name.vsh and i_shader_root_name.fsh exist, then load them in a new shader, otherwise give up and return nullptr
      // useful for loading shaders from disk and/or code
      mws_sp<gfx_shader> nwi_inex_by_shader_root_name(const std::string& i_shader_root_name, bool i_suppress_nex_msg = false);
      mws_sp<gfx_shader> get_current_program();
      void set_current_program(mws_sp<gfx_shader> iglp, bool force = false);

   private:
      friend class gfx;
      mws_wp<gfx> g;
   };
   ic_shader shader;

   struct ic_tex
   {
      mws_sp<gfx> gi() { return g.lock(); }
      mws_sp<gfx_tex> nwi(std::string i_filename, const gfx_tex_params* i_prm = nullptr);
      mws_sp<gfx_tex> nwi(std::string i_tex_id, int i_width, int i_height, const gfx_tex_params* i_prm = nullptr);
      mws_sp<gfx_tex> nwi(std::string i_tex_id, int i_width, int i_height, std::string i_format_id);
      mws_sp<gfx_tex> nwi_external(std::string i_tex_id, int i_gl_id, std::string i_format_id);

      /**
      expects to find in the resources 6 same size images named like this:
      [itex_name-posx.png, itex_name-negx.png, itex_name-posy.png, itex_name-negy.png, itex_name-posz.png, itex_name-negz.png]
      */
      mws_sp<gfx_tex_cube_map> get_tex_cube_map(std::string itex_name, bool inew_inst = false);
      mws_sp<gfx_tex_cube_map> new_tex_cube_map(uint32 isize);
      mws_sp<gfx_tex> get_texture_by_name(std::string i_filename);
      // returns an existing texture by that name, or creates an new one if it doesn't exist yet
      mws_sp<gfx_tex> load_tex(std::string iname);

   private:
      friend class gfx;
      mws_wp<gfx> g;
   };
   ic_tex tex;

private:
   friend class gfx_rt;
   friend class gfx_shader;
   friend class gfx_tex;
   friend class mws_mod_ctrl;

   static void global_init();
   static void on_destroy();
   static void on_resize(int i_width, int i_height);
   void init(mws_sp<gfx> i_new_inst);
   void get_render_target_pixels_impl(mws_sp<gfx_rt> irt, void* ivect);
   void remove_gfx_obj(const gfx_obj* iobj);
   gfx();

   static mws_sp<gfx> main_instance;
   static gfx_int default_framebuffer_id;
   mws_sp<gfx_shader> active_shader;
   mws_sp<gfx_rt> active_rt;
   mws_sp<gfx_state> gfx_state_inst;
   mws_sp<gfx_shader> black_shader;
   mws_sp<gfx_shader> wireframe_shader;
   mws_sp<gfx_shader> basic_tex_shader;
   mws_sp<gfx_shader> mws_shader;
   mws_sp<gfx_shader> c_o_shader;
   std::vector<mws_wp<gfx_rt> > rt_list;
   std::vector<mws_wp<gfx_shader> > shader_list;
   std::vector<mws_wp<gfx_tex> > tex_list;
};
