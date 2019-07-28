#include "main.hxx"

#include "pfm-def.h"
#include "pfm-gl.h"
#include "pfm.hxx"
#include "mws-mod-ctrl.hxx"
#include "input/input-ctrl.hxx"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include <fstream>


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
		if(file)
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

   setup_callbacks();
   mws_mod_ctrl::inst()->pre_init_app();
   mws_mod_ctrl::inst()->set_gfx_available(true);
   mws_mod_ctrl::inst()->init_app();

   is_started = true;
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

float emst_main::get_screen_dpi()const
{
   return 127;
}

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
	if (i_plist->find("index.txt") == i_plist->end())
	{
		umf_r& list = *i_plist;
		mws_sp<pfm_file> index_txt = pfm_file::get_inst(std::make_shared<emst_file_impl>("index.txt", i_directory));
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
			  
			  if(!dir.empty())
			  {
				  dir = i_directory + "/" + dir;
			  }
			  else
			  {
				  dir = i_directory;
			  }
			  
			  list[filename] = pfm_file::get_inst(std::make_shared<emst_file_impl>(filename, dir));
		   }

		   list["index.txt"] = index_txt;
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

emst_main::emst_main()
{
   is_started = false;
}

EM_BOOL emst_key_down(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   key_types key = emst_main::get_instance()->map_key(e->keyCode);

   if (key != KEY_INVALID)
   {
      mws_mod_ctrl::inst()->key_action(KEY_PRESS, key);

      switch (key)
      {
      case KEY_TAB:
      case KEY_BACKSPACE:
      case KEY_ENTER:
         return true;

      default:
         return false;
      }
   }

   return false;
}

EM_BOOL emst_key_up(int event_type, const EmscriptenKeyboardEvent* e, void* user_data)
{
   key_types key = emst_main::get_instance()->map_key(e->keyCode);

   if (key != KEY_INVALID)
   {
      mws_mod_ctrl::inst()->key_action(KEY_RELEASE, key);
   }

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
   te.is_changed = false;
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
   te.is_changed = false;
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
   te.is_changed = false;
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
   te.is_changed = false;
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
   setup_key_table();

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

void emst_main::setup_key_table()
{
   for (int k = 0; k < MAX_KEY_COUNT; k++)
   {
      key_table[k] = KEY_INVALID;
   }

   key_table[8] = KEY_BACKSPACE;
   key_table[9] = KEY_TAB;
   key_table[13] = KEY_ENTER;
   key_table[16] = KEY_SHIFT;
   key_table[17] = KEY_CONTROL;
   key_table[18] = KEY_ALT;
   key_table[27] = KEY_ESCAPE;
   key_table[32] = KEY_SPACE;
   key_table[33] = KEY_PAGE_UP;
   key_table[34] = KEY_PAGE_DOWN;
   key_table[35] = KEY_END;
   key_table[36] = KEY_HOME;
   key_table[37] = KEY_LEFT;
   key_table[38] = KEY_UP;
   key_table[39] = KEY_RIGHT;
   key_table[40] = KEY_DOWN;
   key_table[45] = KEY_INSERT;
   key_table[46] = KEY_DELETE;
   key_table[48] = KEY_0;
   key_table[49] = KEY_1;
   key_table[50] = KEY_2;
   key_table[51] = KEY_3;
   key_table[52] = KEY_4;
   key_table[53] = KEY_5;
   key_table[54] = KEY_6;
   key_table[55] = KEY_7;
   key_table[56] = KEY_8;
   key_table[57] = KEY_9;
   key_table[59] = KEY_SEMICOLON;
   key_table[64] = KEY_EQUAL_SIGN;
   key_table[65] = KEY_A;
   key_table[66] = KEY_B;
   key_table[67] = KEY_C;
   key_table[68] = KEY_D;
   key_table[69] = KEY_E;
   key_table[70] = KEY_F;
   key_table[71] = KEY_G;
   key_table[72] = KEY_H;
   key_table[73] = KEY_I;
   key_table[74] = KEY_J;
   key_table[75] = KEY_K;
   key_table[76] = KEY_L;
   key_table[77] = KEY_M;
   key_table[78] = KEY_N;
   key_table[79] = KEY_O;
   key_table[80] = KEY_P;
   key_table[81] = KEY_Q;
   key_table[82] = KEY_R;
   key_table[83] = KEY_S;
   key_table[84] = KEY_T;
   key_table[85] = KEY_U;
   key_table[86] = KEY_V;
   key_table[87] = KEY_W;
   key_table[88] = KEY_X;
   key_table[89] = KEY_Y;
   key_table[90] = KEY_Z;
   key_table[96] = KEY_NUM0;
   key_table[97] = KEY_NUM1;
   key_table[98] = KEY_NUM2;
   key_table[99] = KEY_NUM3;
   key_table[100] = KEY_NUM4;
   key_table[101] = KEY_NUM5;
   key_table[102] = KEY_NUM6;
   key_table[103] = KEY_NUM7;
   key_table[104] = KEY_NUM8;
   key_table[105] = KEY_NUM9;
   key_table[106] = KEY_NUM_MULTIPLY;
   key_table[107] = KEY_NUM_ADD;
   key_table[109] = KEY_NUM_SUBTRACT;
   key_table[110] = KEY_NUM_DECIMAL;
   key_table[111] = KEY_NUM_DIVIDE;
   key_table[112] = KEY_F1;
   key_table[113] = KEY_F2;
   key_table[114] = KEY_F3;
   key_table[115] = KEY_F4;
   key_table[116] = KEY_F5;
   key_table[117] = KEY_F6;
   key_table[118] = KEY_F7;
   key_table[119] = KEY_F8;
   key_table[120] = KEY_F9;
   key_table[121] = KEY_F10;
   key_table[122] = KEY_F11;
   key_table[123] = KEY_F12;
   key_table[144] = KEY_NUM_LOCK;
   key_table[144] = KEY_NUM_LOCK;
   key_table[145] = KEY_SCROLL_LOCK;
   key_table[173] = KEY_MINUS_SIGN;
   key_table[186] = KEY_SEMICOLON;
   key_table[187] = KEY_EQUAL_SIGN;
   key_table[188] = KEY_COMMA;
   key_table[189] = KEY_MINUS_SIGN;
   key_table[190] = KEY_PERIOD;
   key_table[191] = KEY_SLASH;
   key_table[192] = KEY_GRAVE_ACCENT;
   key_table[219] = KEY_LEFT_BRACKET;
   key_table[220] = KEY_BACKSLASH;
   key_table[221] = KEY_RIGHT_BRACKET;
   key_table[222] = KEY_TILDE_SIGN;
}

key_types emst_main::map_key(unsigned long i_key_code) const
{
   if (i_key_code < MAX_KEY_COUNT)
   {
      return key_table[i_key_code];
   }

   return KEY_INVALID;
}


void report_result(int result)
{
   if (result == 0) {
      printf("Test successful!\n");
   }
   else {
      printf("Test failed!\n");
   }
#ifdef REPORT_RESULT
   REPORT_RESULT(result);
#endif
}

static inline const char *emscripten_event_type_to_string(int event_type) {
   const char *events[] = { "(invalid)", "(none)", "keypress", "keydown", "keyup", "click", "mousedown", "mouseup", "dblclick", "mousemove", "wheel", "resize",
     "scroll", "blur", "focus", "focusin", "focusout", "deviceorientation", "devicemotion", "orientationchange", "fullscreenchange", "pointerlockchange",
     "visibilitychange", "touchstart", "touchend", "touchmove", "touchcancel", "gamepadconnected", "gamepaddisconnected", "beforeunload",
     "batterychargingchange", "batterylevelchange", "webglcontextlost", "webglcontextrestored", "(invalid)" };
   ++event_type;
   if (event_type < 0) event_type = 0;
   if (event_type >= sizeof(events) / sizeof(events[0])) event_type = sizeof(events) / sizeof(events[0]) - 1;
   return events[event_type];
}

const char *emscripten_result_to_string(EMSCRIPTEN_RESULT result) {
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
   mws_mod_ctrl::inst()->resize_app(width, height);

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
