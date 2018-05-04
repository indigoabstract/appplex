#include "stdafx.h"

#include "main.hpp"

#if defined PLATFORM_WINDOWS_PC

#include "pfm.hpp"
#include "pfm-gl.h"
#include "unit.hpp"
#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <tchar.h>
#include <wchar.h>
#include <WindowsX.h>

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON						(WM_USER + 1)



// app entry points
int APIENTRY _tWinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, LPTSTR lpcmd_line, int ncmd_show)
// gui subsystem entry point
{
   UNREFERENCED_PARAMETER(hprev_instance);
   UNREFERENCED_PARAMETER(lpcmd_line);

   shared_ptr<msvc_main> app = msvc_main::get_instance();

   app->set_params(hinstance, ncmd_show, subsys_windows);

   if (app->init_app(0, 0))
   {
      return app->main_loop();
   }

   return -1;
}


int main(int argc, char** argv)
// console subsystem entry point
{
   shared_ptr<msvc_main> app = msvc_main::get_instance();
   HINSTANCE hinstance = GetModuleHandle(NULL);
   bool nCmdShow = true;

   app->set_params(hinstance, nCmdShow, subsys_console);

   if (app->init_app(argc, argv))
   {
      return app->main_loop();
   }

   return -1;
}



class msvc_file_impl : public pfm_impl::pfm_file_impl
{
public:
   msvc_file_impl(const std::string& ifilename, const std::string& iroot_dir) : pfm_impl::pfm_file_impl(ifilename, iroot_dir)
   {
   }

   virtual uint64 length()
   {
      std::string path = ppath.get_full_path();
      WIN32_FILE_ATTRIBUTE_DATA file_info;

      if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &file_info))
      {
         ULARGE_INTEGER size;
         size.HighPart = file_info.nFileSizeHigh;
         size.LowPart = file_info.nFileSizeLow;
         //mws_print("%s %llu\n", filename.c_str(), size.QuadPart);

         return size.QuadPart;
      }

      return 0;
   }

   virtual uint64 creation_time()const
   {
      std::string path = ppath.get_full_path();
      WIN32_FILE_ATTRIBUTE_DATA file_info;

      if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &file_info))
      {
         ULARGE_INTEGER time;
         time.HighPart = file_info.ftCreationTime.dwHighDateTime;
         time.LowPart = file_info.ftCreationTime.dwLowDateTime;
         //mws_print("%s %llu\n", filename.c_str(), time.QuadPart);

         return time.QuadPart;
      }

      return 0;
   }

   virtual uint64 last_write_time()const
   {
      std::string path = ppath.get_full_path();
      WIN32_FILE_ATTRIBUTE_DATA file_info;

      if (GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &file_info))
      {
         ULARGE_INTEGER time;
         time.HighPart = file_info.ftLastWriteTime.dwHighDateTime;
         time.LowPart = file_info.ftLastWriteTime.dwLowDateTime;
         //mws_print("%s %llu\n", filename.c_str(), time.QuadPart);

         return time.QuadPart;
      }

      return 0;
   }
};


// helper functions
ATOM				register_new_window_class(HINSTANCE hinstance);
HWND				create_app_window(HINSTANCE, RECT& iclient_rect);
LRESULT CALLBACK	wnd_proc(HWND, UINT, WPARAM, LPARAM);
int					get_key(int key);


shared_ptr<msvc_main> msvc_main::instance;


msvc_main::msvc_main()
{
   is_full_screen = false;
   disable_paint = false;
   subsys = subsys_console;
   app_has_window = false;
   hinstance = 0;
   ncmd_show = false;
   hwnd = 0;
   hdc_window = 0;
   console_handle = INVALID_HANDLE_VALUE;

#if defined MWS_USES_OPENGL_ES
   egl_display = 0;
   egl_config = 0;
   egl_surface = 0;
   egl_context = 0;
   egl_window = 0;
#else
   hgl_rendering_context = 0;
#endif
}

msvc_main::~msvc_main()
{
}

shared_ptr<msvc_main> msvc_main::get_instance()
{
   if (!instance)
   {
      instance = shared_ptr<msvc_main>(new msvc_main());
   }

   return instance;
}

shared_ptr<pfm_impl::pfm_file_impl> msvc_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
   return std::make_shared<msvc_file_impl>(ifilename, iroot_dir);
}

void msvc_main::init()
{
   pfm_main::init();

   unit_ctrl::inst()->init_app();
}

void msvc_main::start()
{
   pfm_main::start();

   unit_ctrl::inst()->start_app();
}

void msvc_main::run()
{
   unit_ctrl::inst()->update();
}

int msvc_main::get_screen_dpi()const
{
   return 127;
}

void msvc_main::write_text(const char* text)const
{
   if (text && instance->console_handle != INVALID_HANDLE_VALUE)
   {
      //WriteConsoleA(instance.consoleHandle, text, wcslen(text), NULL, NULL);
      printf(text);
   }

   OutputDebugStringA(text);
}

void msvc_main::write_text_nl(const char* text)const
{
   write_text(text);
   write_text("\n");
}

void msvc_main::write_text(const wchar_t* text)const
{
   if (text && instance->console_handle != INVALID_HANDLE_VALUE)
   {
      //WriteConsoleW(instance.consoleHandle, text, wcslen(text), NULL, NULL);
      wprintf(text);
   }

   OutputDebugStringW(text);
}

void msvc_main::write_text_v(const char* iformat, ...)const
{
   static char dest[1024 * 16];
   va_list argptr;

   va_start(argptr, iformat);
   vsnprintf_s(dest, 1024 * 16 - 1, _TRUNCATE, iformat, argptr);
   va_end(argptr);

   if (iformat && instance->console_handle != INVALID_HANDLE_VALUE)
   {
      //WriteConsoleA(instance.consoleHandle, text, strlen(text), NULL, NULL);
      printf(dest);
   }

   OutputDebugStringA(dest);
}

std::string msvc_main::get_writable_path()const
{
   return "";
}

void msvc_main::write_text_nl(const wchar_t* text)const
{
   write_text(text);
   write_text(L"\n");
}

umf_list msvc_main::get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive)
{
   if (idirectory.length() > 0)
   {
      std::string dir = idirectory;
      WIN32_FIND_DATAA fd;
      HANDLE hfind = 0;

      if (dir[dir.length() - 1] != '/')
      {
         dir += "/";
      }

      std::string search_path = dir + "*.*";
      hfind = ::FindFirstFileA(search_path.c_str(), &fd);

      if (hfind != INVALID_HANDLE_VALUE)
      {
         do
         {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
               // regular file
            {
               umf_r& list = *iplist;
               std::string key(fd.cFileName);

               if (list[key])
               {
                  mws_signal_error(std::string("duplicate filename: " + key).c_str());
               }

               shared_ptr<msvc_file_impl> file_impl(new msvc_file_impl(key, dir));

               list[key] = pfm_file::get_inst(file_impl);
            }
            else if (fd.cFileName[0] != '.' && is_recursive)
               // directory
            {
               std::string sub_dir = dir + fd.cFileName;
               get_directory_listing(sub_dir, iplist, true);
            }
         } while (::FindNextFileA(hfind, &fd));

         ::FindClose(hfind);
      }
   }

   return iplist;
}

bool msvc_main::init_app(int argc, char** argv)
{
   // pass arguments
   int wargc = 0;
   LPWSTR* arg_list = CommandLineToArgvW(GetCommandLineW(), &wargc);
   pfm::params::set_app_arguments(wargc, arg_list, true);
   LocalFree(arg_list);

   unit_ctrl::inst()->pre_init_app();

   app_has_window = unit_ctrl::inst()->app_uses_gfx();
   unit_ctrl::inst()->set_gfx_available(app_has_window);

   if (app_has_window)
   {
      // I want to be notified when windows explorer
      // crashes and re-launches the taskbar.  the WM_TASKBARCREATED
      // event will be sent to my WndProc() AUTOMATICALLY whenever
      // explorer.exe starts up and fires up the taskbar again.
      // So its great, because now, even if explorer crashes,
      // I have a way to re-add my system tray icon in case
      // the app is already in the "minimized" (hidden) state.
      // if we did not do this an explorer crashed, the application
      // would remain inaccessible!!
      wm_taskbarcreated = RegisterWindowMessageA("TaskbarCreated");

      // create and show window
      register_new_window_class(hinstance);

      auto u = unit_ctrl::inst()->get_app_start_unit();
      //int x = CW_USEDEFAULT;
      int x = 0;
      int y = 0;
      int width = pfm::screen::get_width();
      int height = pfm::screen::get_height();
      bool start_full_screen = false;

      if (u)
      {
         auto unit_pref = u->get_preferences();
         int pref_width = unit_pref->get_preferred_screen_width();
         int pref_height = unit_pref->get_preferred_screen_height();
         start_full_screen = unit_pref->start_full_screen();

         if (pref_width > 0 && pref_height > 0)
         {
            width = pref_width;
            height = pref_height;
         }
      }

#if defined MWS_DEBUG_BUILD

      x = GetSystemMetrics(SM_CXSCREEN) - width - 5;
      y = GetSystemMetrics(SM_CYSCREEN) - height - 5;

#endif

      window_coord.left = x;
      window_coord.right = x + width;
      window_coord.top = y;
      window_coord.bottom = y + height;
      AdjustWindowRect(&window_coord, WS_OVERLAPPEDWINDOW, FALSE);

      hwnd = create_app_window(hinstance, window_coord);

      if (hwnd == NULL)
      {
         return false;
      }

      hdc_window = GetDC(hwnd);

      if (!create_open_gl_context())
      {
         return false;
      }

      if (start_full_screen)
      {
         set_full_screen_mode(true);
      }

      init_notify_icon_data();

      ShowWindow(hwnd, ncmd_show);
      UpdateWindow(hwnd);
   }

   // find console status
   bool consoleExists = (subsys == subsys_console) ? true : AttachConsole(ATTACH_PARENT_PROCESS);

   if (consoleExists)
   {
      console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

      // redirect unbuffered STDOUT to the console
      int hConHandle = _open_osfhandle((long)console_handle, _O_TEXT);
      FILE* fp = _fdopen(hConHandle, "w");
      *stdout = *fp;
      setvbuf(stdout, NULL, _IONBF, 0);
      std::ios_base::sync_with_stdio();
   }

   init();

   return true;
}

int msvc_main::main_loop()
{
   start();

   if (app_has_window)
   {
      return win_main_loop();
   }

   return console_main_loop();
}

void msvc_main::set_params(HINSTANCE ihinstance, bool incmd_show, lnk_subsystem isubsys)
{
   hinstance = ihinstance;
   ncmd_show = incmd_show;
   subsys = isubsys;
}

HWND msvc_main::get_hwnd()
{
   return hwnd;
}

HMENU msvc_main::get_hmenu()
{
   return hmenu;
}

void msvc_main::set_hmenu(HMENU ihmenu)
{
   hmenu = ihmenu;
}

UINT msvc_main::get_taskbar_created_msg()
{
   return wm_taskbarcreated;
}

void msvc_main::minimize_window()
{
   // add the icon to the system tray
   Shell_NotifyIcon(NIM_ADD, &notify_icon_data);
   ShowWindow(hwnd, SW_HIDE);
}

void msvc_main::restore_window()
{
   // remove the icon from the system tray
   Shell_NotifyIcon(NIM_DELETE, &notify_icon_data);
   ShowWindow(hwnd, SW_SHOW);
}

bool msvc_main::is_full_screen_mode()
{
   return is_full_screen;
}

void msvc_main::set_full_screen_mode(bool ienabled)
{
   if (is_full_screen != ienabled)
   {
      long style = ienabled ? WS_POPUP : WS_OVERLAPPEDWINDOW;

      if (!is_full_screen)
      {
         window_coord = get_window_coord();
      }

      SetWindowLong(hwnd, GWL_STYLE, style);

      if (ienabled)
      {
         DEVMODE mode_info;
         EnumDisplaySettings(0, ENUM_CURRENT_SETTINGS, &mode_info);

         SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, mode_info.dmPelsWidth, mode_info.dmPelsHeight, SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_NOREDRAW);
      }
      else
      {
         int x = window_coord.left;
         int y = window_coord.top;
         int width = window_coord.right - window_coord.left;
         int height = window_coord.bottom - window_coord.top;

         SetWindowPos(hwnd, HWND_NOTOPMOST, x, y, width, height, SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_NOREDRAW);
      }

      is_full_screen = ienabled;
      disable_paint = true;
   }
}

RECT msvc_main::get_window_coord()
{
   RECT client_area_coord = {};

   GetWindowRect(hwnd, &client_area_coord);

   return client_area_coord;
}

void msvc_main::init_notify_icon_data()
// initialize the NOTIFYICONDATA structure.
// see MSDN docs http://msdn.microsoft.com/en-us/library/bb773352(VS.85).aspx for details on the NOTIFYICONDATA structure.
{
   memset(&notify_icon_data, 0, sizeof(NOTIFYICONDATA));

   // the combination of HWND and uID form a unique identifier for each item in the system tray
   // windows knows which application each icon in the system tray belongs to by the HWND parameter.
   notify_icon_data.cbSize = sizeof(NOTIFYICONDATA);
   notify_icon_data.hWnd = hwnd;
   notify_icon_data.uID = ID_TRAY_APP_ICON;
   notify_icon_data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
   // this message must be handled in HWND's window procedure
   notify_icon_data.uCallbackMessage = WM_TRAYICON;
   notify_icon_data.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   // set the tooltip text. must be less than 64 chars
   wcscpy(notify_icon_data.szTip, TEXT("appplex"));

   // balloon
   notify_icon_data.uTimeout = 5000;
   notify_icon_data.uFlags = notify_icon_data.uFlags | NIF_INFO;
   notify_icon_data.dwInfoFlags = NIIF_INFO;
   wcscpy(notify_icon_data.szInfoTitle, TEXT("TITLE"));
   wcscpy(notify_icon_data.szInfo, TEXT("SOME TEXT"));
}

int msvc_main::console_main_loop()
{
   const int timerInterval = 1000 / pfm::screen::get_target_fps();
   long curTime = pfm::time::get_time_millis();
   long nextUpdateTime = curTime;

   while (!unit_ctrl::inst()->is_set_app_exit_on_next_run())
   {
      curTime = pfm::time::get_time_millis();

      if (curTime >= nextUpdateTime)
      {
         run();
         nextUpdateTime = curTime + timerInterval;
      }
      else
      {
         Sleep(15);
      }
   }

   unit_ctrl::inst()->destroy_app();

   return 0;
}

int msvc_main::win_main_loop()
{
   const int timer_interval = 1000 / pfm::screen::get_target_fps();
   uint32 current_time = pfm::time::get_time_millis();
   uint32 next_update_time = current_time;
   MSG msg;

   while (!unit_ctrl::inst()->is_set_app_exit_on_next_run())
   {
      if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
      {
         if (msg.message == WM_QUIT)// || msg.message == WM_CLOSE)
         {
            break;
         }
         else
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
         }
      }

      current_time = pfm::time::get_time_millis();

      if (current_time >= next_update_time)
      {
         run();
         {
            if (!disable_paint)
            {
#if defined MWS_USES_OPENGL_ES
               eglSwapBuffers(egl_display, egl_surface);
#else
               SwapBuffers(hdc_window);
#endif
            }
            else
            {
               disable_paint = false;
            }
         }

         next_update_time = current_time + timer_interval;
      }
      else
      {
         Sleep(15);
      }
   }

   if (!IsWindowVisible(hwnd))
      // remove the tray icon
   {
      Shell_NotifyIcon(NIM_DELETE, &notify_icon_data);
   }

   unit_ctrl::inst()->destroy_app();
   destroy_open_gl_context();
   ReleaseDC(hwnd, hdc_window);
   DestroyWindow(hwnd);

   return msg.wParam;
}

bool msvc_main::create_open_gl_context()
{
#if defined MWS_USES_OPENGL_ES
   return create_open_gl_es_ctx();
#elif defined MWS_USES_OPENGL_GLEW
   return create_open_gl_glew_ctx();
#endif

   return false;
}

bool msvc_main::create_open_gl_es_ctx()
{
#if defined MWS_USES_OPENGL_ES

   class test_egl
   {
   public:
      static bool test_egl_error(HWND hWnd, char* pszLocation)
      {
         /*
         eglGetError returns the last error that has happened using egl,
         not the status of the last called function. The user has to
         check after every single egl call or at least once every frame.
         */
         EGLint iErr = eglGetError();

         if (iErr != EGL_SUCCESS)
         {
            TCHAR pszStr[256];
            _stprintf(pszStr, _T("%s failed (%d).\n"), pszLocation, iErr);
            MessageBox(hWnd, pszStr, _T("Error"), MB_OK | MB_ICONEXCLAMATION);

            return false;
         }

         return true;
      }
   };

   egl_window = hwnd;
   /*
   Step 1 - Get the default display.
   EGL uses the concept of a "display" which in most environments
   corresponds to a single physical screen. Since we usually want
   to draw to the main screen or only have a single screen to begin
   with, we let EGL pick the default display.
   Querying other displays is platform specific.
   */
   egl_display = eglGetDisplay((NativeDisplayType)hdc_window);

   if (egl_display == EGL_NO_DISPLAY)
   {
      egl_display = eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
   }
   /*
   Step 2 - Initialize EGL.
   EGL has to be initialized with the display obtained in the
   previous step. We cannot use other EGL functions except
   eglGetDisplay and eglGetError before eglInitialize has been
   called.
   If we're not interested in the EGL version number we can just
   pass NULL for the second and third parameters.
   */
   EGLint iMajorVersion, iMinorVersion;

   if (!eglInitialize(egl_display, &iMajorVersion, &iMinorVersion))
   {
      MessageBox(0, _T("eglInitialize() failed."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);

      return false;
   }

   /*
   Step 3 - Specify the required configuration attributes.
   An EGL "configuration" describes the pixel format and type of
   surfaces that can be used for drawing.
   For now we just want to use a 16 bit RGB surface that is a
   Window surface, i.e. it will be visible on screen. The list
   has to contain key/value pairs, terminated with EGL_NONE.
   */
   {
      int i = 0;
      pi32_config_attribs[i++] = EGL_RED_SIZE;
      pi32_config_attribs[i++] = 5;
      pi32_config_attribs[i++] = EGL_GREEN_SIZE;
      pi32_config_attribs[i++] = 6;
      pi32_config_attribs[i++] = EGL_BLUE_SIZE;
      pi32_config_attribs[i++] = 5;
      pi32_config_attribs[i++] = EGL_ALPHA_SIZE;
      pi32_config_attribs[i++] = 0;
      pi32_config_attribs[i++] = EGL_SURFACE_TYPE;
      pi32_config_attribs[i++] = EGL_WINDOW_BIT;
      pi32_config_attribs[i++] = EGL_NONE;
   }

   /*
   Step 4 - Find a config that matches all requirements.
   eglChooseConfig provides a list of all available configurations
   that meet or exceed the requirements given as the second
   argument. In most cases we just want the first config that meets
   all criteria, so we can limit the number of configs returned to 1.
   */
   EGLint iConfigs;

   if (!eglChooseConfig(egl_display, pi32_config_attribs, &egl_config, 1, &iConfigs) || (iConfigs != 1))
   {
      MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);

      return false;
   }

   /*
   Step 5 - Create a surface to draw to.
   Use the config picked in the previous step and the native window
   handle when available to create a window surface. A window surface
   is one that will be visible on screen inside the native display (or
   fullscreen if there is no windowing system).
   Pixmaps and pbuffers are surfaces which only exist in off-screen
   memory.
   */
   egl_surface = eglCreateWindowSurface(egl_display, egl_config, egl_window, NULL);

   if (egl_surface == EGL_NO_SURFACE)
   {
      eglGetError(); // Clear error
      egl_surface = eglCreateWindowSurface(egl_display, egl_config, NULL, NULL);
   }

   if (!test_egl::test_egl_error(hwnd, "eglCreateWindowSurface"))
   {
      return false;
   }

   /*
   Step 6 - Create a context.
   EGL has to create a context for OpenGL ES. Our OpenGL ES resources
   like textures will only be valid inside this context
   (or shared contexts)
   */
   egl_context = eglCreateContext(egl_display, egl_config, NULL, NULL);

   if (!test_egl::test_egl_error(hwnd, "eglCreateContext"))
   {
      return false;
   }

   /*
   Step 7 - Bind the context to the current thread and use our
   window surface for drawing and reading.
   Contexts are bound to a thread. This means you don't have to
   worry about other threads and processes interfering with your
   OpenGL ES application.
   We need to specify a surface that will be the target of all
   subsequent drawing operations, and one that will be the source
   of read operations. They can be the same surface.
   */
   eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

   if (!test_egl::test_egl_error(hwnd, "eglMakeCurrent"))
   {
      return false;
   }

   return true;
#endif

   return false;
}

bool msvc_main::create_open_gl_glew_ctx()
{
#if defined MWS_USES_OPENGL_GLEW

   // set pixel format
   PIXELFORMATDESCRIPTOR pfd = { 0 };

   pfd.nSize = sizeof(pfd);
   pfd.nVersion = 1;
   pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 24;
   pfd.cRedBits = 8;
   pfd.cGreenBits = 8;
   pfd.cBlueBits = 8;
   pfd.cAlphaBits = 8;
   pfd.cDepthBits = 24;
   pfd.cStencilBits = 8;
   pfd.iLayerType = PFD_MAIN_PLANE;

   int pixelFormat = ChoosePixelFormat(hdc_window, &pfd);
   mws_assert(pixelFormat > 0);
   SetPixelFormat(hdc_window, pixelFormat, &pfd);

   // create rendering context.
   hgl_rendering_context = wglCreateContext(hdc_window);
   wglMakeCurrent(hdc_window, hgl_rendering_context);

   bool ENABLE_MULTISAMPLING = true;

   if (ENABLE_MULTISAMPLING)
   {
      int pixelAttribs[] =
      {
         WGL_SAMPLES_ARB, 16,
         WGL_SAMPLE_BUFFERS_ARB, 1,
         WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
         WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
         WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
         WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
         WGL_COLOR_BITS_ARB, 24,
         WGL_RED_BITS_ARB, 8,
         WGL_GREEN_BITS_ARB, 8,
         WGL_BLUE_BITS_ARB, 8,
         WGL_ALPHA_BITS_ARB, 8,
         WGL_DEPTH_BITS_ARB, 24,
         WGL_STENCIL_BITS_ARB, 8,
         WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
         0,
      };
      int& sampleCount = pixelAttribs[1];
      int& sampleBufferCount = pixelAttribs[3];
      int pixelFormat = -1;
      PROC proc = wglGetProcAddress("wglChoosePixelFormatARB");
      unsigned int numFormats;
      PFNWGLCHOOSEPIXELFORMATARBPROC twglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)proc;

      if (!twglChoosePixelFormatARB)
      {
         mws_print("Could not load function pointer for 'wglChoosePixelFormatARB'.  Is your driver properly installed?");
         return false;
      }


      // try_ fewer and fewer samples per pixel till we find one that is supported:
      while (pixelFormat <= 0 && sampleCount >= 0)
      {
         twglChoosePixelFormatARB(hdc_window, pixelAttribs, 0, 1, &pixelFormat, &numFormats);
         sampleCount--;

         if (sampleCount <= 1)
         {
            sampleBufferCount = 0;
         }
      }

      // win32 allows the pixel format to be set only once per window, so destroy and re-create the app window
      DestroyWindow(hwnd);
      hwnd = create_app_window(hinstance, window_coord);
      hdc_window = GetDC(hwnd);
      SetPixelFormat(hdc_window, pixelFormat, &pfd);
      destroy_open_gl_context();
      hgl_rendering_context = wglCreateContext(hdc_window);
      wglMakeCurrent(hdc_window, hgl_rendering_context);

      if (sampleBufferCount > 0)
      {
         mws_print("MSAA samples per pixel count: {}", sampleCount);
      }
   }

   GLenum init = glewInit();

   if (init != GLEW_OK)
   {
      return false;
   }

   if (GL_VERSION_4_2)
   {
      mws_print("GLEW_VERSION_4_2 supported");
   }

   //if (glewIsSupported("GL_VERSION_4_2"))
   //{
   //	const int contextAttribs[] =
   //	{
   //		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
   //		WGL_CONTEXT_MINOR_VERSION_ARB, 0,
   //		WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
   //		0
   //	};

   //	HGLRC new_rc = wglCreateContextAttribsARB(hdc_window, 0, contextAttribs);
   //	wglMakeCurrent(0, 0);
   //	wglDeleteContext(hgl_rendering_context);
   //	hgl_rendering_context = new_rc;
   //	wglMakeCurrent(hdc_window, hgl_rendering_context);
   //}

   return true;
#endif

   return false;
}

void msvc_main::destroy_open_gl_context()
{
#if defined MWS_USES_OPENGL_ES

   /*
   Step 9 - Terminate OpenGL ES and destroy the window (if present).
   eglTerminate takes care of destroying any context or surface created
   with this display, so we don't need to call eglDestroySurface or
   eglDestroyContext here.
   */
   eglMakeCurrent(egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(egl_display);
   egl_display = 0;
   egl_config = 0;
   egl_surface = 0;
   egl_context = 0;
   egl_window = 0;

#else

   wglMakeCurrent(NULL, NULL);
   wglDeleteContext(hgl_rendering_context);
   hgl_rendering_context = 0;

#endif
}


// helper functions
ATOM register_new_window_class(HINSTANCE hinstance)
// registers the window class
{
   WNDCLASSEX wcex;

   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = wnd_proc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hinstance;
   wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
   wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wcex.lpszMenuName = NULL;
   wcex.lpszClassName = unit_ctrl::inst()->get_app_name().c_str();

   return RegisterClassEx(&wcex);
}

HWND create_app_window(HINSTANCE hinstance, RECT& iclient_rect)
// creates the main window
{
   //	HWND hWnd;
   //	RECT clientRect = {0};
   //
   //	clientRect.right = 800;
   //	clientRect.bottom = 480;
   //	AdjustWindowRect(&clientRect, WS_OVERLAPPEDWINDOW, FALSE);
   //
   //	int width = clientRect.right - clientRect.left;
   //	int height = clientRect.bottom - clientRect.top;
   //	int x = CW_USEDEFAULT;
   //	int y = 0;
   //
   //#if defined MWS_DEBUG_BUILD
   //	x = 1920 - width - 5;
   //	y = 1080 - height - 5;
   //#endif
   //
   //	hWnd = CreateWindow(unitctrl::getAppName().c_str(), unitctrl::getAppName().c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hinstance, NULL);

   HWND hWnd;

   int x = iclient_rect.left;
   int y = iclient_rect.top;
   int width = iclient_rect.right - x;
   int height = iclient_rect.bottom - y;

   hWnd = CreateWindow(unit_ctrl::inst()->get_app_name().c_str(), unit_ctrl::inst()->get_app_name().c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hinstance, NULL);

   if (!hWnd)
   {
      return NULL;
   }

   return hWnd;
}

POINT get_pointer_coord(HWND hwnd)
{
   POINT touch_point;

   GetCursorPos(&touch_point);
   ScreenToClient(hwnd, &touch_point);

   return touch_point;
}

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
// processes messages for the main window
{
   shared_ptr<msvc_main> app = msvc_main::get_instance();

   //mws_print("message 0x%04x") % message;

   if (message == app->get_taskbar_created_msg() && !IsWindowVisible(app->get_hwnd()))
   {
      app->minimize_window();
      return 0;
   }

   switch (message)
   {
      //case WM_NCPAINT:
      //case WM_PAINT:
      //	return 0;

   case WM_ERASEBKGND:
      return TRUE;

   case WM_CREATE:
      // create the menu
      app->set_hmenu(CreatePopupMenu());
      AppendMenu(app->get_hmenu(), MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));
      break;

   case WM_SYSCOMMAND:
      switch (wparam & 0xfff0)
         // filter out reserved lower 4 bits. see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx
      {
      case SC_MINIMIZE:
      case SC_CLOSE:
         app->minimize_window();
         return 0;
      }
      break;

   case WM_TRAYICON:
      // user defined WM_TRAYICON message
   {
      printf("Tray icon notification, from %d\n", wparam);

      switch (wparam)
      {
      case ID_TRAY_APP_ICON:
         printf("Its the ID_TRAY_APP_ICON.. one app can have several tray icons, ya know..\n");
         break;
      }

      if (lparam == WM_LBUTTONUP)
      {
         printf("You have restored me!\n");
         app->restore_window();
      }
      else if (lparam == WM_RBUTTONDOWN)
      {
         POINT pointer_coord;

         printf("Mmm.  Let's get contextual.  I'm showing you my context menu.\n");
         GetCursorPos(&pointer_coord);
         //SetActiveWindow(hWnd);
         SetForegroundWindow(hwnd);

         // TrackPopupMenu blocks the app until it returns
         printf("calling track\n");
         UINT clicked = TrackPopupMenu(
            app->get_hmenu(),
            // don't send WM_COMMAND messages about this window, instead return the identifier of the clicked menu item
            TPM_RETURNCMD | TPM_NONOTIFY,
            pointer_coord.x,
            pointer_coord.y,
            0,
            hwnd,
            NULL
         );
         printf("returned from call to track\n");

         if (clicked == ID_TRAY_EXIT_CONTEXT_MENU_ITEM)
         {
            // quit the application.
            printf("I have posted the quit message, biatch\n");
            PostQuitMessage(0);
         }
      }
   }
   break;

   case WM_LBUTTONDOWN:
   {
      auto pfm_te = std::make_shared<pointer_evt>();
      POINT pointer_coord = get_pointer_coord(hwnd);
      pointer_evt::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = false;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = pfm::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = pointer_evt::touch_began;

      unit_ctrl::inst()->pointer_action(pfm_te);

      return 0;
   }

   case WM_LBUTTONUP:
   {
      auto pfm_te = std::make_shared<pointer_evt>();
      POINT pointer_coord = get_pointer_coord(hwnd);
      pointer_evt::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = false;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = pfm::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = pointer_evt::touch_ended;

      unit_ctrl::inst()->pointer_action(pfm_te);

      return 0;
   }

   case WM_MOUSEMOVE:
   {
      auto pfm_te = std::make_shared<pointer_evt>();
      POINT pointer_coord = get_pointer_coord(hwnd);
      pointer_evt::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = false;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = pfm::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = pointer_evt::touch_moved;

      unit_ctrl::inst()->pointer_action(pfm_te);

      return 0;
   }

   case WM_MOUSEWHEEL:
   {
      int state = GET_KEYSTATE_WPARAM(wparam);
      int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
      POINT pointer_coord;
      pointer_coord.x = GET_X_LPARAM(lparam);
      pointer_coord.y = GET_Y_LPARAM(lparam);

      ScreenToClient(hwnd, &pointer_coord);
      RECT client_area_coord = app->get_window_coord();
      int width = pfm::screen::get_width();
      int height = pfm::screen::get_height();

      if (pointer_coord.x >= 0 && pointer_coord.y >= 0 && pointer_coord.x < width && pointer_coord.y < height)
      {
         auto pfm_te = std::make_shared<pointer_evt>();
         pointer_evt::touch_point& te = pfm_te->points[0];

         te.identifier = 0;
         te.is_changed = false;
         te.x = (float)pointer_coord.x;
         te.y = (float)pointer_coord.y;
         pfm_te->time = pfm::time::get_time_millis();
         pfm_te->touch_count = 1;
         pfm_te->type = pointer_evt::mouse_wheel;
         pfm_te->mouse_wheel_delta = wheel_delta;

         unit_ctrl::inst()->pointer_action(pfm_te);
         //mws_print("mouse wheel %1% %2% %3%") % wheel_delta % pointer_coord.x % pointer_coord.y;
      }

      return 0;
   }

   case WM_KEYDOWN:
   case WM_SYSKEYDOWN:
   {
      unit_ctrl::inst()->key_action(KEY_PRESS, get_key(wparam));
      return 0;
   }

   case WM_KEYUP:
   case WM_SYSKEYUP:
   {
      unit_ctrl::inst()->key_action(KEY_RELEASE, get_key(wparam));

      if (get_key(wparam) == KEY_ESCAPE)
      {
         unit_ctrl::inst()->back_evt();
      }

      return 0;
   }

   case WM_CLOSE:
      printf("Got an actual WM_CLOSE Message!  Woo hoo!\n");
      app->minimize_window();
      return 0;

   case WM_SIZE:
   {
      int width = LOWORD(lparam);
      int height = HIWORD(lparam);

      height = (height > 0) ? height : 1;
      unit_ctrl::inst()->resize_app(width, height);

      return 0;
   }

   //case WM_DESTROY:
   //	PostQuitMessage(0);
   //	break;
   }

   return DefWindowProc(hwnd, message, wparam, lparam);
}

int get_key(int ikey)
{
   if (ikey >= 'A' && ikey <= 'Z')
   {
      int diff = ikey - 'A';
      return KEY_A + diff;
   }

   switch (ikey)
   {
   case VK_SHIFT:
      return KEY_SHIFT;

   case VK_CONTROL:
      return KEY_CONTROL;

   case VK_MENU:
      return KEY_ALT;

   case VK_UP:
      return KEY_UP;

   case VK_DOWN:
      return KEY_DOWN;

   case VK_LEFT:
      return KEY_LEFT;

   case VK_RIGHT:
      return KEY_RIGHT;

   case VK_RETURN:
      return KEY_SELECT;

   case VK_BACK:
      return KEY_BACK;

   case VK_ESCAPE:
      return KEY_ESCAPE;

   case VK_SPACE:
      return KEY_SPACE;

   case VK_F1:
      return KEY_F1;

   case VK_F2:
      return KEY_F2;

   case VK_F3:
      return KEY_F3;

   case VK_F4:
      return KEY_F4;

   case VK_F5:
      return KEY_F5;

   case VK_F6:
      return KEY_F6;

   case VK_F7:
      return KEY_F7;

   case VK_F8:
      return KEY_F8;

   case VK_F9:
      return KEY_F9;

   case VK_F10:
      return KEY_F10;

   case VK_F11:
      return KEY_F11;

   case VK_F12:
      return KEY_F12;
   }

   return KEY_INVALID;
}


int mws_is_gl_extension_supported(const char* i_extension)
{
   GLboolean is_supported = (glewIsSupported(i_extension) == 0) ? 0 : 1;

   return is_supported;
}

#endif
