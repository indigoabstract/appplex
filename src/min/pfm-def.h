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


enum mws_key_actions
{
   mws_key_press,
   mws_key_release,
};


enum mws_key_types
{
   mws_key_invalid,
   mws_key_left,
   mws_key_up,
   mws_key_right,
   mws_key_down,
   mws_key_num_lock,
   mws_key_insert,
   mws_key_delete,
   mws_key_backspace,
   mws_key_tab,
   mws_key_page_up,
   mws_key_page_down,
   mws_key_scroll_lock,
   mws_key_enter,
   mws_key_home,
   mws_key_f1,
   mws_key_f2,
   mws_key_f3,
   mws_key_f4,
   mws_key_f5,
   mws_key_f6,
   mws_key_f7,
   mws_key_f8,
   mws_key_f9,
   mws_key_f10,
   mws_key_f11,
   mws_key_f12,
   mws_key_escape,
   mws_key_shift,
   mws_key_control,
   mws_key_alt,
   mws_key_end,

   // begin printable ascii
   mws_key_space,
   mws_key_exclamation,
   mws_key_double_quote,
   mws_key_number_sign,
   mws_key_dollar_sign,
   mws_key_percent_sign,
   mws_key_ampersand,
   mws_key_single_quote,
   mws_key_left_parenthesis,
   mws_key_right_parenthesis,
   mws_key_asterisk,
   mws_key_plus_sign,
   mws_key_comma,
   mws_key_minus_sign,
   mws_key_period,
   mws_key_slash,
   mws_key_0,
   mws_key_1,
   mws_key_2,
   mws_key_3,
   mws_key_4,
   mws_key_5,
   mws_key_6,
   mws_key_7,
   mws_key_8,
   mws_key_9,
   mws_key_colon,
   mws_key_semicolon,
   mws_key_less_than_sign,
   mws_key_equal_sign,
   mws_key_greater_than_sign,
   mws_key_question_mark,
   mws_key_at_symbol,
   mws_key_a_upper_case,
   mws_key_b_upper_case,
   mws_key_c_upper_case,
   mws_key_d_upper_case,
   mws_key_e_upper_case,
   mws_key_f_upper_case,
   mws_key_g_upper_case,
   mws_key_h_upper_case,
   mws_key_i_upper_case,
   mws_key_j_upper_case,
   mws_key_k_upper_case,
   mws_key_l_upper_case,
   mws_key_m_upper_case,
   mws_key_n_upper_case,
   mws_key_o_upper_case,
   mws_key_p_upper_case,
   mws_key_q_upper_case,
   mws_key_r_upper_case,
   mws_key_s_upper_case,
   mws_key_t_upper_case,
   mws_key_u_upper_case,
   mws_key_v_upper_case,
   mws_key_w_upper_case,
   mws_key_x_upper_case,
   mws_key_y_upper_case,
   mws_key_z_upper_case,
   mws_key_left_bracket,
   mws_key_backslash,
   mws_key_right_bracket,
   mws_key_circumflex,
   mws_key_underscore,
   mws_key_grave_accent,
   // add small caps letters. simplifies handling of keys/characters
   mws_key_a,
   mws_key_b,
   mws_key_c,
   mws_key_d,
   mws_key_e,
   mws_key_f,
   mws_key_g,
   mws_key_h,
   mws_key_i,
   mws_key_j,
   mws_key_k,
   mws_key_l,
   mws_key_m,
   mws_key_n,
   mws_key_o,
   mws_key_p,
   mws_key_q,
   mws_key_r,
   mws_key_s,
   mws_key_t,
   mws_key_u,
   mws_key_v,
   mws_key_w,
   mws_key_x,
   mws_key_y,
   mws_key_z,
   mws_key_left_brace,
   mws_key_vertical_bar,
   mws_key_right_brace,
   mws_key_tilde_sign,
   // end printable ascii

   mws_key_del,
   mws_key_num0,
   mws_key_num1,
   mws_key_num2,
   mws_key_num3,
   mws_key_num4,
   mws_key_num5,
   mws_key_num6,
   mws_key_num7,
   mws_key_num8,
   mws_key_num9,
   mws_key_num_multiply,
   mws_key_num_add,
   mws_key_num_subtract,
   mws_key_num_decimal,
   mws_key_num_divide,
   mws_key_select,

   mws_key_count,
};


enum mws_dir_types
{
   mws_dir_none,
   mws_dir_left,
   mws_dir_up_left,
   mws_dir_up,
   mws_dir_up_right,
   mws_dir_right,
   mws_dir_down_right,
   mws_dir_down,
   mws_dir_down_left,
};


enum mws_alpha_dir_types
{
   mws_dk_up = mws_key_w,
   mws_dk_down = mws_key_x,
   mws_dk_left = mws_key_a,
   mws_dk_right = mws_key_d,
   mws_dk_down_left = mws_key_z,
   mws_dk_down_right = mws_key_c,
   mws_dk_up_left = mws_key_q,
   mws_dk_up_right = mws_key_e,
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
#define MWS_CXX_VERSION 17

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
#define MWS_CXX_VERSION 17

#elif defined EMSCRIPTEN

#define MWS_PFM_EMSCRIPTEN
#define MWS_UNICODE_USING_STD_STRING
//#define MWS_USES_EXCEPTIONS
//#define MWS_USES_RTTI
#define MWS_CXX_VERSION 17

#elif defined WIN32

#define MWS_PFM_WINDOWS_PC
#define MWS_WINDOWS_API
#define MWS_UNICODE_USING_STD_WSTRING
#define MWS_USES_EXCEPTIONS
#define MWS_USES_RTTI
#define MWS_CXX_VERSION 17

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
