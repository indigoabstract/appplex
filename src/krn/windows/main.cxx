#include "stdafx.hxx"

#include "pfm-def.h"

#if defined MWS_PFM_WINDOWS_PC

#define _UNICODE
#define UNICODE
#define NOMINMAX
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif

#include "pfm.hxx"
#include "mws-impl.hxx"
#include "pfm-gl.h"
#include "krn.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "input/input-ctrl.hxx"
#include "gfx.hxx"
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <fcntl.h>
#include <io.h>
#include <string>
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wchar.h>
#include <WindowsX.h>
#include <direct.h>
#include <filesystem>

#define ID_TRAY_APP_ICON                5000
#define ID_TRAY_EXIT_CONTEXT_MENU_ITEM  3000
#define WM_TRAYICON						(WM_USER + 1)


class msvc_main : public mws_app
{
public:
   msvc_main();
   virtual ~msvc_main();
   virtual mws_key_types translate_key(int i_pfm_key_id) const override;
   virtual mws_key_types apply_key_modifiers_impl(mws_key_types i_key_id) const override;
   // screen metrix
   virtual std::pair<uint32_t, uint32_t> get_screen_res_px() const override;
   virtual float get_avg_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dim_inch() const override;
   virtual float get_avg_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dim_cm() const override;
   virtual void flip_screen() const override;
   virtual bool is_full_screen_mode() const override;
   virtual void set_full_screen_mode(bool i_enabled) const override;
   // log
   virtual void write_text(const char* i_text) const override;
   virtual void write_text_nl(const char* i_text) const override;
   virtual void write_text(const wchar_t* i_text) const override;
   virtual void write_text_nl(const wchar_t* i_text) const override;
   virtual void write_text_v(const char* i_format, ...) const override;
   // filesystem
   virtual mws_sp<mws_file_impl> new_mws_file_impl(const mws_path& i_path, bool i_is_internal = false) const override;
   virtual mws_file_map list_internal_directory() const override;
   virtual std::vector<mws_sp<mws_file>> list_external_directory(const mws_path& i_directory, bool i_recursive) const override;
   virtual const mws_path& prv_dir() const override;
   virtual const mws_path& res_dir() const override;
   virtual const mws_path& tmp_dir() const override;
   virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) override;
   virtual std::string get_timezone_id() const override;
};


enum lnk_subsystem
{
   subsys_console,
   subsys_windows,
};


bool init_app(int argc, const char** argv);
int main_loop();
void set_params(HINSTANCE ihinstance, bool incmd_show, lnk_subsystem isubsys);
HWND get_hwnd();
HMENU get_hmenu();
void set_hmenu(HMENU ihmenu);
UINT get_taskbar_created_msg();
void minimize_window();
void restore_window();
RECT get_window_coord();
void init_notify_icon_data();
int console_main_loop();
int win_main_loop();
bool create_open_gl_context();
bool create_open_gl_es_ctx();
bool create_open_gl_glew_ctx();
void destroy_open_gl_context();


namespace
{
   const int shift_key_down = (1 << 0);
   const int ctrl_key_down = (1 << 1);
   const int alt_key_down = (1 << 2);
   int mod_keys_down = 0;
   mws_ptr_evt_base::e_pointer_press_type mouse_btn_down = mws_ptr_evt_base::e_not_pressed;

   // main

   mws_sp<msvc_main> instance;
   bool is_full_screen;
   bool is_window_flipped = false;
   uint32_t portrait_flip_count = 0;
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
   unicode_string wnd_class_name = untr("appplex");
   // screen metrix
   std::pair<uint32_t, uint32_t> screen_res;
   float avg_screen_dpi = 0.f;
   std::pair<float, float> screen_dpi;
   std::pair<float, float> screen_dim_inch;
   float avg_screen_dpcm = 0.f;
   std::pair<float, float> screen_dpcm;
   std::pair<float, float> screen_dim_cm;
   mws_path prv_path;
   mws_path res_path;
   mws_path tmp_path;
   bool prv_path_exists = false;
   bool tmp_path_exists = false;

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
}


mws_sp<mws_app> mws_app_inst() { return instance; }


class msvc_file_impl : public mws_file_impl
{
public:
   msvc_file_impl(const mws_path& i_path, bool i_is_internal = false) : mws_file_impl(i_path, i_is_internal) {}

   virtual FILE* get_file_impl() const override
   {
      return file;
   }

   virtual uint64_t length() override
   {
      std::string path = ppath.string();
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

   virtual uint64_t creation_time()const override
   {
      std::string path = ppath.string();
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

   virtual uint64_t last_write_time()const override
   {
      std::string path = ppath.string();
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

   virtual bool open_impl(std::string i_open_mode) override
   {
      std::string path = ppath.string();
#pragma warning(suppress : 4996)
      file = fopen(path.c_str(), i_open_mode.c_str());
      bool file_opened = (file != nullptr);

      return file_opened;
   }

   virtual void close_impl() override
   {
      if (file)
      {
         fclose(file);
         file = nullptr;
      }
   }

   virtual void flush_impl() override
   {
      fflush(file);
   }

   FILE* file = nullptr;
};


// helper functions
ATOM				register_new_window_class(HINSTANCE hinstance);
HWND				create_app_window(HINSTANCE, RECT& iclient_rect);
LRESULT CALLBACK	wnd_proc(HWND, UINT, WPARAM, LPARAM);


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

mws_key_types msvc_main::translate_key(int i_pfm_key_id) const
{
   // test if key is a number
   if (i_pfm_key_id >= '0' && i_pfm_key_id <= '9')
   {
      int diff = i_pfm_key_id - '0';

      return mws_key_types(mws_key_0 + diff);
   }
   // test if key is a letter
   else if (i_pfm_key_id >= 'A' && i_pfm_key_id <= 'Z')
   {
      int diff = i_pfm_key_id - 'A';

      return mws_key_types(mws_key_a + diff);
   }

   // none of the above, so it's a special key
   switch (i_pfm_key_id)
   {
   case VK_BACK: return mws_key_backspace;
   case VK_TAB: return mws_key_tab;
   case VK_CLEAR: return mws_key_num5;
   case VK_RETURN: return mws_key_enter;
   case VK_SHIFT: return mws_key_shift;
   case VK_CONTROL: return mws_key_control;
   case VK_MENU: return mws_key_alt;
   case VK_ESCAPE: return mws_key_escape;
   case VK_SPACE: return mws_key_space;
   case VK_END: return mws_key_end;
   case VK_HOME: return mws_key_home;
   case VK_LEFT: return mws_key_left;
   case VK_UP: return mws_key_up;
   case VK_RIGHT: return mws_key_right;
   case VK_DOWN: return mws_key_down;
   case VK_INSERT: return mws_key_insert;
   case VK_DELETE: return mws_key_delete;
   case VK_NUMPAD0: return mws_key_num0;
   case VK_NUMPAD1: return mws_key_num1;
   case VK_NUMPAD2: return mws_key_num2;
   case VK_NUMPAD3: return mws_key_num3;
   case VK_NUMPAD4: return mws_key_num4;
   case VK_NUMPAD5: return mws_key_num5;
   case VK_NUMPAD6: return mws_key_num6;
   case VK_NUMPAD7: return mws_key_num7;
   case VK_NUMPAD8: return mws_key_num8;
   case VK_NUMPAD9: return mws_key_num9;
   case VK_MULTIPLY: return mws_key_num_multiply;
   case VK_ADD: return mws_key_num_add;
   case VK_SUBTRACT: return mws_key_num_subtract;
   case VK_DECIMAL: return mws_key_num_decimal;
   case VK_DIVIDE: return mws_key_num_divide;
   case VK_F1: return mws_key_f1;
   case VK_F2: return mws_key_f2;
   case VK_F3: return mws_key_f3;
   case VK_F4: return mws_key_f4;
   case VK_F5: return mws_key_f5;
   case VK_F6: return mws_key_f6;
   case VK_F7: return mws_key_f7;
   case VK_F8: return mws_key_f8;
   case VK_F9: return mws_key_f9;
   case VK_F10: return mws_key_f10;
   case VK_F11: return mws_key_f11;
   case VK_F12: return mws_key_f12;
   case VK_OEM_1: return mws_key_semicolon; // ';:' for US
   case VK_OEM_PLUS: return mws_key_equal_sign; // '+' any country
   case VK_OEM_COMMA: return  mws_key_comma; // ',' any country
   case VK_OEM_MINUS: return mws_key_minus_sign; // '-' any country
   case VK_OEM_PERIOD: return mws_key_period; // '.' any country
   case VK_OEM_2: return mws_key_slash; // '/?' for US
   case VK_OEM_3: return mws_key_grave_accent; // '`~' for US
   case VK_OEM_4: return mws_key_left_bracket; //  '[{' for US
   case VK_OEM_5: return mws_key_backslash; //  '\|' for US
   case VK_OEM_6: return mws_key_right_bracket; //  ']}' for US
   case VK_OEM_7: return mws_key_single_quote; //  ''"' for US
   }

   // key was not recognized. mark as invalid
   return mws_key_invalid;
}

mws_key_types msvc_main::apply_key_modifiers_impl(mws_key_types i_key_id) const
{
   if (i_key_id == mws_key_invalid)
   {
      return mws_key_invalid;
   }

   bool num_lock_active = false;
   bool shift_held = ((mod_keys_down & shift_key_down) != 0);

   if (i_key_id >= mws_key_0 && i_key_id <= mws_key_9)
   {
      if (shift_held)
      {
         int diff = i_key_id - mws_key_0;

         switch (diff)
         {
         case 0: return mws_key_right_parenthesis;
         case 1: return mws_key_exclamation;
         case 2: return mws_key_at_symbol;
         case 3: return mws_key_number_sign;
         case 4: return mws_key_dollar_sign;
         case 5: return mws_key_percent_sign;
         case 6: return mws_key_circumflex;
         case 7: return mws_key_ampersand;
         case 8: return mws_key_asterisk;
         case 9: return mws_key_left_parenthesis;
         }
      }
      else
      {
         return i_key_id;
      }
   }
   else if (i_key_id >= mws_key_a && i_key_id <= mws_key_z)
   {
      if (shift_held)
      {
         int diff = i_key_id - mws_key_a;

         return mws_key_types(mws_key_a_upper_case + diff);
      }
      else
      {
         return i_key_id;
      }
   }

   switch (i_key_id)
   {
   case mws_key_num0: return (num_lock_active) ? mws_key_0 : mws_key_insert;
   case mws_key_num1: return (num_lock_active) ? mws_key_1 : mws_key_end;
   case mws_key_num2: return (num_lock_active) ? mws_key_2 : mws_key_down;
   case mws_key_num3: return (num_lock_active) ? mws_key_3 : mws_key_page_down;
   case mws_key_num4: return (num_lock_active) ? mws_key_4 : mws_key_left;
   case mws_key_num5: return (num_lock_active) ? mws_key_5 : mws_key_enter;
   case mws_key_num6: return (num_lock_active) ? mws_key_6 : mws_key_right;
   case mws_key_num7: return (num_lock_active) ? mws_key_7 : mws_key_home;
   case mws_key_num8: return (num_lock_active) ? mws_key_8 : mws_key_up;
   case mws_key_num9: return (num_lock_active) ? mws_key_9 : mws_key_page_up;
   case mws_key_num_multiply: return (num_lock_active) ? mws_key_asterisk : mws_key_asterisk;
   case mws_key_num_add: return (num_lock_active) ? mws_key_plus_sign : mws_key_plus_sign;
   case mws_key_num_subtract: return (num_lock_active) ? mws_key_minus_sign : mws_key_minus_sign;
   case mws_key_num_decimal: return (num_lock_active) ? mws_key_period : mws_key_del;
   case mws_key_num_divide: return (num_lock_active) ? mws_key_slash : mws_key_slash;
   case mws_key_semicolon: return (shift_held) ? mws_key_colon : mws_key_semicolon; // ';:' for US
   case mws_key_equal_sign: return (shift_held) ? mws_key_plus_sign : mws_key_equal_sign; // '+' any country
   case mws_key_comma: return (shift_held) ? mws_key_less_than_sign : mws_key_comma; // ',' any country
   case mws_key_minus_sign: return (shift_held) ? mws_key_underscore : mws_key_minus_sign; // '-' any country
   case mws_key_period: return (shift_held) ? mws_key_greater_than_sign : mws_key_period; // '.' any country
   case mws_key_slash: return (shift_held) ? mws_key_question_mark : mws_key_slash; // '/?' for US
   case mws_key_grave_accent: return (shift_held) ? mws_key_tilde_sign : mws_key_grave_accent; // '`~' for US
   case mws_key_left_bracket: return (shift_held) ? mws_key_left_brace : mws_key_left_bracket; //  '[{' for US
   case mws_key_backslash: return (shift_held) ? mws_key_vertical_bar : mws_key_backslash; //  '\|' for US
   case mws_key_right_bracket: return (shift_held) ? mws_key_right_brace : mws_key_right_bracket; //  ']}' for US
   case mws_key_single_quote: return (shift_held) ? mws_key_double_quote : mws_key_single_quote; //  ''"' for US
   }

   return i_key_id;
}

// screen metrix
std::pair<uint32_t, uint32_t> msvc_main::get_screen_res_px() const { return screen_res; }
float msvc_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> msvc_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> msvc_main::get_screen_dim_inch() const { return screen_dim_inch; }
float msvc_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> msvc_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> msvc_main::get_screen_dim_cm() const { return screen_dim_cm; }

void msvc_main::flip_screen() const
{
   if (is_full_screen_mode())
   {
      // this only works in windowed apps
      return;
   }

   is_window_flipped = !is_window_flipped;

   auto u = mws_mod_ctrl::inst()->get_app_start_mod();
   int x = 0;
   int y = 0;
   int width = mws::screen::get_width();
   int height = mws::screen::get_height();
   bool start_full_screen = false;

   if (u)
   {
      auto mod_pref = u->get_preferences();
      int pref_width = mod_pref->get_preferred_screen_width();
      int pref_height = mod_pref->get_preferred_screen_height();
      start_full_screen = mod_pref->start_full_screen();

      if (pref_width > 0 && pref_height > 0)
      {
         width = pref_width;
         height = pref_height;
      }
   }

   if (is_window_flipped)
   {
      int t = width;
      width = height;
      height = t;
   }

   //#if defined MWS_DEBUG_BUILD

   x = GetSystemMetrics(SM_CXSCREEN) - width - 5;
   y = GetSystemMetrics(SM_CYSCREEN) - height - 5;

   if (width < height)
   {
      if (portrait_flip_count % 2 == 0)
      {
         y = GetSystemMetrics(SM_CYSCREEN) - height - 5;
      }
      else
      {
         y = 5;
      }

      portrait_flip_count++;
   }

   //#endif
   window_coord.left = x;
   window_coord.right = x + width;
   window_coord.top = y;
   window_coord.bottom = y + height;
   AdjustWindowRect(&window_coord, WS_OVERLAPPEDWINDOW, FALSE);
   SetWindowPos(hwnd, HWND_TOP, window_coord.left, window_coord.top, window_coord.right - window_coord.left, window_coord.bottom - window_coord.top, 0);

   //mws_println("flip_screen wnd[ x[ %d ] y [ %d ] w [ %d ] h [ %d ] ]", (int)x, (int)y, int(width), int(height));
}

bool msvc_main::is_full_screen_mode() const
{
   return is_full_screen;
}

void msvc_main::set_full_screen_mode(bool i_enabled) const
{
   if (is_full_screen != i_enabled)
   {
      long style = i_enabled ? WS_POPUP : WS_OVERLAPPEDWINDOW;

      if (!is_full_screen)
      {
         window_coord = get_window_coord();
      }

      SetWindowLong(hwnd, GWL_STYLE, style);

      if (i_enabled)
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

      is_full_screen = i_enabled;
      disable_paint = true;
   }
}

void msvc_main::write_text(const char* i_text)const
{
   if (i_text && console_handle != INVALID_HANDLE_VALUE)
   {
      printf(i_text);
   }

   OutputDebugStringA(i_text);
}

void msvc_main::write_text_nl(const char* i_text)const
{
   write_text(i_text);
   write_text("\n");
}

void msvc_main::write_text(const wchar_t* i_text)const
{
   if (i_text && console_handle != INVALID_HANDLE_VALUE)
   {
      wprintf(i_text);
   }

   OutputDebugStringW(i_text);
}

void msvc_main::write_text_v(const char* i_format, ...)const
{
   char dest[16000];
   va_list argptr;

   va_start(argptr, i_format);
   vsnprintf_s(dest, 16000 - 1, _TRUNCATE, i_format, argptr);
   va_end(argptr);

   if (i_format && console_handle != INVALID_HANDLE_VALUE)
   {
      printf(dest);
   }

   OutputDebugStringA(dest);
}

static bool mws_make_directory(const mws_path& i_path)
{
   bool path_exists = false;

   if (!i_path.exists())
   {
      path_exists = i_path.make_dir();
   }
   else
   {
      path_exists = true;
   }

   if (!path_exists)
   {
      mws_println("WARNING[ failed to create path [ %s ]]", i_path.string().c_str());
   }

   return path_exists;
}

mws_sp<mws_file_impl> msvc_main::new_mws_file_impl(const mws_path& i_path, bool i_is_internal) const
{
   return std::make_shared<msvc_file_impl>(i_path, i_is_internal);
}

static void mws_list_external_directory(const mws_path& i_directory, std::vector<mws_sp<mws_file>>& i_file_list, bool i_recursive)
{
   if (!i_directory.is_empty())
   {
      WIN32_FIND_DATAA fd;
      HANDLE hfind = 0;
      mws_path search_path = i_directory / "*.*";
      hfind = ::FindFirstFileA(search_path.string().c_str(), &fd);

      if (hfind != INVALID_HANDLE_VALUE)
      {
         do
         {
            // regular file
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
               std::string filename(fd.cFileName);
               mws_path path = i_directory / filename;
               mws_sp<msvc_file_impl> file_impl(new msvc_file_impl(path));

               i_file_list.push_back(mws_file::get_inst(file_impl));
            }
            // directory
            else if (fd.cFileName[0] != '.' && i_recursive)
            {
               mws_path sub_dir = i_directory / fd.cFileName;

               mws_list_external_directory(sub_dir, i_file_list, true);
            }
         }
         while (::FindNextFileA(hfind, &fd));

         ::FindClose(hfind);
      }
   }
}

mws_file_map msvc_main::list_internal_directory() const
{
   mws_file_map file_map;
   std::vector<mws_sp<mws_file>> list;

   mws_list_external_directory(mws_app_inst()->res_dir(), list, true);

   for (auto& file : list)
   {
      std::string filename = file->filename();

      // check for duplicate file names
      mws_assert(file_map.find(filename) == file_map.end());
      file_map[filename] = file;
   }

   return file_map;
}

std::vector<mws_sp<mws_file>> msvc_main::list_external_directory(const mws_path& i_directory, bool i_recursive) const
{
   std::vector<mws_sp<mws_file>> list;

   mws_list_external_directory(i_directory, list, i_recursive);

   return list;
}

const mws_path& msvc_main::prv_dir() const
{
   if (!prv_path_exists)
   {
      prv_path_exists = mws_make_directory(prv_path);
   }

   return prv_path;
}

const mws_path& msvc_main::res_dir() const
{
   return res_path;
}

const mws_path& msvc_main::tmp_dir() const
{
   if (!tmp_path_exists)
   {
      tmp_path_exists = mws_make_directory(tmp_path);
   }

   return tmp_path;
}

void msvc_main::reconfigure_directories(mws_sp<mws_mod> i_crt_mod)
{
   const mws_path mod_path = mws_path("../../..") / i_crt_mod->get_proj_rel_path();

   mws_assert(i_crt_mod != nullptr);
   prv_path = mws_path(mod_path / "/.prv/");
   res_path = mws_path(mws_path(mod_path / "/res/").string());
   tmp_path = mws_path(mod_path / "/.tmp/");
   prv_path_exists = false;
   tmp_path_exists = false;
}

std::string msvc_main::get_timezone_id()const
{
   return "Europe/Bucharest";
}

void msvc_main::write_text_nl(const wchar_t* i_text)const
{
   write_text(i_text);
   write_text(L"\n");
}

bool mws_path::make_dir() const
{
   int ret_val = _mkdir(string().c_str());

   return (ret_val == 0);
}

std::string mws_path::current_path()
{
   return std::filesystem::current_path().generic_string();
}

bool init_app(int argc, const char** argv)
{
   // pass arguments
   int wargc = 0;
   LPWSTR* arg_list = CommandLineToArgvW(GetCommandLineW(), &wargc);
   mws::args::set_str_args(argc, argv, true);
   mws::args::set_unicode_args(wargc, static_cast<unicode_char**>(arg_list), true);
   LocalFree(arg_list);

   mws_mod_ctrl::inst()->pre_init_app();

   app_has_window = mws_mod_ctrl::inst()->app_uses_gfx();
   mws_mod_ctrl::inst()->set_gfx_available(app_has_window);

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

      //int x = CW_USEDEFAULT;
      int x = 0;
      int y = 0;
      int width = mws::screen::get_width();
      int height = mws::screen::get_height();
      bool start_full_screen = false;
      auto start_mod = mws_mod_ctrl::inst()->get_app_start_mod();

      if (start_mod)
      {
         auto mod_pref = start_mod->get_preferences();
         int pref_width = mod_pref->get_preferred_screen_width();
         int pref_height = mod_pref->get_preferred_screen_height();
         start_full_screen = mod_pref->start_full_screen();
         emulate_mobile_screen = mod_pref->emulate_mobile_screen();

         if (pref_width > 0 && pref_height > 0)
         {
            width = pref_width;
            height = pref_height;
         }
      }

      //#if defined MWS_DEBUG_BUILD

      x = GetSystemMetrics(SM_CXSCREEN) - width - 5;

      if (height < GetSystemMetrics(SM_CYSCREEN))
      {
         y = GetSystemMetrics(SM_CYSCREEN) - height - 5;
      }
      else
      {
         y = 5;
      }

      //#endif

      window_coord.left = x;
      window_coord.right = x + width;
      window_coord.top = y;
      window_coord.bottom = y + height;
      AdjustWindowRect(&window_coord, WS_OVERLAPPEDWINDOW, FALSE);
      //mws_println("init_app wnd[ x[ %d ] y [ %d ] w [ %d ] h [ %d ] ]",
      //   (int)window_coord.left, (int)window_coord.top, int(window_coord.right - window_coord.left), int(window_coord.bottom - window_coord.top));

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

      // screen metrix
      {
         int horizontal_dim_mm = GetDeviceCaps(hdc_window, HORZSIZE);
         float horizontal_dim_cm = horizontal_dim_mm / 10.f;
         int vertical_dim_mm = GetDeviceCaps(hdc_window, VERTSIZE);
         float vertical_dim_cm = vertical_dim_mm / 10.f;
         uint32_t horizontal_res_px = (uint32_t)GetDeviceCaps(hdc_window, HORZRES);
         uint32_t vertical_res_px = (uint32_t)GetDeviceCaps(hdc_window, VERTRES);

         screen_res = std::make_pair(horizontal_res_px, vertical_res_px);
         screen_dim_cm = std::make_pair(horizontal_dim_cm, vertical_dim_cm);
         screen_dpcm = std::make_pair(screen_res.first / screen_dim_cm.first, screen_res.second / screen_dim_cm.second);
         avg_screen_dpcm = (screen_dpcm.first + screen_dpcm.second) * 0.5f;
         screen_dim_inch = std::make_pair(mws_cm(screen_dim_cm.first).to_in().val(), mws_cm(screen_dim_cm.second).to_in().val());
         screen_dpi = std::make_pair(screen_res.first / screen_dim_inch.first, screen_res.second / screen_dim_inch.second);
         avg_screen_dpi = (screen_dpi.first + screen_dpi.second) * 0.5f;
      }

      if (start_full_screen)
      {
         instance->set_full_screen_mode(true);
      }

      init_notify_icon_data();

      ShowWindow(hwnd, ncmd_show);
      UpdateWindow(hwnd);
   }

   // find console status
   bool console_exists = (subsys == subsys_console) ? true : AttachConsole(ATTACH_PARENT_PROCESS);

   if (console_exists)
   {
      console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

      // redirect unbuffered STDOUT to the console
      int hcon_handle = _open_osfhandle((long)console_handle, _O_TEXT);
      FILE* fp = _fdopen(hcon_handle, "w");
      *stdout = *fp;
      setvbuf(stdout, NULL, _IONBF, 0);
      std::ios_base::sync_with_stdio();
   }

   instance->init();

   return true;
}

int main_loop()
{
   instance->start();

   if (app_has_window)
   {
      return win_main_loop();
   }

   return console_main_loop();
}

void set_params(HINSTANCE ihinstance, bool incmd_show, lnk_subsystem isubsys)
{
   hinstance = ihinstance;
   ncmd_show = incmd_show;
   subsys = isubsys;
}

HWND get_hwnd()
{
   return hwnd;
}

HMENU get_hmenu()
{
   return hmenu;
}

void set_hmenu(HMENU ihmenu)
{
   hmenu = ihmenu;
}

UINT get_taskbar_created_msg()
{
   return wm_taskbarcreated;
}

void minimize_window()
{
   // add the icon to the system tray
   Shell_NotifyIcon(NIM_ADD, &notify_icon_data);
   ShowWindow(hwnd, SW_HIDE);
}

void restore_window()
{
   // remove the icon from the system tray
   Shell_NotifyIcon(NIM_DELETE, &notify_icon_data);
   ShowWindow(hwnd, SW_SHOW);
}


RECT get_window_coord()
{
   RECT client_area_coord = {};

   GetWindowRect(hwnd, &client_area_coord);

   return client_area_coord;
}

void init_notify_icon_data()
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
#pragma warning(suppress : 4996)
   wcscpy(notify_icon_data.szTip, TEXT("appplex"));

   // balloon
   notify_icon_data.uTimeout = 5000;
   notify_icon_data.uFlags = notify_icon_data.uFlags | NIF_INFO;
   notify_icon_data.dwInfoFlags = NIIF_INFO;
#pragma warning(suppress : 4996)
   wcscpy(notify_icon_data.szInfoTitle, TEXT("TITLE"));
#pragma warning(suppress : 4996)
   wcscpy(notify_icon_data.szInfo, TEXT("SOME TEXT"));
}

int console_main_loop()
{
   const int timerInterval = 1000 / mws::screen::get_target_fps();
   long curTime = mws::time::get_time_millis();
   long nextUpdateTime = curTime;

   while (!mws_mod_ctrl::inst()->is_set_app_exit_on_next_run())
   {
      curTime = mws::time::get_time_millis();

      if (curTime >= nextUpdateTime)
      {
         instance->run();
         nextUpdateTime = curTime + timerInterval;
      }
      else
      {
         Sleep(15);
      }
   }

   mws_mod_ctrl::inst()->destroy_app();

   return 0;
}

int win_main_loop()
{
   const int timer_interval = 1000 / mws::screen::get_target_fps();
   uint32_t current_time = mws::time::get_time_millis();
   uint32_t next_update_time = current_time;
   MSG msg;

   while (!mws_mod_ctrl::inst()->is_set_app_exit_on_next_run())
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

      current_time = mws::time::get_time_millis();

      if (current_time >= next_update_time)
      {
         instance->run();
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

   mws_mod_ctrl::inst()->destroy_app();
   destroy_open_gl_context();
   ReleaseDC(hwnd, hdc_window);
   DestroyWindow(hwnd);

   return msg.wParam;
}

bool create_open_gl_context()
{
#if defined MWS_USES_OPENGL_ES
   return create_open_gl_es_ctx();
#elif defined MWS_USES_OPENGL_GLEW
   return create_open_gl_glew_ctx();
#else
   return false;
#endif
}

bool create_open_gl_es_ctx()
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

bool create_open_gl_glew_ctx()
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
         mws_print("MSAA samples per pixel count: [%d]\n", sampleCount);
      }
   }

   GLenum init = glewInit();

   if (init != GLEW_OK)
   {
      return false;
   }

   if (GL_VERSION_4_2)
   {
      mws_print("GLEW_VERSION_4_2 supported\n");
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

#else

return false;

#endif
}

void destroy_open_gl_context()
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
   wcex.lpszClassName = wnd_class_name.c_str();

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
   //	hWnd = CreateWindow(mws_mod_ctrl::getAppName().c_str(), mws_mod_ctrl::getAppName().c_str(), WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hinstance, NULL);

   HWND hWnd;

   int x = iclient_rect.left;
   int y = iclient_rect.top;
   int width = iclient_rect.right - x;
   int height = iclient_rect.bottom - y;

   hWnd = CreateWindow(wnd_class_name.c_str(), mws_mod_ctrl::inst()->app_name().c_str(),
      WS_OVERLAPPEDWINDOW, x, y, width, height, NULL, NULL, hinstance, NULL);

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
   if (message == get_taskbar_created_msg() && !IsWindowVisible(get_hwnd()))
   {
      minimize_window();
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
      set_hmenu(CreatePopupMenu());
      AppendMenu(get_hmenu(), MF_STRING, ID_TRAY_EXIT_CONTEXT_MENU_ITEM, TEXT("Exit"));
      break;

   case WM_SYSCOMMAND:
      switch (wparam & 0xfff0)
         // filter out reserved lower 4 bits. see msdn remarks http://msdn.microsoft.com/en-us/library/ms646360(VS.85).aspx
      {
      case SC_MINIMIZE:
      case SC_CLOSE:
         minimize_window();
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
         restore_window();
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
            get_hmenu(),
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
   case WM_RBUTTONDOWN:
   case WM_MBUTTONDOWN:
   {
      auto pfm_te = mws_ptr_evt_base::nwi();
      POINT pointer_coord = get_pointer_coord(hwnd);
      mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = true;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = mws::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = mws_ptr_evt_base::touch_began;

      switch (message)
      {
      case WM_LBUTTONDOWN:
         mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_left_mouse_btn; break;
      case WM_RBUTTONDOWN:
         mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_right_mouse_btn; break;
      case WM_MBUTTONDOWN:
         mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_middle_mouse_btn; break;
      }

      mws_mod_ctrl::inst()->pointer_action(pfm_te);

      return 0;
   }

   case WM_LBUTTONUP:
   case WM_RBUTTONUP:
   case WM_MBUTTONUP:
   {
      auto pfm_te = mws_ptr_evt_base::nwi();
      POINT pointer_coord = get_pointer_coord(hwnd);
      mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = true;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = mws::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = mws_ptr_evt_base::touch_ended;

      switch (message)
      {
      case WM_LBUTTONUP:
         pfm_te->press_type = mws_ptr_evt_base::e_left_mouse_btn; break;
      case WM_RBUTTONUP:
         pfm_te->press_type = mws_ptr_evt_base::e_right_mouse_btn; break;
      case WM_MBUTTONUP:
         pfm_te->press_type = mws_ptr_evt_base::e_middle_mouse_btn; break;
      }

      mouse_btn_down = mws_ptr_evt_base::e_not_pressed;
      mws_mod_ctrl::inst()->pointer_action(pfm_te);

      return 0;
   }

   case WM_MOUSEMOVE:
   {
      auto pfm_te = mws_ptr_evt_base::nwi();
      POINT pointer_coord = get_pointer_coord(hwnd);
      mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

      te.identifier = 0;
      te.is_changed = true;
      te.x = (float)pointer_coord.x;
      te.y = (float)pointer_coord.y;
      pfm_te->time = mws::time::get_time_millis();
      pfm_te->touch_count = 1;
      pfm_te->type = mws_ptr_evt_base::touch_moved;
      pfm_te->press_type = mouse_btn_down;

      mws_mod_ctrl::inst()->pointer_action(pfm_te);

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
      RECT client_area_coord = get_window_coord();
      int width = mws::screen::get_width();
      int height = mws::screen::get_height();

      if (pointer_coord.x >= 0 && pointer_coord.y >= 0 && pointer_coord.x < width && pointer_coord.y < height)
      {
         auto pfm_te = mws_ptr_evt_base::nwi();
         mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

         te.identifier = 0;
         te.is_changed = false;
         te.x = (float)pointer_coord.x;
         te.y = (float)pointer_coord.y;
         pfm_te->time = mws::time::get_time_millis();
         pfm_te->touch_count = 1;
         pfm_te->type = mws_ptr_evt_base::mouse_wheel;
         pfm_te->mouse_wheel_delta = (float)wheel_delta;

         mws_mod_ctrl::inst()->pointer_action(pfm_te);
         //mws_print("mouse wheel %1% %2% %3%") % wheel_delta % pointer_coord.x % pointer_coord.y;
      }

      return 0;
   }

   case WM_KEYDOWN:
   case WM_SYSKEYDOWN:
   {
      mws_key_types key_id = instance->translate_key(wparam);

      switch (wparam)
      {
      case VK_SHIFT: mod_keys_down |= shift_key_down; break;
      case VK_CONTROL: mod_keys_down |= ctrl_key_down; break;
      case VK_MENU: mod_keys_down |= alt_key_down; break;
      }

      mws_mod_ctrl::inst()->key_action(mws_key_press, key_id);

      return 0;
   }

   case WM_KEYUP:
   case WM_SYSKEYUP:
   {
      mws_key_types key_id = instance->translate_key(wparam);

      mws_mod_ctrl::inst()->key_action(mws_key_release, key_id);

      if (key_id == mws_key_escape)
      {
         bool back = mws_mod_ctrl::inst()->back_evt();
         mws_mod_ctrl::inst()->set_app_exit_on_next_run(back);
      }

      switch (wparam)
      {
      case VK_SHIFT: mod_keys_down &= ~shift_key_down; break;
      case VK_CONTROL: mod_keys_down &= ~ctrl_key_down; break;
      case VK_MENU: mod_keys_down &= ~alt_key_down; break;
      }

      return 0;
   }

   case WM_CLOSE:
      printf("Got an actual WM_CLOSE Message!  Woo hoo!\n");
      minimize_window();
      return 0;

   case WM_SIZE:
   {
      int width = LOWORD(lparam);
      int height = HIWORD(lparam);

      height = (height > 0) ? height : 1;
      mws_mod_ctrl::inst()->resize_app(width, height);

      return 0;
   }
   case WM_GETMINMAXINFO:
   {
      DefWindowProc(hwnd, message, wparam, lparam);

      MINMAXINFO* pmmi = (MINMAXINFO*)lparam;
      pmmi->ptMaxTrackSize.x = 1600;
      pmmi->ptMaxTrackSize.y = 1600;

      return 0;
   }

   //case WM_DESTROY:
   //	PostQuitMessage(0);
   //	break;
   }

   return DefWindowProc(hwnd, message, wparam, lparam);
}

int mws_is_gl_extension_supported(const char* i_extension)
{
   GLboolean is_supported = (glewIsSupported(i_extension) == 0) ? 0 : 1;

   return is_supported;
}


// app entry points
int APIENTRY _tWinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, LPTSTR lpcmd_line, int ncmd_show)
// gui subsystem entry point
{
   UNREFERENCED_PARAMETER(hprev_instance);
   UNREFERENCED_PARAMETER(lpcmd_line);

   instance = mws_sp<msvc_main>(new msvc_main());
   set_params(hinstance, ncmd_show, subsys_windows);

   if (init_app(0, nullptr))
   {
      return main_loop();
   }

   return -1;
}


int main(int argc, const char** argv)
// console subsystem entry point
{
   HINSTANCE hinstance = GetModuleHandle(NULL);
   bool nCmdShow = true;

   instance = mws_sp<msvc_main>(new msvc_main());
   set_params(hinstance, nCmdShow, subsys_console);

   if (init_app(argc, argv))
   {
      return main_loop();
   }

   return -1;
}


#endif
