#pragma once

#include "pfm-def.h"

#if defined PLATFORM_WINDOWS_PC

#define _UNICODE
#define UNICODE
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "pfm.hxx"
#include <windows.h>
#include <shellapi.h>


enum lnk_subsystem
{
   subsys_console,
   subsys_windows,
};


class msvc_main : public pfm_main
{
public:
   virtual ~msvc_main();
   static mws_sp<msvc_main> get_instance();
   static mws_sp<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
   virtual void init() override;
   virtual void start() override;
   virtual void run() override;
   virtual key_types translate_key(int i_pfm_key_id) const override;
   virtual key_types apply_key_modifiers_impl(key_types i_key_id) const override;
   // screen metrix
   virtual std::pair<uint32, uint32> get_screen_res_px() const override;
   virtual float get_avg_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dim_inch() const override;
   virtual float get_avg_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dim_cm() const override;
   virtual void flip_screen() override;
   virtual void write_text(const char* text)const override;
   virtual void write_text_nl(const char* text)const override;
   virtual void write_text(const wchar_t* text)const override;
   virtual void write_text_nl(const wchar_t* text)const override;
   virtual void write_text_v(const char* iformat, ...)const override;
   virtual std::string get_writable_path()const override;
   virtual std::string get_timezone_id()const override;
   umf_list get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive);
   bool init_app(int argc, char** argv);
   int main_loop();
   void set_params(HINSTANCE ihinstance, bool incmd_show, lnk_subsystem isubsys);
   HWND get_hwnd();
   HMENU get_hmenu();
   void set_hmenu(HMENU ihmenu);
   UINT get_taskbar_created_msg();
   void minimize_window();
   void restore_window();
   virtual float get_screen_scale() const override;
   bool is_full_screen_mode();
   void set_full_screen_mode(bool ienabled);
   RECT get_window_coord();

private:
   msvc_main();

   void init_notify_icon_data();
   int console_main_loop();
   int win_main_loop();
   bool create_open_gl_context();
   bool create_open_gl_es_ctx();
   bool create_open_gl_glew_ctx();
   void destroy_open_gl_context();

   static mws_sp<msvc_main> instance;
   bool is_full_screen;
   bool is_window_flipped = false;
   uint32 portrait_flip_count = 0;
   bool emulate_mobile_screen = false;
   RECT window_coord;
   bool disable_paint;
   lnk_subsystem subsys;
   bool app_has_window;
   HINSTANCE hinstance;
   bool ncmd_show;
   HWND hwnd;
   HMENU hmenu;
   NOTIFYICONDATA notify_icon_data;
   UINT wm_taskbarcreated;
   HDC hdc_window;
   HANDLE console_handle;
   // screen metrix
   std::pair<uint32, uint32> screen_res;
   float avg_screen_dpi = 0.f;
   std::pair<float, float> screen_dpi;
   std::pair<float, float> screen_dim_inch;
   float avg_screen_dpcm = 0.f;
   std::pair<float, float> screen_dpcm;
   std::pair<float, float> screen_dim_cm;

#if defined USES_OPENGL_ES
   EGLDisplay egl_display;
   EGLConfig egl_config;
   EGLSurface egl_surface;
   EGLContext egl_context;
   NativeWindowType egl_window;
   EGLint pi32_config_attribs[128];
#else
   HGLRC hgl_rendering_context;
#endif
};

#endif
