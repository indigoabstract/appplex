#pragma once
// common c/c++ code.


#ifdef __cplusplus
#include <cstdint>
#include <limits>
#include <memory>

using float32 = float;
using float64 = double;
using std::static_pointer_cast;
template <typename T> using mws_sp = std::shared_ptr<T>;
template <typename T> using mws_wp = std::weak_ptr<T>;
template <typename T> using mws_up = std::unique_ptr<T>;

inline const uint32_t mws_u8_max = std::numeric_limits<uint8_t>::max();
inline const uint32_t mws_u16_max = std::numeric_limits<uint16_t>::max();
inline const uint32_t mws_u32_max = std::numeric_limits<uint32_t>::max();
inline const uint64_t mws_u64_max = std::numeric_limits<uint64_t>::max();
#else
#include <stdint.h>

#if !defined bool && !defined false
typedef unsigned int bool;
typedef enum { false = 0, true = 1 }bool_type;
#endif
typedef float float32;
typedef double float64;
#endif

// gl types
typedef char gfx_char;
typedef uint32_t gfx_enum;
typedef uint8_t gfx_boolean;
typedef uint32_t gfx_bitfield;
typedef int8_t gfx_byte;
typedef int16_t gfx_short;
typedef int32_t gfx_int;
typedef int32_t gfx_sizei;
typedef uint8_t gfx_ubyte;
typedef uint16_t gfx_ushort;
typedef uint32_t gfx_uint;
typedef float32 gfx_float;
typedef float32 gfx_clampf;
typedef float64 gfx_double;
typedef float64 gfx_clampd;
typedef void gfx_void;
typedef uint32_t gfx_indices_type;


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
   mws_pfm_linux,
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


#if defined(_MSC_VER)

#define mws_push_disable_all_warnings       __pragma(warning(push, 0))
#define mws_pop_disable_all_warnings        __pragma(warning(pop))
#define mws_push_disable_warning            __pragma(warning(push))
#define mws_pop_disable_warning             __pragma(warning(pop)) 
#define mws_disable_warning(warning_number) __pragma(warning(disable : warning_number))

#elif defined(__GNUC__) || defined(__clang__)

#define mws_push_disable_all_warnings
#define mws_pop_disable_all_warnings
#define mws_push_disable_warning
#define mws_pop_disable_warning
#define mws_disable_warning(warning_number)

#else

#define mws_push_disable_all_warnings
#define mws_pop_disable_all_warnings
#define mws_push_disable_warning
#define mws_pop_disable_warning
#define mws_disable_warning(warning_number)

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

#elif defined __linux__

#define MWS_PFM_LINUX
#define MWS_POSIX_API
#define MWS_UNICODE_USING_STD_STRING
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


   void mws_signal_error_impl(const char* i_file, uint32_t i_line, const char* i_message);
   void mws_assert_impl(const char* i_file, uint32_t i_line, bool i_condition);
   void mws_report_gfx_errs_impl(const char* i_file, uint32_t i_line);
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
