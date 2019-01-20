#pragma once
// common c/c++ code.


#ifdef __cplusplus
#include <memory>

using std::enable_shared_from_this;
using std::shared_ptr;
using std::static_pointer_cast;
using std::weak_ptr;
template <typename T> using mws_sp = std::shared_ptr<T>;
template <typename T> using mws_wp = std::weak_ptr<T>;

extern "C"
{
#endif 


#define mws_stringify(s) #s

#if !defined NDEBUG || defined _DEBUG

#define MWS_DEBUG_BUILD

#define mws_signal_error(i_message) mws_signal_error_impl(__FILE__, __LINE__, i_message)
#define mws_assert(i_condition) mws_assert_impl(__FILE__, __LINE__, i_condition)

#define MWS_REPORT_GL_ERRORS
#define mws_report_gfx_errs() mws_report_gfx_errs_impl(__FILE__, __LINE__)
#define mws_nl() mws_nl_impl()
#define mws_print(i_format, ...) mws_print_impl(i_format, ##__VA_ARGS__)
#define mws_println(i_format, ...) mws_print_impl(i_format, ##__VA_ARGS__); mws_nl_impl()

#else

#define MWS_RELEASE_BUILD
#define MWS_PRODUCTION_BUILD

#define mws_signal_error(i_message)
#define mws_assert(i_condition)

#define mws_report_gfx_errs()
#define mws_print(i_format, ...)

#endif


   // determine compile target platform
   // unicode strings are 16-bit characters on windows, and 8-bit utf-8 characters on others
#if defined ANDROID

#define PLATFORM_ANDROID
#define UNICODE_USING_STD_STRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#elif defined __APPLE__
    
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR

    // iOS simulator
#define PLATFORM_IOS
    
#elif TARGET_OS_IPHONE

    // actual iOS device
#define PLATFORM_IOS
    
#else
#define TARGET_OS_OSX 1
    
    // mac OS X
    // define something for OSX
    
#endif
    
#define UNICODE_USING_STD_STRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#elif defined EMSCRIPTEN

#define PLATFORM_EMSCRIPTEN
#define UNICODE_USING_STD_STRING
//#define MWS_USES_EXCEPTIONS
//#define MWS_USES_RTTI

#elif defined PLATFORM_QT_WINDOWS_PC

#define UNICODE_USING_STD_WSTRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#elif defined WIN32

#define PLATFORM_WINDOWS_PC
#define UNICODE_USING_STD_WSTRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#else

#pragma error

#endif


#ifdef MWS_USES_EXCEPTIONS

#define mws_throw throw
#define mws_try try
#define mws_catch(arg) catch(arg)

#else

#define mws_throw
#define mws_try if(true)
#define mws_catch(arg) if(false)

#endif


#ifdef MWS_USES_RTTI

#define mws_dynamic_cast dynamic_cast
#define mws_dynamic_pointer_cast std::dynamic_pointer_cast

#else

#define mws_dynamic_cast static_cast
#define mws_dynamic_pointer_cast std::static_pointer_cast

#endif


   enum platform_id
   {
      platform_android,
      platform_ios,
      platform_emscripten,
      platform_qt_windows_pc,
      platform_windows_pc,
   };


   enum gfx_type_id
   {
      gfx_type_none,
      gfx_type_opengl,
      gfx_type_opengl_es,
   };


#ifndef __cplusplus
#ifndef bool
    typedef unsigned int bool;
#endif
#endif
   typedef char int8;
   typedef signed char sint8;
   typedef unsigned char uint8;
   typedef short int16;
   typedef signed short sint16;
   typedef unsigned short uint16;
   typedef int int32;
   typedef signed int sint32;
   typedef unsigned int uint32;
   typedef long long int64;
   typedef signed long long sint64;
   typedef unsigned long long uint64;
   typedef float real32;
   typedef double real64;

   // gl types
   typedef int8 gfx_char;
   typedef uint32 gfx_enum;
   typedef uint8 gfx_boolean;
   typedef uint32 gfx_bitfield;
   typedef int8 gfx_byte;
   typedef int16 gfx_short;
   typedef int32 gfx_int;
   typedef int32 gfx_sizei;
   typedef uint8 gfx_ubyte;
   typedef uint16 gfx_ushort;
   typedef uint32 gfx_uint;
   typedef real32 gfx_float;
   typedef real32 gfx_clampf;
   typedef real64 gfx_double;
   typedef real64 gfx_clampd;
   typedef void gfx_void;
   typedef uint32 gfx_indices_type;


#define _USE_MATH_DEFINES

#if defined PLATFORM_WINDOWS_PC && defined MWS_DEBUG_BUILD
   //#define USE_VLD
#endif


   void mws_signal_error_impl(const char* i_file, uint32 i_line, const char* i_message);
   void mws_assert_impl(const char* i_file, uint32 i_line, bool i_condition);
   void mws_report_gfx_errs_impl(const char* i_file, uint32 i_line);
   void mws_nl_impl();
   void mws_print_impl(const char* i_format, ...);


   // input defines

   enum key_actions
   {
      KEY_PRESS,
      KEY_RELEASE,
   };


   enum key_types
   {
      KEY_INVALID,
      KEY_LEFT,
      KEY_UP,
      KEY_RIGHT,
      KEY_DOWN,
      KEY_NUM_LOCK,
      KEY_INSERT,
      KEY_DELETE,
      KEY_BACKSPACE,
      KEY_TAB,
      KEY_PAGE_UP,
      KEY_PAGE_DOWN,
      KEY_SCROLL_LOCK,
      KEY_ENTER,
      KEY_HOME,
      KEY_F1,
      KEY_F2,
      KEY_F3,
      KEY_F4,
      KEY_F5,
      KEY_F6,
      KEY_F7,
      KEY_F8,
      KEY_F9,
      KEY_F10,
      KEY_F11,
      KEY_F12,
      KEY_ESCAPE,
      KEY_SHIFT,
      KEY_CONTROL,
      KEY_ALT,
      KEY_END,

      // begin printable ascii
      KEY_SPACE,
      KEY_EXCLAMATION,
      KEY_DOUBLE_QUOTE,
      KEY_NUMBER_SIGN,
      KEY_DOLLAR_SIGN,
      KEY_PERCENT_SIGN,
      KEY_AMPERSAND,
      KEY_SINGLE_QUOTE,
      KEY_LEFT_PARENTHESIS,
      KEY_RIGHT_PARENTHESIS,
      KEY_ASTERISK,
      KEY_PLUS_SIGN,
      KEY_COMMA,
      KEY_MINUS_SIGN,
      KEY_PERIOD,
      KEY_SLASH,
      KEY_0,
      KEY_1,
      KEY_2,
      KEY_3,
      KEY_4,
      KEY_5,
      KEY_6,
      KEY_7,
      KEY_8,
      KEY_9,
      KEY_COLON,
      KEY_SEMICOLON,
      KEY_LESS_THAN_SIGN,
      KEY_EQUAL_SIGN,
      KEY_GREATER_THAN_SIGN,
      KEY_QUESTION_MARK,
      KEY_AT_SYMBOL,
      KEY_A,
      KEY_B,
      KEY_C,
      KEY_D,
      KEY_E,
      KEY_F,
      KEY_G,
      KEY_H,
      KEY_I,
      KEY_J,
      KEY_K,
      KEY_L,
      KEY_M,
      KEY_N,
      KEY_O,
      KEY_P,
      KEY_Q,
      KEY_R,
      KEY_S,
      KEY_T,
      KEY_U,
      KEY_V,
      KEY_W,
      KEY_X,
      KEY_Y,
      KEY_Z,
      KEY_LEFT_BRACKET,
      KEY_BACKSLASH,
      KEY_RIGHT_BRACKET,
      KEY_CIRCUMFLEX,
      KEY_UNDERSCORE,
      KEY_GRAVE_ACCENT,
      // skip the small caps letters, as checking the 'shift held' flag can tell the difference
      KEY_LEFT_BRACE = 123,
      KEY_VERTICAL_BAR,
      KEY_RIGHT_BRACE,
      KEY_TILDE_SIGN,
      // end printable ascii

      KEY_DEL,
      KEY_NUM0,
      KEY_NUM1,
      KEY_NUM2,
      KEY_NUM3,
      KEY_NUM4,
      KEY_NUM5,
      KEY_NUM6,
      KEY_NUM7,
      KEY_NUM8,
      KEY_NUM9,
      KEY_NUM_MULTIPLY,
      KEY_NUM_ADD,
      KEY_NUM_SUBTRACT,
      KEY_NUM_DECIMAL,
      KEY_NUM_DIVIDE,
      KEY_SELECT,

      KEY_COUNT,
   };


#define DK_UP           KEY_W
#define DK_DOWN         KEY_X
#define DK_LEFT         KEY_A
#define DK_RIGHT        KEY_D
#define DK_DOWN_LEFT    KEY_Z
#define DK_DOWN_RIGHT   KEY_C
#define DK_UP_LEFT      KEY_Q
#define DK_UP_RIGHT     KEY_E


#ifdef __cplusplus
}
#endif 
