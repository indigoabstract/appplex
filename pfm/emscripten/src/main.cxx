#include "pfm-gl.h"
#include "pfm.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "gfx.hxx"
#include "input/input-ctrl.hxx"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <fstream>
#include <filesystem>


#define VK_BACK 8
#define VK_TAB 9
#define VK_CLEAR 12
#define VK_RETURN 13
#define VK_SHIFT 16
#define VK_CONTROL 17
#define VK_MENU 18
#define VK_ESCAPE 27
#define VK_SPACE 32
//#define VK_NUMPAD9 33
//#define VK_NUMPAD3 34
#define VK_END 35
#define VK_HOME 36
#define VK_LEFT 37
#define VK_UP 38
#define VK_RIGHT 39
#define VK_DOWN 40
#define VK_INSERT 45
#define VK_NUMPAD0 96
#define VK_NUMPAD1 97
#define VK_NUMPAD2 98
#define VK_NUMPAD3 99
#define VK_NUMPAD4 100
#define VK_NUMPAD5 101
#define VK_NUMPAD6 102
#define VK_NUMPAD7 103
#define VK_NUMPAD8 104
#define VK_NUMPAD9 105
#define VK_DELETE 46
#define VK_MULTIPLY 106
#define VK_ADD 107
#define VK_SEPARATOR 111
#define VK_SUBTRACT 109
#define VK_DECIMAL 110
#define VK_DIVIDE 111
#define VK_F1 112
#define VK_F2 113
#define VK_F3 114
#define VK_F4 115
#define VK_F5 116
#define VK_F6 117
#define VK_F7 118
#define VK_F8 119
#define VK_F9 120
#define VK_F10 121
#define VK_F11 122
#define VK_F12 123
#define VK_OEM_1 59 // ';:' for US
#define VK_OEM_PLUS 61 // '+' any country
#define VK_OEM_COMMA 188 // ',' any country
#define VK_OEM_MINUS 173 // '-' any country
#define VK_OEM_PERIOD 190 // '.' any country
#define VK_OEM_2 191 // '/?' for US
#define VK_OEM_3 192 // '`~' for US
#define VK_OEM_4 219 //  '[{' for US
#define VK_OEM_5 220 //  '\|' for US
#define VK_OEM_6 221 //  ']}' for US
#define VK_OEM_7 222 //  ''"' for US


class emst_main : public mws_pfm_app
{
public:
   virtual ~emst_main();
   virtual void init() override;
   // input
   virtual mws_key_types translate_key(int i_pfm_key_id) const override;
   virtual mws_key_types apply_key_modifiers_impl(mws_key_types i_key_id) const override;
   // screen
   virtual bool is_full_screen_mode() const override;
   virtual void set_full_screen_mode(bool i_enabled) const override;
   // screen metrix
   virtual std::pair<uint32, uint32> get_screen_res_px() const override;
   virtual float get_avg_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dpi() const override;
   virtual std::pair<float, float> get_screen_dim_inch() const override;
   virtual float get_avg_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dpcm() const override;
   virtual std::pair<float, float> get_screen_dim_cm() const override;
   // log
   virtual void write_text(const char* i_text) const override;
   virtual void write_text_nl(const char* i_text) const override;
   virtual void write_text(const wchar_t* i_text) const override;
   virtual void write_text_nl(const wchar_t* i_text) const override;
   virtual void write_text_v(const char* i_format, ...) const override;
   // filesystem
   virtual mws_sp<mws_impl::mws_file_impl> new_mws_file_impl(const mws_path& i_path) const override;
   virtual const mws_path& prv_dir() const override;
   virtual const mws_path& res_dir() const override;
   virtual const mws_path& tmp_dir() const override;
   virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) override;
   virtual std::string get_timezone_id() const override;
   virtual umf_list get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive) const override;

   void init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi);
   void on_resize(uint32 i_screen_width, uint32 i_screen_height);

private:
   void setup_callbacks();

   umf_list plist;
   // screen metrix
   std::pair<uint32, uint32> screen_res;
   float avg_screen_dpi = 0.f;
   std::pair<float, float> screen_dpi;
   std::pair<float, float> screen_dim_inch;
   float avg_screen_dpcm = 0.f;
   std::pair<float, float> screen_dpcm;
   std::pair<float, float> screen_dim_cm;
};


namespace
{
	const int shift_key_down = (1 << 0);
	const int ctrl_key_down = (1 << 1);
	const int alt_key_down = (1 << 2);
	int mod_keys_down = 0;
	mws_ptr_evt_base::e_pointer_press_type mouse_btn_down = mws_ptr_evt_base::e_not_pressed;
   mws_path prv_path;
   mws_path res_path;
   mws_path tmp_path;
   bool prv_path_exists = false;
   bool tmp_path_exists = false;
   mws_sp<emst_main> instance;
}


static mws_sp<emst_main> app_inst()
{
   if (!instance)
   {
      instance = mws_sp<emst_main>(new emst_main());
   }

   return instance;
}

mws_sp<mws_pfm_app> mws_app_inst() { return app_inst(); }


class emst_file_impl : public mws_impl::mws_file_impl
{
public:
   emst_file_impl(const mws_path& i_path) : mws_impl::mws_file_impl(i_path) {}

   virtual ~emst_file_impl() {}

   virtual FILE* get_file_impl() const override
   {
      return file;
   }

   virtual uint64 length() override
   {
      uint64 size = 0;

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

   virtual uint64 creation_time() const override
   {
      return 0;
   }

   virtual uint64 last_write_time() const override
   {
      return 0;
   }

   virtual bool open_impl(std::string i_open_mode) override
   {
      const std::string& path = ppath.string();
      file = fopen(path.c_str(), i_open_mode.c_str());
      bool file_opened = (file != nullptr);
      mws_println("open_impl: opening external file %s success %d", path.c_str(), (int)file_opened);

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

private:
   FILE* file = nullptr;
};


emst_main::~emst_main()
{
}

void emst_main::init()
{
   // screen metrix
   {
      double screen_width = EM_ASM_DOUBLE({ return window.screen.width; });
      double screen_height = EM_ASM_DOUBLE({ return window.screen.height; });
      double horizontal_screen_dpi = EM_ASM_DOUBLE({ return Module.mws_horizontal_screen_dpi(); });
      double vertical_screen_dpi = EM_ASM_DOUBLE({ return Module.mws_vertical_screen_dpi(); });

      init_screen_metrix((uint32)screen_width, (uint32)screen_height, (float)horizontal_screen_dpi, (float)vertical_screen_dpi);
   }

   setup_callbacks();
   mws_mod_ctrl::inst()->pre_init_app();
   mws_mod_ctrl::inst()->set_gfx_available(true);
   auto start_mod = mws_mod_ctrl::inst()->get_app_start_mod();

   if (start_mod)
   {
      auto mod_pref = start_mod->get_preferences();
      mws_log::set_enabled(mod_pref->log_enabled());
   }

   mws_mod_ctrl::inst()->init_app();
}

mws_key_types emst_main::translate_key(int i_pfm_key_id) const
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

mws_key_types emst_main::apply_key_modifiers_impl(mws_key_types i_key_id) const
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

bool emst_main::is_full_screen_mode() const
{
   return true;
}

void emst_main::set_full_screen_mode(bool i_enabled) const
{
}

std::pair<uint32, uint32> emst_main::get_screen_res_px() const { return screen_res; }
float emst_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> emst_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> emst_main::get_screen_dim_inch() const { return screen_dim_inch; }
float emst_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> emst_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> emst_main::get_screen_dim_cm() const { return screen_dim_cm; }

void emst_main::write_text(const char* i_text) const
{
   printf("%s", i_text);
}

void emst_main::write_text_nl(const char* i_text) const
{
   printf("%s\n", i_text);
}

void emst_main::write_text(const wchar_t* i_text) const
{
   printf("wstring not supported");
}

void emst_main::write_text_nl(const wchar_t* i_text) const
{
   write_text(i_text);
   write_text(L"\n");
}

void emst_main::write_text_v(const char* i_format, ...) const
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
   std::filesystem::path fs_path(i_path.string());

   if (!std::filesystem::exists(fs_path))
   {
      path_exists = std::filesystem::create_directory(fs_path);
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

mws_sp<mws_impl::mws_file_impl> emst_main::new_mws_file_impl(const mws_path& i_path) const
{
   return std::make_shared<emst_file_impl>(i_path);
}

const mws_path& emst_main::prv_dir() const
{
   if (!prv_path_exists)
   {
      prv_path_exists = mws_make_directory(prv_path);
   }

   return prv_path;
}

const mws_path& emst_main::res_dir() const
{
   return res_path;
}

const mws_path& emst_main::tmp_dir() const
{
   if (!tmp_path_exists)
   {
      tmp_path_exists = mws_make_directory(tmp_path);
   }

   return tmp_path;
}

void emst_main::reconfigure_directories(mws_sp<mws_mod> i_crt_mod)
{
   std::string mod_dir = i_crt_mod->get_name();

   mws_assert(i_crt_mod != nullptr);
   prv_path = mws_path(mod_dir + "-prv/");
   res_path = mws_path(mod_dir + "/", false);
   tmp_path = mws_path(mod_dir + "-tmp/");
   prv_path_exists = false;
   tmp_path_exists = false;
}

std::string emst_main::get_timezone_id() const
{
   return "Europe/Bucharest";
}

umf_list emst_main::get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive) const
{
   if (i_plist->find(mws::filesys::res_idx_name) == i_plist->end())
   {
      mws_path base_dir(i_directory);
      umf_r& list = *i_plist;
      mws_sp<mws_file> index_txt = mws_file::get_inst(std::make_shared<emst_file_impl>(base_dir / mws::filesys::res_idx_name));
      std::ifstream infile(index_txt->string_path());
      //mws_println("i_directory %s file size %d path %s", i_directory.c_str(), index_txt->length(), path.c_str());

      if (infile.is_open())
      {
         std::string line;

         while (std::getline(infile, line))
         {
            // trim the new line at the end
            line = mws_str::rtrim(line);
            mws_path path(line);
            std::string filename = path.filename();

            if (!base_dir.is_empty())
            {
               path = base_dir / path;
            }

            list[filename] = mws_file::get_inst(std::make_shared<emst_file_impl>(path));
         }

         list[mws::filesys::res_idx_name] = index_txt;
         infile.close();
      }
   }

   return i_plist;
}

void emst_main::init_screen_metrix(uint32 i_screen_width, uint32 i_screen_height, float i_screen_horizontal_dpi, float i_screen_vertical_dpi)
{
   const float scale_factor = 1.33f;
   i_screen_horizontal_dpi *= scale_factor;
   i_screen_vertical_dpi *= scale_factor;
   mws_println("resolution width [ %d ] height [ %d ] horizontal_dpi [ %3.2f ] vertical_dpi [ %3.2f ]",
      i_screen_width, i_screen_height, i_screen_horizontal_dpi, i_screen_vertical_dpi);

   float horizontal_dim_inch = i_screen_width / i_screen_horizontal_dpi;
   float vertical_dim_inch = i_screen_height / i_screen_vertical_dpi;
   float horizontal_dim_cm = mws_in(horizontal_dim_inch).to_cm().val();
   float vertical_dim_cm = mws_in(vertical_dim_inch).to_cm().val();
   float horizontal_screen_dpcm = mws_cm(i_screen_horizontal_dpi).to_in().val();
   float vertical_screen_dpcm = mws_cm(i_screen_vertical_dpi).to_in().val();

   screen_res = std::make_pair((uint32)i_screen_width, (uint32)i_screen_height);
   screen_dim_inch = std::make_pair(horizontal_dim_inch, vertical_dim_inch);
   screen_dpi = std::make_pair(i_screen_horizontal_dpi, i_screen_vertical_dpi);
   avg_screen_dpi = (screen_dpi.first + screen_dpi.second) * 0.5f;
   screen_dim_cm = std::make_pair(horizontal_dim_cm, vertical_dim_cm);
   screen_dpcm = std::make_pair(horizontal_screen_dpcm, vertical_screen_dpcm);
   avg_screen_dpcm = (screen_dpcm.first + screen_dpcm.second) * 0.5f;
}

void emst_main::on_resize(uint32 i_screen_width, uint32 i_screen_height)
{
   bool is_landscape_0 = (i_screen_width > i_screen_height);
   bool is_landscape_1 = (screen_res.first > screen_res.second);

   if (is_landscape_0 != is_landscape_1)
   {
      std::swap(screen_res.first, screen_res.second);
      std::swap(screen_dpi.first, screen_dpi.second);
      std::swap(screen_dim_inch.first, screen_dim_inch.second);
      std::swap(screen_dpcm.first, screen_dpcm.second);
      std::swap(screen_dim_cm.first, screen_dim_cm.second);
   }

   mws_mod_ctrl::inst()->resize_app(i_screen_width, i_screen_height);
}

static EM_BOOL mws_emst_key_down(int i_event_type, const EmscriptenKeyboardEvent* i_e, void* i_user_data)
{
   mws_key_types key_id = mws::input::translate_key(i_e->keyCode);

   switch (key_id)
   {
   case mws_key_shift: mod_keys_down |= shift_key_down; break;
   case mws_key_control: mod_keys_down |= ctrl_key_down; break;
   case mws_key_alt: mod_keys_down |= alt_key_down; break;
   }

   mws_mod_ctrl::inst()->key_action(mws_key_press, key_id);

   switch (key_id)
   {
   case mws_key_tab:
   case mws_key_backspace:
   case mws_key_enter:
   case mws_key_slash:
   case mws_key_single_quote:
   case mws_key_num_divide:
      return true;
   }

   return false;
}

static EM_BOOL mws_emst_key_up(int i_event_type, const EmscriptenKeyboardEvent* i_e, void* i_user_data)
{
   mws_key_types key_id = mws::input::translate_key(i_e->keyCode);

   switch (key_id)
   {
   case mws_key_shift: mod_keys_down &= ~shift_key_down; break;
   case mws_key_control: mod_keys_down &= ~ctrl_key_down; break;
   case mws_key_alt: mod_keys_down &= ~alt_key_down; break;
   }

   mws_mod_ctrl::inst()->key_action(mws_key_release, key_id);

   return false;
}

static EM_BOOL mws_emst_key_press(int i_event_type, const EmscriptenKeyboardEvent* i_e, void* i_user_data)
{
   return false;
}

static EM_BOOL mws_emst_mouse_down(int i_event_type, const EmscriptenMouseEvent* i_e, void* i_user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)i_e->canvasX;
   te.y = (float)i_e->canvasY;
   pfm_te->time = mws::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_began;
   
	switch (i_e->button)
	{
	case 0:
	 mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_left_mouse_btn; break;
	case 1:
	 mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_middle_mouse_btn; break;
	case 2:
	 mouse_btn_down = pfm_te->press_type = mws_ptr_evt_base::e_right_mouse_btn; break;
	}

   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

static EM_BOOL mws_emst_mouse_up(int i_event_type, const EmscriptenMouseEvent* i_e, void* i_user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)i_e->canvasX;
   te.y = (float)i_e->canvasY;
   pfm_te->time = mws::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_ended;

	switch (i_e->button)
	{
	case 0:
	 pfm_te->press_type = mws_ptr_evt_base::e_left_mouse_btn; break;
	case 1:
	 pfm_te->press_type = mws_ptr_evt_base::e_middle_mouse_btn; break;
	case 2:
	 pfm_te->press_type = mws_ptr_evt_base::e_right_mouse_btn; break;
	}

	mouse_btn_down = mws_ptr_evt_base::e_not_pressed;
   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

static EM_BOOL mws_emst_mouse_move(int i_event_type, const EmscriptenMouseEvent* i_e, void* i_user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)i_e->canvasX;
   te.y = (float)i_e->canvasY;
   pfm_te->time = mws::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_moved;
   pfm_te->press_type = mouse_btn_down;
   
   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

static EM_BOOL mws_emst_mouse_wheel(int i_event_type, const EmscriptenWheelEvent* i_e, void* i_user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = 0.f;
   te.y = 0.f;
   pfm_te->time = mws::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::mouse_wheel;
   pfm_te->mouse_wheel_delta = float(i_e->deltaY * -0.09f);

   mws_mod_ctrl::inst()->pointer_action(pfm_te);
   //trx("mouse wheel %1% %2% %3%") % wheel_delta % pointer_coord.x % pointer_coord.y;

   return true;
}

static EM_BOOL mws_emst_touch(int i_event_type, const EmscriptenTouchEvent* i_e, void* i_user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();

   switch (i_event_type)
   {
   case EMSCRIPTEN_EVENT_TOUCHSTART:
      pfm_te->type = mws_ptr_evt_base::touch_began;
      break;
   case EMSCRIPTEN_EVENT_TOUCHEND:
      pfm_te->type = mws_ptr_evt_base::touch_ended;
      break;
   case EMSCRIPTEN_EVENT_TOUCHMOVE:
      pfm_te->type = mws_ptr_evt_base::touch_moved;
      break;
   case EMSCRIPTEN_EVENT_TOUCHCANCEL:
      pfm_te->type = mws_ptr_evt_base::touch_cancelled;
      break;
   default:
      return false;
   }

   pfm_te->time = mws::time::get_time_millis();
   pfm_te->touch_count = i_e->numTouches;

   for (uint32 k = 0; k < pfm_te->touch_count; k++)
   {
      auto& touches = i_e->touches[k];
      mws_ptr_evt_base::touch_point& point = pfm_te->points[k];

      point.identifier = touches.identifier;
      point.x = touches.canvasX;
      point.y = touches.canvasY;
      point.is_changed = touches.isChanged;
   }

   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

void emst_main::setup_callbacks()
{
   emscripten_set_keydown_callback(0, this, true, mws_emst_key_down);
   emscripten_set_keyup_callback(0, this, true, mws_emst_key_up);
   emscripten_set_keypress_callback(0, this, true, mws_emst_key_press);
   emscripten_set_mousedown_callback("#canvas", this, true, mws_emst_mouse_down);
   emscripten_set_mouseup_callback("#canvas", this, true, mws_emst_mouse_up);
   emscripten_set_mousemove_callback("#canvas", this, true, mws_emst_mouse_move);
   emscripten_set_wheel_callback("#canvas", this, false, mws_emst_mouse_wheel);
   emscripten_set_touchstart_callback("#canvas", this, true, mws_emst_touch);
   emscripten_set_touchend_callback("#canvas", this, true, mws_emst_touch);
   emscripten_set_touchmove_callback("#canvas", this, true, mws_emst_touch);
   emscripten_set_touchcancel_callback("#canvas", this, true, mws_emst_touch);
   //emscripten_set_devicemotion_callback(this, true, emst_device_motion);
   //emscripten_set_deviceorientation_callback(this, true, emst_device_orientation);
}


static inline const char* mws_emscripten_event_type_to_string(int i_event_type)
{
   const char* events[] =
   {
     "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
     "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange",
     "pointerlockchange", "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected",
     "beforeunload", "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)"
   };

   ++i_event_type;

   if (i_event_type < 0) { i_event_type = 0; }
   if (i_event_type >= sizeof(events) / sizeof(events[0])) { i_event_type = sizeof(events) / sizeof(events[0]) - 1; }

   return events[i_event_type];
}

static const char* mws_emscripten_result_to_string(EMSCRIPTEN_RESULT i_result)
{
   if (i_result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
   if (i_result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
   if (i_result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
   if (i_result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
   if (i_result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
   if (i_result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
   if (i_result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
   if (i_result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
   if (i_result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";

   return "Unknown EMSCRIPTEN_RESULT!";
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, mws_emscripten_result_to_string(ret));

static void mws_get_canvas_size(int* i_width, int* i_height, int* i_is_full_screen)
{
   EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", i_width, i_height);
   EmscriptenFullscreenChangeEvent e;

   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      printf("mws_get_canvas_size: failed getting canvas size\n");
      return;
   }

   r = emscripten_get_fullscreen_status(&e);

   if (r != EMSCRIPTEN_RESULT_SUCCESS)
   {
      printf("mws_get_canvas_size: failed getting fullscreen status\n");
      return;
   }

   *i_is_full_screen = e.isFullscreen;
}

static EM_BOOL mws_on_canvassize_changed(int i_event_type, const void* i_reserved, void* i_user_data)
{
   int width, height, fs;
   mws_get_canvas_size(&width, &height, &fs);
   double css_width, css_height;
   emscripten_get_element_css_size(0, &css_width, &css_height);
   height = (height > 0) ? height : 1;
   app_inst()->on_resize(width, height);

   printf("canvas resized: WebGL RTT size: %dx%d, canvas CSS size: %02gx%02g\n", width, height, css_width, css_height);

   return 0;
}

static void mws_request_fullscreen(int i_scale_mode, int i_canvas_resolution_scale_mode, int i_filtering_mode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = i_scale_mode;
   s.canvasResolutionScaleMode = i_canvas_resolution_scale_mode;
   s.filteringMode = i_filtering_mode;
   s.canvasResizedCallback = mws_on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(0, 1, &s);
   TEST_RESULT(mws_request_fullscreen);
}

static void mws_enter_soft_fullscreen(int i_scale_mode, int i_canvas_resolution_scale_mode, int i_filtering_mode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = i_scale_mode;
   s.canvasResolutionScaleMode = i_canvas_resolution_scale_mode;
   s.filteringMode = i_filtering_mode;
   s.canvasResizedCallback = mws_on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen(0, &s);
   TEST_RESULT(mws_enter_soft_fullscreen);
}

static void mws_run_step()
{
   app_inst()->run();
}

// implement this gl function missing in emscripten
GL_APICALL void GL_APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
   EM_ASM_(
      {
         Module.ctx.getBufferSubData(Module.ctx.PIXEL_PACK_BUFFER, 0, HEAPU8.subarray($0, $0 + $1));
      }, data, size);
}

GL_APICALL void* GL_APIENTRY glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
   mws_throw mws_exception("glMapBufferRange n/a"); return nullptr;
}

GL_APICALL GLboolean GL_APIENTRY glUnmapBuffer(GLenum target)
{
   mws_throw mws_exception("glUnmapBuffer n/a"); return (GLboolean)false;
}


int main()
{
   mws_enter_soft_fullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT);

   EmscriptenWebGLContextAttributes attr;

   emscripten_webgl_init_context_attributes(&attr);
   attr.alpha = attr.stencil = attr.preserveDrawingBuffer = attr.preferLowPowerToHighPerformance = attr.failIfMajorPerformanceCaveat = 0;
   attr.antialias = attr.depth = 1;
   attr.enableExtensionsByDefault = 1;
   attr.premultipliedAlpha = 0;
   //attr.alpha = 1;

#if MWS_OPENGL_ES_VERSION == MWS_OPENGL_ES_2_0

   attr.majorVersion = 1;
   attr.minorVersion = 0;

#elif MWS_OPENGL_ES_VERSION == MWS_OPENGL_ES_3_0

   attr.majorVersion = 2;
   attr.minorVersion = 0;

#endif // MWS_OPENGL_ES_VERSION == MWS_OPENGL_ES_2_0

   EMSCRIPTEN_WEBGL_CONTEXT_HANDLE ctx = emscripten_webgl_create_context(0, &attr);
   emscripten_webgl_make_context_current(ctx);

   EM_BOOL anisotropy_enabled = emscripten_webgl_enable_extension(ctx, "EXT_texture_filter_anisotropic");
   mws_print("anisotropy enabled: %d\n", anisotropy_enabled);

   app_inst()->init();
   app_inst()->start();

   emscripten_set_main_loop(mws_run_step, 0, 0);

   return 0;
}
