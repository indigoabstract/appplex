#pragma once
// common c/c++ code.

// determine compile target platform
// unicode strings are 16-bit characters on windows, and 8-bit utf-8 characters on others
#if defined ANDROID

	#define PLATFORM_ANDROID
	#define UNICODE_USING_STD_STRING

#elif defined __APPLE__ && defined TARGET_OS_IPHONE

	#define PLATFORM_IOS
	#define UNICODE_USING_STD_STRING

#elif defined EMSCRIPTEN

	#define PLATFORM_EMSCRIPTEN
	#define UNICODE_USING_STD_STRING

#elif defined PLATFORM_QT_WINDOWS_PC

	#define UNICODE_USING_STD_WSTRING

#elif defined WIN32

	#define PLATFORM_WINDOWS_PC
	#define UNICODE_USING_STD_WSTRING

#else

	#pragma error

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


typedef signed char int8;
typedef unsigned char uint8;
typedef signed short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;
typedef signed long long int64;
typedef unsigned long long uint64;
typedef float real32;
typedef double real64;

// gl types
typedef char gfx_char;
typedef unsigned int gfx_enum;
typedef unsigned char gfx_boolean;
typedef unsigned int gfx_bitfield;
typedef signed char gfx_byte;
typedef short gfx_short;
typedef int gfx_int;
typedef int gfx_sizei;
typedef unsigned char gfx_ubyte;
typedef unsigned short gfx_ushort;
typedef unsigned int gfx_uint;
typedef float gfx_float;
typedef float gfx_clampf;
typedef double gfx_double;
typedef double gfx_clampd;
typedef void gfx_void;
typedef uint32 gfx_indices_type;


#define _USE_MATH_DEFINES

#if defined PLATFORM_WINDOWS_PC && defined _DEBUG
	//#define USE_VLD
#endif


// input defines
enum pointer_actions
{
	POINTER_PRESS,
	POINTER_RELEASE,
	POINTER_DRAGG,
	POINTER_MOUSE_WHEEL,
};


enum key_actions
{
	KEY_PRESS,
	KEY_RELEASE,
};


enum key_types
{
	KEY_INVALID,
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
	KEY_SHIFT,
	KEY_CONTROL,
	KEY_ALT,
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_ENTER,
	KEY_SELECT,
	KEY_BACK,
	KEY_ESCAPE,
	KEY_SPACE,
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
	KEY_COUNT,
};


#define AK_SELECT		KEY_SELECT
#define AK_UP           KEY_UP
#define AK_DOWN         KEY_DOWN
#define AK_LEFT         KEY_LEFT
#define AK_RIGHT        KEY_RIGHT
#define DK_UP           KEY_W
#define DK_DOWN         KEY_X
#define DK_LEFT         KEY_A
#define DK_RIGHT        KEY_D
#define DK_DOWN_LEFT    KEY_Z
#define DK_DOWN_RIGHT   KEY_C
#define DK_UP_LEFT      KEY_Q
#define DK_UP_RIGHT     KEY_E
