#include "main.hxx"

#include "pfm-def.h"
#include "pfm-gl.h"
#include "pfm.hxx"
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


#define VK_BACK 8
#define VK_TAB 9
#define VK_CLEAR 12
#define VK_RETURN 13
#define VK_SHIFT 16
#define VK_CONTROL 17
#define VK_MENU 18
#define VK_ESCAPE 27
#define VK_SPACE 32
#define VK_NUMPAD9 33
#define VK_NUMPAD3 34
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


const int shift_key_down = (1 << 0);
const int ctrl_key_down = (1 << 1);
const int alt_key_down = (1 << 2);
static int mod_keys_down = 0;


class emst_file_impl : public pfm_impl::pfm_file_impl
{
public:
   emst_file_impl(const std::string& ifilename, const std::string& iroot_dir) : pfm_impl::pfm_file_impl(ifilename, iroot_dir) {}
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

   virtual uint64 creation_time()const override
   {
      return 0;
   }

   virtual uint64 last_write_time()const override
   {
      return 0;
   }

   virtual bool open_impl(std::string i_open_mode) override
   {
      std::string path = ppath.get_full_path();
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


mws_sp<emst_main> emst_main::instance;

emst_main::~emst_main()
{
}

mws_sp<emst_main> emst_main::get_instance()
{
   if (!instance)
   {
      instance = mws_sp<emst_main>(new emst_main());
   }

   return instance;
}

mws_sp<pfm_impl::pfm_file_impl> emst_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
   return std::make_shared<emst_file_impl>(ifilename, iroot_dir);
}

void emst_main::init()
{
   pfm_main::init();

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
   mws_mod_ctrl::inst()->init_app();
}

void emst_main::start()
{
   pfm_main::start();

   mws_mod_ctrl::inst()->start_app();
}

void emst_main::run()
{
   mws_mod_ctrl::inst()->update();
}

key_types emst_main::translate_key(int i_pfm_key_id) const
{
   // test if key is a number
   if (i_pfm_key_id >= '0' && i_pfm_key_id <= '9')
   {
      int diff = i_pfm_key_id - '0';

      return key_types(KEY_0 + diff);
   }
   // test if key is a letter
   else if (i_pfm_key_id >= 'A' && i_pfm_key_id <= 'Z')
   {
      int diff = i_pfm_key_id - 'A';

      return key_types(KEY_A + diff);
   }

   // none of the above, so it's a special key
   switch (i_pfm_key_id)
   {
   case VK_BACK: return KEY_BACKSPACE;
   case VK_TAB: return KEY_TAB;
   case VK_CLEAR: return KEY_NUM5;
   case VK_RETURN: return KEY_ENTER;
   case VK_SHIFT: return KEY_SHIFT;
   case VK_CONTROL: return KEY_CONTROL;
   case VK_MENU: return KEY_ALT;
   case VK_ESCAPE: return KEY_ESCAPE;
   case VK_SPACE: return KEY_SPACE;
   case VK_END: return KEY_END;
   case VK_HOME: return KEY_HOME;
   case VK_LEFT: return KEY_LEFT;
   case VK_UP: return KEY_UP;
   case VK_RIGHT: return KEY_RIGHT;
   case VK_DOWN: return KEY_DOWN;
   case VK_INSERT: return KEY_INSERT;
   case VK_DELETE: return KEY_DELETE;
   case VK_NUMPAD0: return KEY_NUM0;
   case VK_NUMPAD1: return KEY_NUM1;
   case VK_NUMPAD2: return KEY_NUM2;
   case VK_NUMPAD3: return KEY_NUM3;
   case VK_NUMPAD4: return KEY_NUM4;
   case VK_NUMPAD5: return KEY_NUM5;
   case VK_NUMPAD6: return KEY_NUM6;
   case VK_NUMPAD7: return KEY_NUM7;
   case VK_NUMPAD8: return KEY_NUM8;
   case VK_NUMPAD9: return KEY_NUM9;
   case VK_MULTIPLY: return KEY_NUM_MULTIPLY;
   case VK_ADD: return KEY_NUM_ADD;
   case VK_SUBTRACT: return KEY_NUM_SUBTRACT;
   case VK_DECIMAL: return KEY_NUM_DECIMAL;
   case VK_DIVIDE: return KEY_NUM_DIVIDE;
   case VK_F1: return KEY_F1;
   case VK_F2: return KEY_F2;
   case VK_F3: return KEY_F3;
   case VK_F4: return KEY_F4;
   case VK_F5: return KEY_F5;
   case VK_F6: return KEY_F6;
   case VK_F7: return KEY_F7;
   case VK_F8: return KEY_F8;
   case VK_F9: return KEY_F9;
   case VK_F10: return KEY_F10;
   case VK_F11: return KEY_F11;
   case VK_F12: return KEY_F12;
   case VK_OEM_1: return KEY_SEMICOLON; // ';:' for US
   case VK_OEM_PLUS: return KEY_EQUAL_SIGN; // '+' any country
   case VK_OEM_COMMA: return  KEY_COMMA; // ',' any country
   case VK_OEM_MINUS: return KEY_MINUS_SIGN; // '-' any country
   case VK_OEM_PERIOD: return KEY_PERIOD; // '.' any country
   case VK_OEM_2: return KEY_SLASH; // '/?' for US
   case VK_OEM_3: return KEY_GRAVE_ACCENT; // '`~' for US
   case VK_OEM_4: return KEY_LEFT_BRACKET; //  '[{' for US
   case VK_OEM_5: return KEY_BACKSLASH; //  '\|' for US
   case VK_OEM_6: return KEY_RIGHT_BRACKET; //  ']}' for US
   case VK_OEM_7: return KEY_SINGLE_QUOTE; //  ''"' for US
   }

   // key was not recognized. mark as invalid
   return KEY_INVALID;
}

key_types emst_main::apply_key_modifiers_impl(key_types i_key_id) const
{
   if (i_key_id == KEY_INVALID)
   {
      return KEY_INVALID;
   }

   bool num_lock_active = false;
   bool shift_held = ((mod_keys_down & shift_key_down) != 0);

   if (i_key_id >= KEY_0 && i_key_id <= KEY_9)
   {
      if (shift_held)
      {
         int diff = i_key_id - KEY_0;

         switch (diff)
         {
         case 0: return KEY_RIGHT_PARENTHESIS;
         case 1: return KEY_EXCLAMATION;
         case 2: return KEY_AT_SYMBOL;
         case 3: return KEY_NUMBER_SIGN;
         case 4: return KEY_DOLLAR_SIGN;
         case 5: return KEY_PERCENT_SIGN;
         case 6: return KEY_CIRCUMFLEX;
         case 7: return KEY_AMPERSAND;
         case 8: return KEY_ASTERISK;
         case 9: return KEY_LEFT_PARENTHESIS;
         }
      }
      else
      {
         return i_key_id;
      }
   }
   else if (i_key_id >= KEY_A && i_key_id <= KEY_Z)
   {
      if (shift_held)
      {
         int diff = i_key_id - KEY_A;

         return key_types(KEY_A_UPPER_CASE + diff);
      }
      else
      {
         return i_key_id;
      }
   }

   switch (i_key_id)
   {
   case KEY_NUM0: return (num_lock_active) ? KEY_0 : KEY_INSERT;
   case KEY_NUM1: return (num_lock_active) ? KEY_1 : KEY_END;
   case KEY_NUM2: return (num_lock_active) ? KEY_2 : KEY_DOWN;
   case KEY_NUM3: return (num_lock_active) ? KEY_3 : KEY_PAGE_DOWN;
   case KEY_NUM4: return (num_lock_active) ? KEY_4 : KEY_LEFT;
   case KEY_NUM5: return (num_lock_active) ? KEY_5 : KEY_ENTER;
   case KEY_NUM6: return (num_lock_active) ? KEY_6 : KEY_RIGHT;
   case KEY_NUM7: return (num_lock_active) ? KEY_7 : KEY_HOME;
   case KEY_NUM8: return (num_lock_active) ? KEY_8 : KEY_UP;
   case KEY_NUM9: return (num_lock_active) ? KEY_9 : KEY_PAGE_UP;
   case KEY_NUM_MULTIPLY: return (num_lock_active) ? KEY_ASTERISK : KEY_ASTERISK;
   case KEY_NUM_ADD: return (num_lock_active) ? KEY_PLUS_SIGN : KEY_PLUS_SIGN;
   case KEY_NUM_SUBTRACT: return (num_lock_active) ? KEY_MINUS_SIGN : KEY_MINUS_SIGN;
   case KEY_NUM_DECIMAL: return (num_lock_active) ? KEY_PERIOD : KEY_DEL;
   case KEY_NUM_DIVIDE: return (num_lock_active) ? KEY_SLASH : KEY_SLASH;
   case KEY_SEMICOLON: return (shift_held) ? KEY_COLON : KEY_SEMICOLON; // ';:' for US
   case KEY_EQUAL_SIGN: return (shift_held) ? KEY_PLUS_SIGN : KEY_EQUAL_SIGN; // '+' any country
   case KEY_COMMA: return (shift_held) ? KEY_LESS_THAN_SIGN : KEY_COMMA; // ',' any country
   case KEY_MINUS_SIGN: return (shift_held) ? KEY_UNDERSCORE : KEY_MINUS_SIGN; // '-' any country
   case KEY_PERIOD: return (shift_held) ? KEY_GREATER_THAN_SIGN : KEY_PERIOD; // '.' any country
   case KEY_SLASH: return (shift_held) ? KEY_QUESTION_MARK : KEY_SLASH; // '/?' for US
   case KEY_GRAVE_ACCENT: return (shift_held) ? KEY_TILDE_SIGN : KEY_GRAVE_ACCENT; // '`~' for US
   case KEY_LEFT_BRACKET: return (shift_held) ? KEY_LEFT_BRACE : KEY_LEFT_BRACKET; //  '[{' for US
   case KEY_BACKSLASH: return (shift_held) ? KEY_VERTICAL_BAR : KEY_BACKSLASH; //  '\|' for US
   case KEY_RIGHT_BRACKET: return (shift_held) ? KEY_RIGHT_BRACE : KEY_RIGHT_BRACKET; //  ']}' for US
   case KEY_SINGLE_QUOTE: return (shift_held) ? KEY_DOUBLE_QUOTE : KEY_SINGLE_QUOTE; //  ''"' for US
   }

   return i_key_id;
}

// screen metrix
std::pair<uint32, uint32> emst_main::get_screen_res_px() const { return screen_res; }
float emst_main::get_avg_screen_dpi() const { return avg_screen_dpi; }
std::pair<float, float> emst_main::get_screen_dpi() const { return screen_dpi; }
std::pair<float, float> emst_main::get_screen_dim_inch() const { return screen_dim_inch; }
float emst_main::get_avg_screen_dpcm() const { return avg_screen_dpcm; }
std::pair<float, float> emst_main::get_screen_dpcm() const { return screen_dpcm; }
std::pair<float, float> emst_main::get_screen_dim_cm() const { return screen_dim_cm; }

void emst_main::write_text(const char* text)const
{
   printf("%s", text);
}

void emst_main::write_text_nl(const char* text)const
{
   write_text(text);
   write_text("\n");
}

void emst_main::write_text(const wchar_t* text)const
{
   printf("wstring not supported");
}

void emst_main::write_text_nl(const wchar_t* text)const
{
   write_text(text);
   write_text(L"\n");
}

void emst_main::write_text_v(const char* iformat, ...)const
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, iformat);
   vsnprintf(dest, 1024 * 16 - 1, iformat, arg_ptr);
   va_end(arg_ptr);
   printf("%s", dest);
}

std::string emst_main::get_writable_path()const
{
   return "";
}

std::string emst_main::get_timezone_id()const
{
   return "Europe/Bucharest";
}

umf_list emst_main::get_directory_listing(const std::string& i_directory, umf_list i_plist, bool is_recursive)
{
   if (i_plist->find(pfm::filesystem::res_idx_name) == i_plist->end())
   {
      umf_r& list = *i_plist;
      mws_sp<pfm_file> index_txt = pfm_file::get_inst(std::make_shared<emst_file_impl>(pfm::filesystem::res_idx_name, i_directory));
      std::string path = index_txt->get_full_path();
      std::ifstream infile(path);
      //mws_println("i_directory %s file size %d path %s", i_directory.c_str(), index_txt->length(), path.c_str());

      if (infile.is_open())
      {
         std::string line;

         while (std::getline(infile, line))
         {
            // trim the new line at the end
            line = mws_str::rtrim(line);
            std::string filename = mws_util::path::get_filename_from_path(line);
            std::string dir = mws_util::path::get_directory_from_path(line);

            if (!dir.empty())
            {
               dir = i_directory + "/" + dir;
            }
            else
            {
               dir = i_directory;
            }

            list[filename] = pfm_file::get_inst(std::make_shared<emst_file_impl>(filename, dir));
         }

         list[pfm::filesystem::res_idx_name] = index_txt;
         infile.close();
      }
   }

   return i_plist;
}

bool emst_main::is_full_screen_mode()
{
   return true;
}

void emst_main::set_full_screen_mode(bool ienabled)
{
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

emst_main::emst_main() {}

EM_BOOL emst_key_down(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   key_types key_id = pfm_main::gi()->translate_key(e->keyCode);

   switch (key_id)
   {
   case KEY_SHIFT: mod_keys_down |= shift_key_down; break;
   case KEY_CONTROL: mod_keys_down |= ctrl_key_down; break;
   case KEY_ALT: mod_keys_down |= alt_key_down; break;
   }

   mws_mod_ctrl::inst()->key_action(KEY_PRESS, key_id);

   switch (key_id)
   {
   case KEY_TAB:
   case KEY_BACKSPACE:
   case KEY_ENTER:
   case KEY_SLASH:
   case KEY_SINGLE_QUOTE:
   case KEY_NUM_DIVIDE:
      return true;
   }

   return false;
}

EM_BOOL emst_key_up(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   key_types key_id = pfm_main::gi()->translate_key(e->keyCode);

   switch (key_id)
   {
   case KEY_SHIFT: mod_keys_down &= ~shift_key_down; break;
   case KEY_CONTROL: mod_keys_down &= ~ctrl_key_down; break;
   case KEY_ALT: mod_keys_down &= ~alt_key_down; break;
   }

   mws_mod_ctrl::inst()->key_action(KEY_RELEASE, key_id);

   return false;
}

EM_BOOL emst_key_press(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   return false;
}

EM_BOOL emst_mouse_down(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_began;

   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_up(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_ended;

   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_move(int event_type, const EmscriptenMouseEvent* e, void* user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = (float)e->canvasX;
   te.y = (float)e->canvasY;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::touch_moved;

   mws_mod_ctrl::inst()->pointer_action(pfm_te);

   return true;
}

EM_BOOL emst_mouse_wheel(int event_type, const EmscriptenWheelEvent* e, void* user_data)
{
   auto pfm_te = mws_ptr_evt_base::nwi();
   mws_ptr_evt_base::touch_point& te = pfm_te->points[0];

   te.identifier = 0;
   te.is_changed = true;
   te.x = 0.f;
   te.y = 0.f;
   pfm_te->time = pfm::time::get_time_millis();
   pfm_te->touch_count = 1;
   pfm_te->type = mws_ptr_evt_base::mouse_wheel;
   pfm_te->mouse_wheel_delta = float(e->deltaY * -0.09f);

   mws_mod_ctrl::inst()->pointer_action(pfm_te);
   //trx("mouse wheel %1% %2% %3%") % wheel_delta % pointer_coord.x % pointer_coord.y;

   return true;
}

EM_BOOL emst_touch(int event_type, const EmscriptenTouchEvent* e, void* user_data)
{
   return true;
}

void emst_main::setup_callbacks()
{
   emscripten_set_keydown_callback(0, this, true, emst_key_down);
   emscripten_set_keyup_callback(0, this, true, emst_key_up);
   emscripten_set_keypress_callback(0, this, true, emst_key_press);
   emscripten_set_mousedown_callback("#canvas", this, true, emst_mouse_down);
   emscripten_set_mouseup_callback("#canvas", this, true, emst_mouse_up);
   emscripten_set_mousemove_callback("#canvas", this, true, emst_mouse_move);
   emscripten_set_wheel_callback("#canvas", this, false, emst_mouse_wheel);
   emscripten_set_touchstart_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchend_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchmove_callback("#canvas", this, true, emst_touch);
   emscripten_set_touchcancel_callback("#canvas", this, true, emst_touch);
   //emscripten_set_devicemotion_callback(this, true, emst_device_motion);
   //emscripten_set_deviceorientation_callback(this, true, emst_device_orientation);
}


static inline const char* emscripten_event_type_to_string(int event_type)
{
   const char* events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
     "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
     "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
     "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };
   ++event_type;
   if (event_type < 0) event_type = 0;
   if (event_type >= sizeof(events) / sizeof(events[0])) event_type = sizeof(events) / sizeof(events[0]) - 1;
   return events[event_type];
}

const char* emscripten_result_to_string(EMSCRIPTEN_RESULT result)
{
   if (result == EMSCRIPTEN_RESULT_SUCCESS) return "EMSCRIPTEN_RESULT_SUCCESS";
   if (result == EMSCRIPTEN_RESULT_DEFERRED) return "EMSCRIPTEN_RESULT_DEFERRED";
   if (result == EMSCRIPTEN_RESULT_NOT_SUPPORTED) return "EMSCRIPTEN_RESULT_NOT_SUPPORTED";
   if (result == EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED) return "EMSCRIPTEN_RESULT_FAILED_NOT_DEFERRED";
   if (result == EMSCRIPTEN_RESULT_INVALID_TARGET) return "EMSCRIPTEN_RESULT_INVALID_TARGET";
   if (result == EMSCRIPTEN_RESULT_UNKNOWN_TARGET) return "EMSCRIPTEN_RESULT_UNKNOWN_TARGET";
   if (result == EMSCRIPTEN_RESULT_INVALID_PARAM) return "EMSCRIPTEN_RESULT_INVALID_PARAM";
   if (result == EMSCRIPTEN_RESULT_FAILED) return "EMSCRIPTEN_RESULT_FAILED";
   if (result == EMSCRIPTEN_RESULT_NO_DATA) return "EMSCRIPTEN_RESULT_NO_DATA";
   return "Unknown EMSCRIPTEN_RESULT!";
}

#define TEST_RESULT(x) if (ret != EMSCRIPTEN_RESULT_SUCCESS) printf("%s returned %s.\n", #x, emscripten_result_to_string(ret));

void mws_get_canvas_size(int* width, int* height, int* is_full_screen)
{
   EMSCRIPTEN_RESULT r = emscripten_get_canvas_element_size("#canvas", width, height);
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

   *is_full_screen = e.isFullscreen;
}

EM_BOOL on_canvassize_changed(int event_type, const void* reserved, void* user_data)
{
   int width, height, fs;
   mws_get_canvas_size(&width, &height, &fs);
   double css_width, css_height;
   emscripten_get_element_css_size(0, &css_width, &css_height);
   height = (height > 0) ? height : 1;
   emst_main::get_instance()->on_resize(width, height);

   printf("Canvas resized: WebGL RTT size: %dx%d, canvas CSS size: %02gx%02g\n", width, height, css_width, css_height);

   return 0;
}

void requestFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = scaleMode;
   s.canvasResolutionScaleMode = canvasResolutionScaleMode;
   s.filteringMode = filteringMode;
   s.canvasResizedCallback = on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_request_fullscreen_strategy(0, 1, &s);
   TEST_RESULT(requestFullscreen);
}

void enterSoftFullscreen(int scaleMode, int canvasResolutionScaleMode, int filteringMode)
{
   EmscriptenFullscreenStrategy s;
   memset(&s, 0, sizeof(s));
   s.scaleMode = scaleMode;
   s.canvasResolutionScaleMode = canvasResolutionScaleMode;
   s.filteringMode = filteringMode;
   s.canvasResizedCallback = on_canvassize_changed;
   EMSCRIPTEN_RESULT ret = emscripten_enter_soft_fullscreen(0, &s);
   TEST_RESULT(enterSoftFullscreen);
}

void run_step()
{
   emst_main::get_instance()->run();
}

// implement this gl function missing in emscripten
GL_APICALL void GL_APIENTRY glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
   EM_ASM_(
      {
         Module.ctx.getBufferSubData(Module.ctx.PIXEL_PACK_BUFFER, 0, HEAPU8.subarray($0, $0 + $1));
      }, data, size);
}

GL_APICALL void* GL_APIENTRY glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access) { mws_throw mws_exception("glMapBufferRange n/a"); return nullptr; }
GL_APICALL GLboolean GL_APIENTRY glUnmapBuffer(GLenum target) { mws_throw mws_exception("glUnmapBuffer n/a"); return (GLboolean)false; }


int main()
{
   enterSoftFullscreen(EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH, EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF, EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT);

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

   emst_main::get_instance()->init();
   emst_main::get_instance()->start();

   emscripten_set_main_loop(run_step, 0, 0);

   return 0;
}
