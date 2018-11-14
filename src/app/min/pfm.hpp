#pragma once

#include "pfm-def.h"
#include <cstdio>
#include <string>
#include <vector>
#include <unordered_map>


#if defined PLATFORM_WINDOWS_PC

void* operator new(std::size_t isize, const std::nothrow_t& nothrow_value);
void operator delete(void* iptr, const std::nothrow_t& nothrow_constant);

void* operator new[](std::size_t isize, const std::nothrow_t& nothrow_value);
void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant);

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

class ia_console;
class unit;
class unit_ctrl;
class pfm_file;

namespace pfm_impl
{
   class pfm_file_impl;
}


class pfm_path
{
public:
   static std::shared_ptr<pfm_path> get_inst(std::string ifile_path, std::string i_aux_root_dir = "");
   std::string get_full_path()const;
   const std::string& get_file_name()const;
   std::string get_file_stem()const;
   std::string get_file_extension()const;
   const std::string& get_root_directory()const;
   shared_ptr<std::vector<shared_ptr<pfm_file> > > list_directory(std::shared_ptr<unit> iu = nullptr, bool recursive = false)const;

private:
   friend class pfm_impl::pfm_file_impl;
   pfm_path() {}
   void make_standard_path();
   void list_directory_impl(std::string ibase_dir, std::shared_ptr<std::vector<std::shared_ptr<pfm_file> > > ifile_list, bool irecursive) const;

   std::string filename;
   std::string aux_root_dir;
};


class pfm_file
{
public:
   static shared_ptr<pfm_file> get_inst(std::string ifilename, std::string iroot_dir = "");
   static shared_ptr<pfm_file> get_inst(shared_ptr<pfm_impl::pfm_file_impl> iimpl);
   virtual ~pfm_file();

   bool remove();
   bool exists();
   bool is_opened()const;
   bool is_writable()const;
   uint64 length();
   uint64 creation_time()const;
   uint64 last_write_time()const;
   std::string get_full_path()const;
   const std::string& get_file_name()const;
   std::string get_file_stem()const;
   std::string get_file_extension()const;
   const std::string& get_root_directory()const;
   FILE* get_file_impl()const;

   class io_op
   {
   public:
      bool open();
      bool open(std::string iopen_mode);
      void close();
      void flush();
      bool reached_eof() const;
      void seek(uint64 ipos);

      int read(std::vector<uint8>& ibuffer);
      int write(const std::vector<uint8>& ibuffer);
      int read(uint8* ibuffer, int isize);
      int write(const uint8* ibuffer, int isize);

   private:
      friend class pfm_file;
      io_op();
      shared_ptr<pfm_impl::pfm_file_impl> impl;
   };
   io_op io;

private:
   friend class msvc_main;
   pfm_file();
};


using umf_r = std::unordered_map < std::string, shared_ptr<pfm_file> >;
using umf_list = shared_ptr < umf_r >;


namespace pfm_impl
{
   class pfm_file_impl
   {
   public:
      pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir);
      virtual ~pfm_file_impl();
      virtual FILE* get_file_impl() const = 0;
      virtual bool exists();
      virtual bool is_opened()const;
      virtual bool is_writable()const;
      virtual uint64 length() = 0;
      virtual uint64 creation_time()const = 0;
      virtual uint64 last_write_time()const = 0;
      virtual bool open(std::string iopen_mode);
      virtual void close();
      virtual void flush();
      virtual bool reached_eof() const;
      virtual void seek(uint64 ipos);
      virtual int read(std::vector<uint8>& ibuffer);
      virtual int write(const std::vector<uint8>& ibuffer);
      virtual int read(uint8* ibuffer, int isize);
      virtual int write(const uint8* ibuffer, int isize);
      virtual void check_state()const;

      pfm_path ppath;
      uint64 file_pos;
      bool file_is_open;
      bool file_is_writable;

   protected:
      virtual bool open_impl(std::string iopen_mode) = 0;
      virtual void close_impl() = 0;
      virtual void flush_impl() = 0;
      virtual void seek_impl(uint64 ipos, int iseek_pos);
      virtual uint64 tell_impl();
      virtual int read_impl(uint8* ibuffer, int isize);
      virtual int write_impl(const uint8* ibuffer, int isize);
   };
}


class mws_log
{
public:
   static mws_sp<mws_log> i();
   virtual const std::vector<std::string>& get_log() = 0;
   virtual void push(const char* i_msg) = 0;
   virtual void pushf(const char* i_fmt, ...) = 0;
   virtual void clear() = 0;

protected:
   mws_log();
   static mws_sp<mws_log> inst;
};


class pfm_data
{
public:
   pfm_data();

   bool gfx_available;
   int screen_width;
   int screen_height;
   //shared_ptr<ia_console> console;
};


class pfm_main
{
public:
   virtual void init();
   virtual void start();
   virtual void run();
   virtual float get_screen_scale() const;
   virtual float get_screen_brightness() const;
   virtual void set_screen_brightness(float i_brightness);
   virtual int get_screen_dpi()const = 0;
   // switches between screen width and height. this only works in windowed desktop applications
   virtual void flip_screen() {};
   virtual void write_text(const char* text)const = 0;
   virtual void write_text_nl(const char* text)const = 0;
   virtual void write_text(const wchar_t* text)const = 0;
   virtual void write_text_nl(const wchar_t* text)const = 0;
   virtual void write_text_v(const char* iformat, ...)const = 0;
   virtual std::string get_writable_path()const = 0;
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
      static void set_app_arguments(int iargument_count, unicodechar** iargument_vector, bool iapp_path_included = false);
   };


   struct screen
   {
      static int get_width();
      static int get_height();
      static float get_scale();
      static float get_scaled_width();
      static float get_scaled_height();
      static int get_target_fps();
      static int get_screen_dpi();
      static bool is_full_screen_mode();
      static void set_full_screen_mode(bool ienabled);
      static bool is_gfx_available();
      // switches between screen width and height. this only works in windowed desktop applications
      static void flip_screen();
   };


   class filesystem
   {
   public:
      static std::string get_writable_path(std::string iname);
      static std::string get_path(std::string iname);
      static void load_res_file_map(shared_ptr<unit> iu = nullptr);
      //static shared_array<uint8> load_res_byte_array(std::string ifile_name, int& isize);
      static shared_ptr<std::vector<uint8> > load_res_byte_vect(shared_ptr<pfm_file> ifile);
      static shared_ptr<std::vector<uint8> > load_res_byte_vect(std::string ifile_name);
      static shared_ptr<std::string> load_res_as_string(shared_ptr<pfm_file> ifile);
      static shared_ptr<std::string> load_res_as_string(std::string ifilename);

   private:

      friend class unit;

      static shared_ptr<std::vector<uint8> > load_unit_byte_vect(shared_ptr<unit> iu, std::string ifile_name);
      //static shared_array<uint8> load_unit_byte_array(shared_ptr<unit> iu, std::string ifile_name, int& isize);
      static bool store_unit_byte_array(shared_ptr<unit> iu, std::string ifile_name, const uint8* ires, int isize);
      static bool store_unit_byte_vect(shared_ptr<unit> iu, std::string ifile_name, const std::vector<uint8>& ires);
      static shared_ptr<pfm_file> random_access(shared_ptr<unit> iu, std::string ifile_name);
   };


   struct time
   {
      static uint32 get_time_millis();
   };


   static platform_id get_platform_id();
   static gfx_type_id get_gfx_type_id();
   static shared_ptr<pfm_main> get_pfm_main_inst();
   //static shared_ptr<ia_console> get_console();

private:
   friend class unit_ctrl;
   friend class unit_ctrl;

   static pfm_data data;

   pfm() {}
};


// format lib
#include <fmt/format.h>

#define trn() pfm::get_pfm_main_inst()->write_text_nl("")
#define wtrn() pfm::get_pfm_main_inst()->write_text_nl("")
void trx(const char* i_msg);
void trx(std::string msg);
std::string mws_to_str(const char* i_format, ...);
void trx(const char* format, fmt::ArgList args);
FMT_VARIADIC(void, trx, const char*)
void wtrx(const wchar_t* format, fmt::ArgList args);
FMT_VARIADIC_W(void, wtrx, const wchar_t*)
void trc(const char* format, fmt::ArgList args);
FMT_VARIADIC(void, trc, const char*)
void wtrc(const wchar_t* format, fmt::ArgList args);
FMT_VARIADIC_W(void, wtrc, const wchar_t*)
std::string trs(const char* format, fmt::ArgList args);
FMT_VARIADIC(std::string, trs, const char*)
std::wstring wtrs(const wchar_t* format, fmt::ArgList args);
FMT_VARIADIC_W(std::wstring, wtrs, const wchar_t*)
