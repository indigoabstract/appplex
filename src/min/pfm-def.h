#pragma once
// common c/c++ code.


#ifdef __cplusplus
#include <cstdint>
#include <memory>

using int8 = int8_t;
using sint8 = int8_t;
using uint8 = uint8_t;
using int16 = int16_t;
using sint16 = int16_t;
using uint16 = uint16_t;
using int32 = int32_t;
using sint32 = int32_t;
using uint32 = uint32_t;
using int64 = int64_t;
using sint64 = int64_t;
using uint64 = uint64_t;
using real32 = float;
using real64 = double;

using std::static_pointer_cast;
template <typename T> using mws_sp = std::shared_ptr<T>;
template <typename T> using mws_wp = std::weak_ptr<T>;
template <typename T> using mws_up = std::unique_ptr<T>;
#else
#if !defined bool && !defined false
typedef unsigned int bool;
typedef enum { false = 0, true = 1 }bool_type;
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
#endif

// gl types
typedef char gfx_char;
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


enum mws_gfx_type
{
   mws_gfx_none,
   mws_gfx_opengl,
   mws_gfx_opengl_es,
};


enum mws_pfm_id
{
   mws_pfm_android,
   mws_pfm_ios,
   mws_pfm_emscripten,
   mws_pfm_windows_pc,
};


#ifdef __cplusplus
extern "C"
{
#endif 


#define mws_stringify(s) #s
#define mws_println_all_builds(i_format, ...) mws_print_impl(i_format, ##__VA_ARGS__); mws_nl_impl()

#if !defined NDEBUG || defined _DEBUG

#define MWS_DEBUG_BUILD
#define mws_debug_enabled true

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
#define mws_debug_enabled false

#define mws_signal_error(i_message)
#define mws_assert(i_condition)

#define mws_report_gfx_errs()
#define mws_print(i_format, ...)
#define mws_println(i_format, ...)

#endif


   // determine compile target platform
   // unicode strings are 16-bit characters on windows, and 8-bit utf-8 characters on others
#if defined ANDROID

#define MWS_PFM_ANDROID
#define MWS_POSIX_API
#define MWS_UNICODE_USING_STD_STRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#elif defined __APPLE__
    
#define MWS_POSIX_API
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE && TARGET_IPHONE_SIMULATOR

    // iOS simulator
#define MWS_PFM_IOS
    
#elif TARGET_OS_IPHONE

    // actual iOS device
#define MWS_PFM_IOS
    
#else
#define TARGET_OS_OSX 1
    
    // mac OS X
    // define something for OSX
    
#endif
    
#define MWS_UNICODE_USING_STD_STRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI

#elif defined EMSCRIPTEN

#define MWS_PFM_EMSCRIPTEN
#define MWS_UNICODE_USING_STD_STRING
//#define MWS_USES_EXCEPTIONS
//#define MWS_USES_RTTI

#elif defined WIN32

#define MWS_PFM_WINDOWS_PC
#define MWS_WINDOWS_API
#define MWS_UNICODE_USING_STD_WSTRING
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


#define _USE_MATH_DEFINES

#if defined MWS_PFM_WINDOWS_PC && defined MWS_DEBUG_BUILD
   //#define USE_VLD
#endif


   void mws_signal_error_impl(const char* i_file, uint32 i_line, const char* i_message);
   void mws_assert_impl(const char* i_file, uint32 i_line, bool i_condition);
   void mws_report_gfx_errs_impl(const char* i_file, uint32 i_line);
   void mws_nl_impl();
   void mws_print_impl(const char* i_format, ...);


#ifdef __cplusplus
}

#include <string>

#if defined MWS_UNICODE_USING_STD_STRING

using unicode_string = std::string;
using unicode_char = char;

#elif defined MWS_UNICODE_USING_STD_WSTRING

using unicode_string = std::wstring;
using unicode_char = wchar_t;

#endif

#endif 
