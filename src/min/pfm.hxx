#pragma once

#include "pfm-def.h"
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


#if defined PLATFORM_WINDOWS_PC

//void* operator new(std::size_t i_size, const std::nothrow_t& nothrow_value);
//void operator delete(void* iptr, const std::nothrow_t& nothrow_constant);
//
//void* operator new[](std::size_t i_size, const std::nothrow_t& nothrow_value);
//void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant);

#endif

#if defined UNICODE_USING_STD_STRING

typedef std::string  unicodestring;
typedef char		 unicodechar;

#elif defined UNICODE_USING_STD_WSTRING

typedef std::wstring unicodestring;
typedef wchar_t		 unicodechar;

#endif


#if defined UNICODE_USING_STD_STRING

#define untr(arg)	 arg
#define utrn    trn
#define utrx	 trx
#define utrc	 trc
#define utrs	 trs

#elif defined UNICODE_USING_STD_WSTRING

#define untr(arg)	 L##arg
#define utrn	 wtrn
#define utrx	 wtrx
#define utrc	 wtrc
#define utrs	 wtrs

#endif

class mws_mod;
class mws_mod_ctrl;
class pfm_file;

namespace pfm_impl
{
   class pfm_file_impl;
}


// set debug flags for use in conditional expressions to print debug output
// there should be no cost to using this on release builds, as anything that depends on it should be optimized out,
// as dead / unreachable code (because mws_debug_enabled will be 'false' on release builds)
class mws_dbg
{
public:
   // standard flags list
   // touch events
   static inline const uint64 pfm_touch = 1 << 0;
   static inline const uint64 app_touch = 1 << 1;
   // mws/ui events
   static inline const uint64 pfm_mws = 1 << 1;
   static inline const uint64 app_mws = 1 << 2;
   // net events
   static inline const uint64 pfm_net = 1 << 3;
   static inline const uint64 app_net = 1 << 4;
   // last standard flag. flag values greater than this are considered custom flag values
   static inline const uint64 last_std_flag = app_net;

   static bool enabled(uint64 i_flags) { return mws_debug_enabled && ((flags & i_flags) != 0); }
   static uint64 get_active_flags() { return flags; }

   static void set_flags(uint64 i_flags, bool i_clear_flags = false)
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
   static uint64 get_registered_flags_msb_val()
   {
      if (!used_flags)
      {
         return 0;
      }

      uint64 msb_val = 1;

      while (used_flags >>= 1)
      {
         msb_val <<= 1;
      }

      return msb_val;
   }

   // if using non standard flag, register it first, to prevent collisions with other non standard flags
   static void register_flags(uint64 i_flags)
   {
      uint64 std_flags = last_std_flag + last_std_flag - 1;
      // standard flags are already taken, cannot register
      mws_assert((i_flags & std_flags) == 0);
      // flags must not already be registered
      mws_assert((i_flags & used_flags) == 0);
      // register the custom flags
      used_flags |= i_flags;
   }

private:
   // current active debug flags
   static inline uint64 flags = 0;
   // set the standard flags
   static inline uint64 used_flags = last_std_flag + last_std_flag - 1;
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


class pfm_path
{
public:
   static mws_sp<pfm_path> get_inst(std::string i_file_path, std::string i_aux_root_dir = "");
   bool remove();
   bool make_dir();
   bool is_directory() const;
   bool is_regular_file() const;
   bool exists() const;
   std::string get_current_path() const;
   std::string get_full_path() const;
   const std::string& get_file_name() const;
   std::string get_file_stem() const;
   std::string get_file_extension() const;
   const std::string& get_root_directory() const;
   mws_sp<std::vector<mws_sp<pfm_file>>> list_directory(mws_sp<mws_mod> i_mod = nullptr, bool i_recursive = false) const;

private:
   friend class pfm_impl::pfm_file_impl;
   pfm_path() {}
   void make_standard_path();
   void list_directory_impl(std::string i_base_dir, mws_sp<std::vector<mws_sp<pfm_file> > > i_file_list, bool i_recursive) const;

   std::string filename;
   std::string aux_root_dir;
};


class pfm_file
{
public:
   static mws_sp<pfm_file> get_inst(std::string i_filename, std::string i_root_dir = "");
   static mws_sp<pfm_file> get_inst(mws_sp<pfm_impl::pfm_file_impl> i_impl);
   virtual ~pfm_file();

   bool exists() const;
   bool is_opened() const;
   bool is_writable() const;
   uint64 length();
   uint64 creation_time() const;
   uint64 last_write_time() const;
   std::string get_full_path() const;
   const std::string& get_file_name() const;
   std::string get_file_stem() const;
   std::string get_file_extension() const;
   const std::string& get_root_directory() const;
   FILE* get_file_impl() const;

   class io_op
   {
   public:
      bool open();
      bool open(std::string i_open_mode);
      void close();
      void flush();
      bool reached_eof() const;
      void seek(uint64 i_pos);

      int read(std::vector<uint8>& i_buffer);
      int write(const std::vector<uint8>& i_buffer);
      int read(uint8* i_buffer, int i_size);
      int write(const uint8* i_buffer, int i_size);

   private:
      friend class pfm_file;
      io_op();
      mws_sp<pfm_impl::pfm_file_impl> impl;
   };
   io_op io;

private:
   friend class msvc_main;
   pfm_file();
};


using umf_r = std::unordered_map < std::string, mws_sp<pfm_file> >;
using umf_list = mws_sp < umf_r >;


namespace pfm_impl
{
   class pfm_file_impl
   {
   public:
      pfm_file_impl(const std::string& i_filename, const std::string& i_root_dir);
      virtual ~pfm_file_impl();
      virtual FILE* get_file_impl() const = 0;
      virtual bool exists();
      virtual bool is_opened() const;
      virtual bool is_writable() const;
      virtual uint64 length() = 0;
      virtual uint64 creation_time() const = 0;
      virtual uint64 last_write_time() const = 0;
      virtual bool open(std::string i_open_mode);
      virtual void close();
      virtual void flush();
      virtual bool reached_eof() const;
      virtual void seek(uint64 i_pos);
      virtual int read(std::vector<uint8>& i_buffer);
      virtual int write(const std::vector<uint8>& i_buffer);
      virtual int read(uint8* i_buffer, int i_size);
      virtual int write(const uint8* i_buffer, int i_size);
      virtual void check_state() const;

      pfm_path ppath;
      uint64 file_pos;
      bool file_is_open;
      bool file_is_writable;

   protected:
      virtual bool open_impl(std::string i_open_mode) = 0;
      virtual void close_impl() = 0;
      virtual void flush_impl() = 0;
      virtual void seek_impl(uint64 i_pos, int i_seek_pos);
      virtual uint64 tell_impl();
      virtual int read_impl(uint8* i_buffer, int i_size);
      virtual int write_impl(const uint8* i_buffer, int i_size);
   };
}


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

protected:
   mws_log();
   static mws_sp<mws_log> inst;
   static bool enabled;
};


class pfm_data
{
public:
   pfm_data();

   bool gfx_available;
   uint32 screen_width;
   uint32 screen_height;
};


class pfm_main
{
public:
   static mws_sp<pfm_main> gi();
   virtual void init();
   virtual void start();
   virtual void run();
   // translate a key from the platform encoding into the appplex encoding
   virtual key_types translate_key(int i_pfm_key_id) const = 0;
   // apply any modifiers like shift, ctrl, caps, num lock etc to get the final key value
   key_types apply_key_modifiers(key_types i_key_id) const;
   virtual key_types apply_key_modifiers_impl(key_types i_key_id) const = 0;
   virtual float get_screen_scale() const;
   virtual float get_screen_brightness() const;
   virtual void set_screen_brightness(float i_brightness);
   
   // screen metrix
   // horizontal and vertical screen resolution in dots(pixels)
   virtual std::pair<uint32, uint32> get_screen_res_px() const = 0;
   // average dots(pixels) per inch
   virtual float get_avg_screen_dpi() const = 0;
   // horizontal and vertical dots(pixels) per inch
   virtual std::pair<float, float> get_screen_dpi() const = 0;
   // horizontal and vertical dimensions in inch
   virtual std::pair<float, float> get_screen_dim_inch() const = 0;
   // average dots(pixels) per cm
   virtual float get_avg_screen_dpcm() const;
   // horizontal and vertical dots(pixels) per cm
   virtual std::pair<float, float> get_screen_dpcm() const = 0;
   // horizontal and vertical dimensions in cm
   virtual std::pair<float, float> get_screen_dim_cm() const = 0;

   // switches between screen width and height. this only works in windowed desktop applications
   virtual void flip_screen() {};
   virtual void write_text(const char* i_text) const = 0;
   virtual void write_text_nl(const char* i_text) const = 0;
   virtual void write_text(const wchar_t* i_text) const = 0;
   virtual void write_text_nl(const wchar_t* i_text) const = 0;
   virtual void write_text_v(const char* i_format, ...) const = 0;
   virtual std::string get_writable_path() const = 0;
   virtual std::string get_timezone_id() const = 0;
   // return true to exit the app
   virtual bool back_evt();
};


class pfm
{
public:
   struct params
   {
      static int get_app_argument_count();
      static const unicodestring& get_app_path();
      static const std::vector<unicodestring>& get_app_argument_vector();
      static void set_app_arguments(int i_argument_count, unicodechar** i_argument_vector, bool i_app_path_included = false);
   };


   struct screen
   {
      static uint32 get_width();
      static uint32 get_height();
      static float get_scale();
      static float get_scaled_width();
      static float get_scaled_height();
      static int get_target_fps();
      static float get_avg_screen_dpi();
      static bool is_full_screen_mode();
      static void set_full_screen_mode(bool i_enabled);
      static bool is_gfx_available();
      // switches between screen width and height. this only works in windowed desktop applications
      static void flip_screen();
   };


   class filesystem
   {
   public:
      static const std::string res_idx_name;
      static const umf_list get_res_file_list();
      static std::string get_tmp_path(std::string i_name = "");
      static std::string get_writable_path(std::string i_name = "");
      static std::string get_path(std::string i_name);
      static void load_res_file_map(mws_sp<mws_mod> i_mod);
      //static shared_array<uint8> load_res_byte_array(std::string i_filename, int& i_size);
      static mws_sp<std::vector<uint8> > load_res_byte_vect(mws_sp<pfm_file> i_file);
      static mws_sp<std::vector<uint8> > load_res_byte_vect(std::string i_filename);
      static mws_sp<std::string> load_res_as_string(mws_sp<pfm_file> i_file);
      static mws_sp<std::string> load_res_as_string(std::string i_filename);

   private:

      friend class mws_mod;

      static mws_sp<std::vector<uint8> > load_mod_byte_vect(mws_sp<mws_mod> i_mod, std::string i_filename);
      //static shared_array<uint8> load_mod_byte_array(mws_sp<mws_mod> i_mod, std::string i_filename, int& i_size);
      static bool store_mod_byte_array(mws_sp<mws_mod> i_mod, std::string i_filename, const uint8* i_res, int i_size);
      static bool store_mod_byte_vect(mws_sp<mws_mod> i_mod, std::string i_filename, const std::vector<uint8>& i_res);
      static mws_sp<pfm_file> random_access(mws_sp<mws_mod> i_mod, std::string i_filename);
   };


   struct time
   {
      static uint32 get_time_millis();
   };


   static platform_id get_platform_id();
   // true if device has a touchscreen
   static bool has_touchscreen();
   // true if device has an active touchscreen or emulates one
   static bool uses_touchscreen();
   static gfx_type_id get_gfx_type_id();
   static mws_sp<pfm_main> get_pfm_main_inst();

private:
   friend class mws_mod_ctrl;
   friend class mws_mod_ctrl;

   static pfm_data data;

   pfm() {}
};


// format lib
#include <fmt/format.h>

#define trn() pfm::get_pfm_main_inst()->write_text_nl("")
#define wtrn() pfm::get_pfm_main_inst()->write_text_nl("")
std::string mws_to_str_fmt(const char* i_format, ...);

template <typename... argst> void trx(const char* i_format, const argst& ... i_args)
{
   std::string s = fmt::format(i_format, i_args...);
   pfm::get_pfm_main_inst()->write_text_nl(s.c_str());
}

template <typename... argst> void wtrx(const wchar_t* i_format, const argst& ... i_args)
{
   std::wstring s = fmt::format(i_format, i_args...);
   pfm::get_pfm_main_inst()->write_text_nl(s.c_str());
}

template <typename... argst> void trc(const char* i_format, const argst& ... i_args)
{
   std::string s = fmt::format(i_format, i_args...);
   pfm::get_pfm_main_inst()->write_text(s.c_str());
}

template <typename... argst> void wtrc(const wchar_t* i_format, const argst& ... i_args)
{
   std::wstring s = fmt::format(i_format, i_args...);
   pfm::get_pfm_main_inst()->write_text(s.c_str());
}

template <typename... argst> std::string trs(const char* i_format, const argst& ... i_args)
{
   return fmt::format(i_format, i_args...);
}

template <typename... argst> std::wstring wtrs(const wchar_t* i_format, const argst& ... i_args)
{
   return fmt::format(i_format, i_args...);
}
