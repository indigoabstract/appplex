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


class gfx
{
public:
   static bool is_init();
   static void init();
   static shared_ptr<gfx_state> get_gfx_state();
   static void reload();

   struct rt
   {
      static shared_ptr<gfx_rt> new_rt();
      static int get_screen_width();
      static int get_screen_height();
      static int get_render_target_width();
      static int get_render_target_height();
      static shared_ptr<gfx_rt> get_current_render_target();
      static void set_current_render_target(shared_ptr<gfx_rt> irdt = nullptr);

      template<typename T> static shared_ptr<std::vector<T> > get_render_target_pixels(shared_ptr<gfx_rt> irt = nullptr)
      {
         std::shared_ptr<std::vector<T> > vect(new std::vector<T>(get_render_target_width() * get_render_target_height()));

         get_render_target_pixels<T>(irt, *vect);

         return vect;
      }

      template<typename T> static void get_render_target_pixels(shared_ptr<gfx_rt> irt, std::vector<T>& i_vect)
      {
         get_render_target_pixels_impl(irt, begin_ptr(i_vect));
      }
   };

   struct shader
   {
      static bool reload_shader_on_modify();
      static shared_ptr<gfx_shader> new_program_from_src
      (
         const std::string& iprg_name, shared_ptr<std::string> ivs_shader_src, shared_ptr<std::string> ifs_shader_src,
         shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      static shared_ptr<gfx_shader> new_program(const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener = nullptr);
      static shared_ptr<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ishader_name, shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      static shared_ptr<gfx_shader> new_program
      (
         const std::string& iprg_name, const std::string& ivertex_shader, const std::string& ifragment_shader,
         shared_ptr<gfx_shader_listener> ilistener = nullptr
      );
      static shared_ptr<gfx_shader> get_program_by_shader_id(std::string ishader_id);
      static shared_ptr<gfx_shader> get_program_by_name(std::string iprg_name);
      static shared_ptr<gfx_shader> get_current_program();
      static void set_current_program(shared_ptr<gfx_shader> iglp);
   };

   struct tex
   {
      static shared_ptr<gfx_tex> new_tex_2d(std::string iuni_tex_name, const gfx_tex_params* i_prm = nullptr);
      static shared_ptr<gfx_tex> new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, const gfx_tex_params* i_prm = nullptr);
      static shared_ptr<gfx_tex> new_tex_2d(std::string iuni_tex_name, int iwith, int iheight, std::string iformat, const gfx_tex_params* i_prm = nullptr);
      static shared_ptr<gfx_tex> new_external_tex_2d(std::string iuni_tex_name, int itexture_id, int iwith, int iheight, const gfx_tex_params* i_prm = nullptr);

      /**
      expects to find in the resources 6 same size images named like this:
      [itex_name-posx.png, itex_name-negx.png, itex_name-posy.png, itex_name-negy.png, itex_name-posz.png, itex_name-negz.png]
      */
      static shared_ptr<gfx_tex_cube_map> get_tex_cube_map(std::string itex_name, bool inew_inst = false);
      static shared_ptr<gfx_tex_cube_map> new_tex_cube_map(uint32 isize);
      static shared_ptr<gfx_tex> get_texture_by_name(std::string iname);
   };

private:
   friend class gfx_rt;
   friend class gfx_shader;
   friend class gfx_tex;

   static void check_init();
   static void get_render_target_pixels_impl(shared_ptr<gfx_rt> irt, void* ivect);
   static void remove_gfx_obj(const gfx_obj* iobj);
   gfx();

   static gfx_int default_framebuffer_id;
   static shared_ptr<gfx_shader> active_shader;
   static shared_ptr<gfx_rt> active_rt;
   static shared_ptr<gfx_state> gfx_state_inst;
   static int screen_width;
   static int screen_height;
   static std::vector<weak_ptr<gfx_rt> > rt_list;
   static std::vector<weak_ptr<gfx_shader> > shader_list;
   static std::vector<weak_ptr<gfx_tex> > tex_list;
   static shared_ptr<gfx> inst;
};
