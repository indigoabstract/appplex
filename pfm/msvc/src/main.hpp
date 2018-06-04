#pragma once

#include "pfm.hpp"

#if defined PLATFORM_WINDOWS_PC

#define _UNICODE
#define UNICODE
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "pfm.hpp"
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
   static shared_ptr<msvc_main> get_instance();
   static shared_ptr<pfm_impl::pfm_file_impl> new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
   virtual void init() override;
   virtual void start() override;
   virtual void run() override;
   virtual int get_screen_dpi()const;
   virtual void write_text(const char* text)const;
   virtual void write_text_nl(const char* text)const;
   virtual void write_text(const wchar_t* text)const;
   virtual void write_text_nl(const wchar_t* text)const;
   virtual void write_text_v(const char* iformat, ...)const;
   virtual std::string get_writable_path()const;
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
   float get_screen_scale();
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

   static shared_ptr<msvc_main> instance;
   bool is_full_screen;
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
