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
namespace mws_impl { class mws_file_impl; }
using umf_r = std::unordered_map<std::string, mws_sp<mws_file>>;
using umf_list = mws_sp<umf_r>;


class mws_file final
{
public:
   static mws_sp<mws_file> get_inst(const mws_path& i_path);
   static mws_sp<mws_file> get_inst(mws_sp<mws_impl::mws_file_impl> i_impl);
   virtual ~mws_file();

   bool exists() const;
   bool is_opened() const;
   bool is_writable() const;
   uint64 length();
   uint64 creation_time() const;
   uint64 last_write_time() const;
   // returns the file's path as a mws_path
   const mws_path& path() const;
   // returns the file's path as a string
   std::string string_path() const;
   // returns the filename path component 
   std::string filename() const;
   // returns the stem path component 
   std::string stem() const;
   // returns the extension path component 
   std::string extension() const;
   // returns the directory path component 
   mws_path directory() const;
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
      friend class mws_file;
      io_op();
      mws_sp<mws_impl::mws_file_impl> impl;
   };
   io_op io;

private:
   mws_file();
};


class mws_pfm_app
{
public:
   // update
   virtual void init();
   virtual void start();
   virtual void run();

   // input
   // translate a key from the platform encoding into the appplex encoding
   virtual mws_key_types translate_key(int i_pfm_key_id) const = 0;
   // apply any modifiers like shift, ctrl, caps, num lock etc to get the final key value
   mws_key_types apply_key_modifiers(mws_key_types i_key_id) const;
   virtual mws_key_types apply_key_modifiers_impl(mws_key_types i_key_id) const = 0;
   // return true to exit the app
   virtual bool back_evt() const;

   // screen
   virtual bool is_full_screen_mode() const = 0;
   virtual void set_full_screen_mode(bool i_enabled) const = 0;
   virtual float get_screen_scale() const;
   virtual float get_screen_brightness() const;
   virtual void set_screen_brightness(float i_brightness) const;
   // switches between screen width and height. this only works in windowed desktop applications
   virtual void flip_screen() const;

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

   // output
   virtual void write_text(const char* i_text) const = 0;
   virtual void write_text_nl(const char* i_text) const = 0;
   virtual void write_text(const wchar_t* i_text) const = 0;
   virtual void write_text_nl(const wchar_t* i_text) const = 0;
   virtual void write_text_v(const char* i_format, ...) const = 0;

   // filesystem
   virtual mws_sp<mws_impl::mws_file_impl> new_mws_file_impl(const mws_path& i_path) const = 0;
   virtual umf_list get_directory_listing(const std::string& i_directory, umf_list i_plist, bool i_is_recursive) const = 0;
   // writable/private/persistent files directory for the current mod
   virtual const mws_path& prv_dir() const = 0;
   // read-only/resource files directory for the current mod
   virtual const mws_path& res_dir() const = 0;
   // temporary files directory for the current mod
   virtual const mws_path& tmp_dir() const = 0;
   // reconfigures the above directories path to match the current mod
   virtual void reconfigure_directories(mws_sp<mws_mod> i_crt_mod) = 0;
   virtual std::string get_timezone_id() const = 0;
};


class mws final
{
public:
   struct filesys
   {
      static const std::string res_idx_name;
      static const umf_list get_res_file_list();
      static mws_sp<std::vector<uint8>> load_res_byte_vect(mws_sp<mws_file> i_file);
      static mws_sp<std::vector<uint8>> load_res_byte_vect(std::string i_filename);
      static mws_sp<std::string> load_res_as_string(mws_sp<mws_file> i_file);
      static mws_sp<std::string> load_res_as_string(std::string i_filename);
      // writable/private/persistent files directory
      static const mws_path& prv_dir();
      // read-only/resource files directory
      static const mws_path& res_dir();
      // temporary files directory
      static const mws_path& tmp_dir();
   };


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


   struct args
   {
      static int get_app_argument_count();
      static const unicode_string& get_app_path();
      static const std::vector<unicode_string>& get_app_argument_vector();
      static void set_app_arguments(int i_argument_count, unicode_char** i_argument_vector, bool i_app_path_included = false);
   };


   struct screen
   {
      static bool is_gfx_available();
      static int get_target_fps();
      // switches between screen width and height. this only works in windowed desktop applications
      static void flip_screen();
      static bool is_full_screen_mode();
      static void set_full_screen_mode(bool i_enabled);
      static float get_brightness();
      static void set_brightness(float i_brightness);

      static uint32 get_width();
      static uint32 get_height();
      static float get_scale();
      static float get_scaled_width();
      static float get_scaled_height();

      // screen metrix
      // horizontal and vertical screen resolution in dots(pixels)
      static std::pair<uint32, uint32> get_res_px();
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
      static uint32 get_time_millis();
      static std::string get_current_date(const std::string& i_fmt = "");
      static std::string get_duration_as_string(uint32 i_duration);
   };


   static mws_gfx_type get_gfx_type_id();
   static mws_pfm_id get_platform_id();

private:
   mws() {}
};


class mws_path final
{
public:
   mws_path();
   mws_path(const char* i_path, bool i_regular_path = true);
   mws_path(const std::string& i_path, bool i_regular_path = true);
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
   mws_sp<std::vector<mws_sp<mws_file>>> list_directory(bool i_recursive = false) const;
   // returns true for regular files and directories(reachable by fopen f.e.). returns false for special files, like resources in the apk on android.
   bool is_regular_path() const;
   // returns the current path
   static std::string current_path();

private:
   // replaces '\' with '/'
   void make_standard_path();
   void list_directory_impl(std::string i_base_dir, mws_sp<std::vector<mws_sp<mws_file>>> i_file_list, bool i_recursive) const;

   std::string path;
   bool regular_path = true;
};
mws_path operator/(const mws_path& i_lhs, const mws_path& i_rhs);


namespace mws_impl
{
   class mws_file_impl
   {
   public:
      mws_file_impl(const mws_path& i_path);
      virtual ~mws_file_impl();
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

      mws_path ppath;
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


// set debug flags for use in conditional expressions to print debug output
// there should be no cost to using this on release builds, as anything that depends on it should be optimized out,
// as dead / unreachable code (because mws_debug_enabled will be 'false' on release builds)
class mws_dbg final
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
   mws_dbg() {}

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


class mws_text_buffer
{
public:
   virtual void push_front(const char* i_msg) = 0;
   virtual void clear() = 0;
};


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
