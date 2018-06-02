#pragma once

#include <string>
#include <vector>
#include "pfm.hpp"

class gfx_obj;
class gfx_rt;
class gfx_shader_listener;
class gfx_shader;
class gfx_tex_params;
class gfx_tex;
class gfx_tex_cube_map;
class gfx_state;
class unit_ctrl;


class gfx
{
public:
   static std::shared_ptr<gfx> nwi();
   static mws_sp<gfx> i() { return main_instance; }
   shared_ptr<gfx_state> get_gfx_state();
   void reload();

   struct ic_rt
   {
      mws_sp<gfx> gi() { return g.lock(); }
      std::shared_ptr<gfx_rt> new_rt();
      int get_screen_width();
      int get_screen_height();
      int get_render_target_width();
      int get_render_target_height();
      std::shared_ptr<gfx_rt> get_current_render_target();
       // i_rdt - new framebuffer to bind, null for default
       // i_force_binding - force a rebinding to the default FB, even if it was already bound. useful for resync of opengl state with outside libs
      void set_current_render_target(std::shared_ptr<gfx_rt> i_rdt = nullptr, bool i_force_binding = false);

      template<typename T> shared_ptr<std::vector<T> > get_render_target_pixels(shared_ptr<gfx_rt> irt = nullptr)
      {
         std::shared_ptr<std::vector<T> > vect(new std::vector<T>(get_render_target_width() * get_render_target_height()));

         get_render_target_pixels<T>(irt, vect);

         return vect;
      }

      template<typename T> void get_render_target_pixels(std::shared_ptr<gfx_rt> irt, std::shared_ptr<std::vector<T> > ivect)
      {
         g.lock()->get_render_target_pixels_impl(irt, begin_ptr(ivect));
      }

   private:
      friend class gfx;
      std::weak_ptr<gfx> g;
   };
   ic_rt rt;

   struct ic_shader
   {
      mws_sp<gfx> gi() { return g.lock(); }
      bool reload_shader_on_modify();
      std::shared_ptr<gfx_shader> new_program_from_src
      (
         const std::string& iprg_name, std::shared_ptr<std::string> ivs_shader_src, std::shared_ptr<std::string> ifs_shader_src,
         std::shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      std::shared_ptr<gfx_shader> new_program(const std::string& ishader_name, std::shared_ptr<gfx_shader_listener> ilistener = nullptr);
      std::shared_ptr<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ishader_name, std::shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      std::shared_ptr<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ivertex_shader, const std::string& ifragment_shader,
         std::shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      std::shared_ptr<gfx_shader> get_program_by_shader_id(std::string ishader_id);
      std::shared_ptr<gfx_shader> get_program_by_name(std::string iprg_name);
      std::shared_ptr<gfx_shader> get_current_program();
      void set_current_program(std::shared_ptr<gfx_shader> iglp, bool force = false);

   private:
      friend class gfx;
      std::weak_ptr<gfx> g;
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
      shared_ptr<gfx_tex_cube_map> get_tex_cube_map(std::string itex_name, bool inew_inst = false);
      shared_ptr<gfx_tex_cube_map> new_tex_cube_map(uint32 isize);
      shared_ptr<gfx_tex> get_texture_by_name(std::string i_filename);
      // returns an existing texture by that name, or creates an new one if it doesn't exist yet
      shared_ptr<gfx_tex> load_tex(std::string iname);

   private:
      friend class gfx;
      std::weak_ptr<gfx> g;
   };
   ic_tex tex;

private:
   friend class gfx_rt;
   friend class gfx_shader;
   friend class gfx_tex;
   friend class unit_ctrl;

   static void global_init();
   void init(mws_sp<gfx> i_new_inst);
   void get_render_target_pixels_impl(shared_ptr<gfx_rt> irt, void* ivect);
   void remove_gfx_obj(const gfx_obj* iobj);
   gfx();

   static mws_sp<gfx> main_instance;
   static gfx_int default_framebuffer_id;
   std::shared_ptr<gfx_shader> active_shader;
   std::shared_ptr<gfx_rt> active_rt;
   std::shared_ptr<gfx_state> gfx_state_inst;
   std::shared_ptr<gfx_shader> black_shader;
   std::shared_ptr<gfx_shader> wireframe_shader;
   std::shared_ptr<gfx_shader> basic_tex_shader;
   std::shared_ptr<gfx_shader> mws_shader;
   std::shared_ptr<gfx_shader> c_o_shader;
   std::vector<std::weak_ptr<gfx_rt> > rt_list;
   std::vector<std::weak_ptr<gfx_shader> > shader_list;
   std::vector<std::weak_ptr<gfx_tex> > tex_list;
};
