#include "stdafx.hxx"

#include "pfm-def.h"

#if defined MWS_PFM_LINUX

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
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>


class linux_main : public mws_app
{
public:
   linux_main();
   virtual ~linux_main();
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


bool init_app(int argc, char** argv);
int main_loop();
void init_notify_icon_data();
int console_main_loop();


namespace
{
   const int shift_key_down = (1 << 0);
   const int ctrl_key_down = (1 << 1);
   const int alt_key_down = (1 << 2);
   int mod_keys_down = 0;
   mws_ptr_evt_base::e_pointer_press_type mouse_btn_down = mws_ptr_evt_base::e_not_pressed;

   // main

   mws_sp<linux_main> instance;
   bool is_full_screen = false;
   bool is_window_flipped = false;
   uint32_t portrait_flip_count = 0;
   bool emulate_mobile_screen = false;
   bool disable_paint = false;
   lnk_subsystem subsys;
   bool app_has_window = false;
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
}


mws_sp<mws_app> mws_app_inst() { return instance; }


class linux_file_impl : public mws_file_impl
{
public:
   linux_file_impl(const mws_path& i_path, bool i_is_internal = false) : mws_file_impl(i_path, i_is_internal) {}

   virtual FILE* get_file_impl() const override
   {
      return file;
   }

   virtual uint64_t length() override
   {
      uint64_t size = 0;

      if (!file)
      {
         open("rb");

         if (file)
         {
            fseek(file, 0, SEEK_END);
            size = ftell(file);
            close();
         }
      }
      else
      {
         long crt_pos = ftell(file);

         fseek(file, 0, SEEK_END);
         size = ftell(file);
         fseek(file, crt_pos, SEEK_SET);
      }

      return size;
   }

   virtual uint64_t creation_time()const override
   {
      return 0;
   }

   virtual uint64_t last_write_time()const override
   {
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
      if (file)
      {
         fflush(file);
      }
   }

   FILE* file = nullptr;
};


// helper functions


linux_main::linux_main()
{
   is_full_screen = false;
   disable_paint = false;
   subsys = subsys_console;
   app_has_window = false;
}

linux_main::~linux_main()
{
}

mws_key_types linux_main::translate_key(int i_pfm_key_id) const
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
/*
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
   */

   // key was not recognized. mark as invalid
   return mws_key_invalid;
}

mws_key_types linux_main::apply_key_modifiers_impl(mws_key_types i_key_id) const
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
std::pair<uint32_t, uint32_t> linux_main::get_screen_res_px() const { return screen_res; }
float linux_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> linux_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> linux_main::get_screen_dim_inch() const { return screen_dim_inch; }
float linux_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> linux_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> linux_main::get_screen_dim_cm() const { return screen_dim_cm; }

void linux_main::flip_screen() const
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

   //mws_println("flip_screen wnd[ x[ %d ] y [ %d ] w [ %d ] h [ %d ] ]", (int)x, (int)y, int(width), int(height));
}

bool linux_main::is_full_screen_mode() const
{
   return is_full_screen;
}

void linux_main::set_full_screen_mode(bool i_enabled) const
{
}

void linux_main::write_text(const char* i_text)const
{
   if (i_text)
   {
      printf(i_text);
   }
}

void linux_main::write_text_nl(const char* i_text)const
{
   write_text(i_text);
   write_text("\n");
}

void linux_main::write_text(const wchar_t* i_text)const
{
   if (i_text)
   {
      //wprintf(i_text);
   }

   //OutputDebugStringW(i_text);
}

void linux_main::write_text_v(const char* i_format, ...)const
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, i_format);
   vsnprintf(dest, 1024 * 16 - 1, i_format, arg_ptr);
   va_end(arg_ptr);
   printf("%s", dest);
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

mws_sp<mws_file_impl> linux_main::new_mws_file_impl(const mws_path& i_path, bool i_is_internal) const
{
   return std::make_shared<linux_file_impl>(i_path, i_is_internal);
}

mws_file_map linux_main::list_internal_directory() const
{
   mws_file_map file_map;
   std::vector<mws_sp<mws_file>> list = list_external_directory(mws_app_inst()->res_dir(), true);

   for (auto& file : list)
   {
      std::string filename = file->filename();

      // check for duplicate file names
      mws_assert(file_map.find(filename) == file_map.end());
      file_map[filename] = file;
   }

   return file_map;
}

const mws_path& linux_main::prv_dir() const
{
   if (!prv_path_exists)
   {
      prv_path_exists = mws_make_directory(prv_path);
   }

   return prv_path;
}

const mws_path& linux_main::res_dir() const
{
   return res_path;
}

const mws_path& linux_main::tmp_dir() const
{
   if (!tmp_path_exists)
   {
      tmp_path_exists = mws_make_directory(tmp_path);
   }

   return tmp_path;
}

void linux_main::reconfigure_directories(mws_sp<mws_mod> i_crt_mod)
{
   const mws_path mod_path = mws_path("../../..") / i_crt_mod->get_proj_rel_path();

   mws_assert(i_crt_mod != nullptr);
   prv_path = mws_path(mod_path / "/.prv/");
   res_path = mws_path(mws_path(mod_path / "/res/").string());
   tmp_path = mws_path(mod_path / "/.tmp/");
   prv_path_exists = false;
   tmp_path_exists = false;
}

std::string linux_main::get_timezone_id()const
{
   return "Europe/Bucharest";
}

void linux_main::write_text_nl(const wchar_t* i_text)const
{
   write_text(i_text);
   write_text(L"\n");
}
/*
std::string mws_path::current_path()
{
   return std::filesystem::current_path().generic_string();
}
*/
bool init_app(int argc, char** argv)
{
   // pass arguments
   mws::args::set_app_arguments(argc, argv, true);

   mws_mod_ctrl::inst()->pre_init_app();

   app_has_window = mws_mod_ctrl::inst()->app_uses_gfx();
   mws_mod_ctrl::inst()->set_gfx_available(app_has_window);

   if (app_has_window)
   {
   }

   instance->init();

   return true;
}

int main_loop()
{
   instance->start();

   if (app_has_window)
   {
      return -1;//win_main_loop();
   }

   return console_main_loop();
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
         std::this_thread::sleep_for(std::chrono::milliseconds(15));
      }
   }

   mws_mod_ctrl::inst()->destroy_app();

   return 0;
}

int mws_is_gl_extension_supported(const char* i_extension)
{
   GLboolean is_supported = false;

   return is_supported;
}


// app entry points


int main(int argc, char** argv)
// console subsystem entry point
{
   instance = mws_sp<linux_main>(new linux_main());

   if (init_app(argc, argv))
   {
      return main_loop();
   }

   return -1;
}

#endif

