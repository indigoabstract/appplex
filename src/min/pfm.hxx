#pragma once

#include "pfm-def.h"
#include "input/input-def.hxx"
#include <cstdio>
#include <vector>
#include <unordered_map>
#include <utility>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


#if defined MWS_PFM_WINDOWS_PC

//void* operator new(std::size_t i_size, const std::nothrow_t& nothrow_value);
//void operator delete(void* iptr, const std::nothrow_t& nothrow_constant);
//
//void* operator new[](std::size_t i_size, const std::nothrow_t& nothrow_value);
//void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant);

#endif


#if defined MWS_UNICODE_USING_STD_STRING

#define untr(arg)	 arg
#define utrn    trn
#define utrx	 trx
#define utrc	 trc
#define utrs	 trs

#elif defined MWS_UNICODE_USING_STD_WSTRING

#define untr(arg)	 L##arg
#define utrn	 wtrn
#define utrx	 wtrx
#define utrc	 wtrc
#define utrs	 wtrs

#endif


class mws_mod;
class mws_mod_ctrl;
class mws_file;
class mws_path;
class mws_file_impl;
using mws_file_map = std::unordered_map<std::string, mws_sp<mws_file>>;


// represents either an internal path(inside the app's bundled resources) or an external path(outside of the bundled resources)
class mws_path final
{
public:
   // an internal file(or path) refers to a file inside the resource directory. everything else is considered an external file(or path)
   // an internal path must only have a filename component, but not a directory component,
   // as every internal filename is unique so there is no possibility of name collision
   mws_path();
   mws_path(const char* i_path);
   mws_path(const std::string& i_path);
   mws_path(const mws_path& i_path);
   mws_path& operator/=(const mws_path& i_path);
   // removes a file or directory. returns true on success
   bool remove() const;
   // makes a directory. returns true on success
   bool make_dir() const;
   // tells if the path is empty
   bool is_empty() const;
   // tells if the path refers to a directory 
   bool is_directory() const;
   // tells if the path refers to a regular file 
   bool is_regular_file() const;
   // true if the path is absolute, false otherwise
   bool is_absolute() const;
   // false if the path is absolute, true otherwise
   bool is_relative() const;
   // tells if the path refers to existing file system object 
   bool exists() const;
   const std::string& string() const;
   // returns the filename path component 
   std::string filename() const;
   // returns the stem path component 
   std::string stem() const;
   // returns the file extension path component 
   std::string extension() const;
   // returns the directory path component 
   mws_path directory() const;
   // returns the path of the parent path 
   mws_path parent_path() const;
   // returns a list with the files in the directory denoted by this path
   std::vector<mws_sp<mws_file>> list_directory(bool i_recursive = false) const;
   // returns true if path is inside the resources directory
   bool is_internal() const;
   // returns the current path
   static std::string current_path();

private:
   // replaces '\' with '/'
   void make_standard_path();

   std::string path;
};
mws_path operator/(const mws_path& i_lhs, const mws_path& i_rhs);


// represents a regular file. can be either internal(inside the app's resources) or external(outside of the resource directory)
class mws_file final
{
public:
   // an internal file(or path) refers to a file inside the resource directory. everything else is considered an external file(or path)
   // an internal path must only have a filename component, but not a directory component,
   // as every internal filename is unique so there is no possibility of name collision
   static mws_sp<mws_file> get_inst(const mws_path& i_path);
   static mws_sp<mws_file> get_inst(mws_sp<mws_file_impl> i_impl);
   virtual ~mws_file();

   bool exists() const;
   bool is_open() const;
   bool is_writable() const;
   uint64_t length();
   uint64_t creation_time() const;
   uint64_t last_write_time() const;
   // returns the file's path as a mws_path
   const mws_path& path() const;
   // returns the file's path as a string
   std::string string_path() const;
   // returns the filename path component 
   std::string filename() const;
   // returns the stem path component 
   std::string stem() const;
   // returns the extension path component, including the dot
   std::string extension() const;
   // returns the directory path component 
   mws_path directory() const;
   // returns true if file is internal
   bool is_internal() const;
   FILE* get_file_impl() const;

   class io_op
   {
   public:
      bool open();
      bool open(std::string i_open_mode);
      void close();
      void flush();
      bool reached_eof() const;
      void set_io_position(uint64_t i_pos);

      int read(std::vector<uint8_t>& i_buffer);
      int write(const std::vector<uint8_t>& i_buffer);
      int read(uint8_t* i_buffer, int i_size);
      int write(const uint8_t* i_buffer, int i_size);

   private:
      friend class mws_file;
      io_op();
      mws_sp<mws_file_impl> impl;
   };
   io_op io;

private:
   mws_file();
};


class mws final
{
public:
   struct input
   {
      // translate a key from the platform encoding into the appplex encoding
      static mws_key_types translate_key(int i_pfm_key_id);
      // apply any modifiers like shift, ctrl, caps, num lock etc to get the final key value
      static mws_key_types apply_key_modifiers(mws_key_types i_key_id);
      // true if device has a touchscreen
      static bool has_touchscreen();
      // true if device has an active touchscreen or emulates one
      static bool uses_touchscreen();
   };


   struct output
   {
      static void write_text(const char* i_text);
      static void write_text_nl(const char* i_text);
      static void write_text(const wchar_t* i_text);
      static void write_text_nl(const wchar_t* i_text);
   };


   // arguments passed from the command line
   struct args
   {
      // standard string command line arguments
      static int get_str_arg_count();
      static const std::string& get_str_path();
      static const char** get_str_arg_vect();
      static const std::vector<std::string>& get_str_arg_str_vect();
      static void set_str_args(int i_argument_count, const char** i_argument_vector, bool i_app_path_included);

      // unicode(wide char) command line arguments(used for windows unicode paths)
      static int get_unicode_arg_count();
      static const unicode_string& get_unicode_path();
      static const std::vector<unicode_string>& get_unicode_arg_vect();
      static void set_unicode_args(int i_argument_count, unicode_char** i_argument_vector, bool i_app_path_included);
   };


   struct screen
   {
      static bool is_gfx_available();
      static int get_target_fps();
      // switches between screen width and height. only works in windowed desktop applications
      static void flip_screen();
      static bool is_full_screen_mode();
      static void set_full_screen_mode(bool i_enabled);
      static float get_brightness();
      static void set_brightness(float i_brightness);

      static uint32_t get_width();
      static uint32_t get_height();
      static float get_scale();
      static float get_scaled_width();
      static float get_scaled_height();

      // screen metrix
      // horizontal and vertical screen resolution in dots(pixels)
      static std::pair<uint32_t, uint32_t> get_res_px();
      // average dots(pixels) per inch
      static float get_avg_dpi();
      // horizontal and vertical dots(pixels) per inch
      static std::pair<float, float> get_dpi();
      // horizontal and vertical dimensions in inch
      static std::pair<float, float> get_dim_inch();
      // average dots(pixels) per cm
      static float get_avg_dpcm();
      // horizontal and vertical dots(pixels) per cm
      static std::pair<float, float> get_dpcm();
      // horizontal and vertical dimensions in cm
      static std::pair<float, float> get_dim_cm();
   };


   struct time
   {
      static std::string get_timezone_id();
      static uint32_t get_time_millis();
      static std::string get_current_date(const std::string& i_fmt = "");
      static std::string get_duration_as_string(uint32_t i_duration);
   };


   static mws_gfx_type get_gfx_type_id();
   static mws_pfm_id get_platform_id();

private:
   mws() {}
};


// set debug flags for use in conditional expressions to print debug output
// there should be no cost to using this on release builds, as anything that depends on it should be optimized out,
// as dead / unreachable code (because mws_debug_enabled will be 'false' on release builds)
class mws_dbg final
{
public:
   // standard flags list
   // memory allocations/deallocations
   static inline const uint64_t pfm_mem = 1 << 0;
   static inline const uint64_t app_mem = 1 << 1;
   // file io - open/close
   static inline const uint64_t pfm_files = 1 << 2;
   static inline const uint64_t app_files = 1 << 3;
   // touch events
   static inline const uint64_t pfm_touch = 1 << 4;
   static inline const uint64_t app_touch = 1 << 5;
   // mws/ui events
   static inline const uint64_t pfm_mws = 1 << 6;
   static inline const uint64_t app_mws = 1 << 7;
   // net events
   static inline const uint64_t pfm_net = 1 << 8;
   static inline const uint64_t app_net = 1 << 9;
   // last standard flag. flag values greater than this are considered custom flag values
   static inline const uint64_t last_std_flag = app_net;

   static bool enabled(uint64_t i_flags) { return mws_debug_enabled && ((flags & i_flags) != 0); }
   static uint64_t get_active_flags() { return flags; }

   static void set_flags(uint64_t i_flags, bool i_clear_flags = false)
   {
      // used flags must have already been registered
      mws_assert((i_flags & used_flags) == i_flags);

      if (i_clear_flags)
      {
         flags &= ~i_flags;
      }
      else
      {
         flags |= i_flags;
      }
   }

   // returns the most significant bit set in the registered flags
   // use this to register a new custom flag, by shifting left the returned value
   static uint64_t get_registered_flags_msb_val()
   {
      if (!used_flags)
      {
         return 0;
      }

      uint64_t msb_val = 1;

      while (used_flags >>= 1)
      {
         msb_val <<= 1;
      }

      return msb_val;
   }

   // if using non standard flag, register it first, to prevent collisions with other non standard flags
   static void register_flags(uint64_t i_flags)
   {
#if defined MWS_DEBUG_BUILD
      uint64_t std_flags = last_std_flag + last_std_flag - 1;
      // standard flags are already taken, cannot register
      mws_assert((i_flags & std_flags) == 0);
      // flags must not already be registered
      mws_assert((i_flags & used_flags) == 0);
#endif
      // register the custom flags
      used_flags |= i_flags;
   }

private:
   mws_dbg() {}

   // current active debug flags
   static inline uint64_t flags = 0;
   // set the standard flags
   static inline uint64_t used_flags = last_std_flag + last_std_flag - 1;
};


class mws_exception
#ifdef MWS_USES_EXCEPTIONS
   : public std::exception
#endif
{
public:
   mws_exception();
   mws_exception(const std::string& i_msg);
   mws_exception(const char* i_msg);
   virtual ~mws_exception();

   // returns a C-style character string describing the general cause of the current error
   virtual const char* what() const noexcept;

private:
   void set_msg(const char* i_msg);

   std::string msg;
};


class mws_text_buffer
{
public:
   virtual void push_front(const char* i_msg) = 0;
   virtual void push_front(const std::string& i_msg) = 0;
   virtual void clear() = 0;
};


// logging class. writes log to the tmp directory
class mws_log
{
public:
   static bool is_enabled();
   static void set_enabled(bool i_is_enabled);
   static mws_sp<mws_log> i();
   virtual const std::vector<std::string> get_log();
   virtual void push(const char* i_msg);
   virtual void pushf(const char* i_fmt, ...);
   virtual void clear();
   virtual void set_text_buffer(mws_sp<mws_text_buffer> i_text_buffer);

protected:
   mws_log();
};


// format lib
#include <fmt/format.h>

#define trn() mws::output::write_text_nl("")
#define wtrn() mws::output::write_text_nl("")
std::string mws_to_str_fmt(const char* i_format, ...);

template <typename... argst> void trx(const char* i_format, const argst& ... i_args)
{
   std::string s = fmt::format(i_format, i_args...);
   mws::output::write_text_nl(s.c_str());
}

template <typename... argst> void wtrx(const wchar_t* i_format, const argst& ... i_args)
{
   std::wstring s = fmt::format(i_format, i_args...);
   mws::output::write_text_nl(s.c_str());
}

template <typename... argst> void trc(const char* i_format, const argst& ... i_args)
{
   std::string s = fmt::format(i_format, i_args...);
   mws::output::write_text(s.c_str());
}

template <typename... argst> void wtrc(const wchar_t* i_format, const argst& ... i_args)
{
   std::wstring s = fmt::format(i_format, i_args...);
   mws::output::write_text(s.c_str());
}

template <typename... argst> std::string trs(const char* i_format, const argst& ... i_args)
{
   return fmt::format(i_format, i_args...);
}

template <typename... argst> std::wstring wtrs(const wchar_t* i_format, const argst& ... i_args)
{
   return fmt::format(i_format, i_args...);
}
