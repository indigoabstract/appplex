#pragma once

#include "pfm-def.h"
#include <string>
#include <vector>
#include <unordered_map>

class mws_cm;
class mws_in;
class mws_pt;
class mws_px;
class gfx_obj;
class gfx_rt;
class gfx_shader_listener;
class gfx_shader;
class gfx_tex_params;
class gfx_tex;
class gfx_tex_cube_map;
class gfx_state;
class mws_mod_ctrl;


// screen metrix
class mws_dim
{
public:
   // specifies whether to take the horizontal, vertical or average dpi when converting a physical dimension to pixels
   enum dpi_types
   {
      e_average,
      e_horizontal,
      e_vertical,
   };

   virtual float val() const { return size; }
   virtual mws_cm to_cm() const = 0;
   virtual mws_in to_in() const = 0;
   virtual mws_pt to_pt() const = 0;
   virtual mws_px to_px(dpi_types i_dpi_type = e_average) const = 0;

protected:
   mws_dim() {}

   float size = 0.f;
};


class mws_cm : public mws_dim
{
public:
   mws_cm(float i_size = 0.f) { size = i_size; }
   virtual mws_cm to_cm() const override;
   virtual mws_in to_in() const override;
   virtual mws_pt to_pt() const override;
   virtual mws_px to_px(dpi_types i_dpi_type = e_average) const override;
};


class mws_in : public mws_dim
{
public:
   mws_in(float i_size = 0.f) { size = i_size; }
   virtual mws_cm to_cm() const override;
   virtual mws_in to_in() const override;
   virtual mws_pt to_pt() const override;
   virtual mws_px to_px(dpi_types i_dpi_type = e_average) const override;
};


class mws_pt : public mws_dim
{
public:
   mws_pt(float i_size = 0.f) { size = i_size; }
   virtual mws_cm to_cm() const override;
   virtual mws_in to_in() const override;
   virtual mws_pt to_pt() const override;
   virtual mws_px to_px(dpi_types i_dpi_type = e_average) const override;
};


class mws_px : public mws_dim
{
public:
   mws_px(float i_size = 0.f, dpi_types i_dpi_type = e_average) : dpi_type(i_dpi_type) { size = i_size; }
   uint32_t int_val() const;
   virtual mws_cm to_cm() const override;
   virtual mws_in to_in() const override;
   virtual mws_pt to_pt() const override;
   virtual mws_px to_px(dpi_types i_dpi_type = e_average) const override;

   static float get_px_per_cm(dpi_types i_dpi_type = e_average);
   static float get_px_per_in(dpi_types i_dpi_type = e_average);

protected:
   dpi_types dpi_type = e_average;
};


class gfx
{
public:
   static const inline std::string black_sh_id = "mws-black-shader";
   static const inline std::string wireframe_sh_id = "mws-wireframe-shader";
   static const inline std::string basic_tex_sh_id = "mws-basic-tex-shader";
   static const inline std::string sh_id_texture_transparency = "mws-texture-transparency-shader";
   static const inline std::string c_o_sh_id = "mws-c-o-shader";
   static const inline std::string mws_sh_id = "mws-mws-shader";
   // hashtable with names for standard shaders
   static const inline std::vector<std::string> std_shader_list =
   {
      black_sh_id, wireframe_sh_id, basic_tex_sh_id, sh_id_texture_transparency, c_o_sh_id, mws_sh_id
   };

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

      template<typename T> mws_sp<std::vector<T>> get_render_target_pixels(mws_sp<gfx_rt> i_rt = nullptr)
      {
         mws_sp<std::vector<T> > vect(new std::vector<T>(get_render_target_width() * get_render_target_height()));

         get_render_target_pixels<T>(i_rt, vect);

         return vect;
      }

      template<typename T> void get_render_target_pixels(mws_sp<gfx_rt> i_rt, mws_sp<std::vector<T>> i_vect)
      {
         g.lock()->get_render_target_pixels_impl(i_rt, i_vect->data());
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
         const std::string& i_prg_name, const std::string& i_vs_shader_src, const std::string& i_fs_shader_src,
         mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> new_program(const std::string& i_shader_name, mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false);
      mws_sp<gfx_shader> new_program
      (
         const std::string& i_prg_name, const std::string& i_shader_name, mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> new_program
      (
         const std::string& i_prg_name, const std::string& ivertex_shader, const std::string& ifragment_shader,
         mws_sp<gfx_shader_listener> i_listener = nullptr, bool i_suppress_nex_msg = false
      );
      mws_sp<gfx_shader> get_program_by_shader_id(std::string ishader_id);
      mws_sp<gfx_shader> get_program_by_name(std::string i_prg_name);
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
      mws_sp<gfx_tex_cube_map> new_tex_cube_map(uint32_t i_size);
      mws_sp<gfx_tex> get_texture_by_name(std::string i_filename);
      // returns an existing texture by that name, or creates an new one if it doesn't exist yet
      mws_sp<gfx_tex> load_tex(std::string i_name);

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
   struct shader_src { mws_sp<std::string> vsh; mws_sp<std::string> fsh; };

   static void global_init();
   static void on_destroy();
   static void on_resize(int i_width, int i_height);
   static shader_src get_std_shader_src(const std::string& i_shader_name);
   void init(mws_sp<gfx> i_new_inst);
   void get_render_target_pixels_impl(mws_sp<gfx_rt> i_rt, void* i_vect);
   void remove_gfx_obj(const gfx_obj* iobj);
   gfx();

   static mws_sp<gfx> main_instance;
   static gfx_int default_framebuffer_id;
   mws_sp<gfx_shader> active_shader;
   mws_sp<gfx_rt> active_rt;
   mws_sp<gfx_state> gfx_state_inst;
   std::unordered_map<std::string, mws_sp<gfx_shader>> name_shader_ht;
   std::vector<mws_wp<gfx_rt> > rt_list;
   std::vector<mws_wp<gfx_shader> > shader_list;
   std::vector<mws_wp<gfx_tex> > tex_list;
};
