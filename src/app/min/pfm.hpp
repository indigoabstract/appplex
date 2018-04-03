#pragma once

#include "pfm-def.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

using std::enable_shared_from_this;
using std::shared_ptr;
using std::static_pointer_cast;
using std::weak_ptr;


#if defined PLATFORM_WINDOWS_PC

void* operator new(std::size_t isize, const std::nothrow_t& nothrow_value) throw();
void operator delete(void* iptr, const std::nothrow_t& nothrow_constant) throw();

void* operator new[](std::size_t isize, const std::nothrow_t& nothrow_value) throw();
void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant) throw();

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
   static std::shared_ptr<pfm_path> get_inst(std::string ifile_path, std::string iaux_root_dir = "");
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
   void* get_file_impl()const;

   class io_op
   {
   public:
      bool open();
      bool open(std::string iopen_mode);
      void close();
      void seek(uint64 ipos);

      int read(shared_ptr<std::vector<uint8> > ibuffer);
      int write(const shared_ptr<std::vector<uint8> > ibuffer);
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
      virtual void* get_file_impl()const;
      virtual bool exists();
      virtual bool is_opened()const;
      virtual bool is_writable()const;
      virtual uint64 length() = 0;
      virtual uint64 creation_time()const = 0;
      virtual uint64 last_write_time()const = 0;
      virtual bool open(std::string iopen_mode);
      virtual void close();
      virtual void seek(uint64 ipos);
      virtual int read(shared_ptr<std::vector<uint8> > ibuffer);
      virtual int write(const shared_ptr<std::vector<uint8> > ibuffer);
      virtual int read(uint8* ibuffer, int isize);
      virtual int write(const uint8* ibuffer, int isize);
      virtual void check_state()const;

      pfm_path ppath;
      void* file;
      uint64 file_pos;
      bool file_is_open;
      bool file_is_writable;

   protected:
      virtual void* open_impl(std::string iopen_mode);
      virtual void close_impl();
      virtual void seek_impl(uint64 ipos, int iseek_pos);
      virtual uint64 tell_impl();
      virtual int read_impl(uint8* ibuffer, int isize);
      virtual int write_impl(const uint8* ibuffer, int isize);
   };
}


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
   virtual int get_screen_dpi()const = 0;
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
      static int get_target_fps();
      static int get_screen_dpi();
      static bool is_full_screen_mode();
      static void set_full_screen_mode(bool ienabled);
      static bool is_gfx_available();
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
void trx(std::string msg);
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

#define vprint(iformat, ...)		pfm::get_pfm_main_inst()->write_text_v(iformat, ##__VA_ARGS__)

// assert
#include <assert.h>

inline void ia_signal_error(const char* imessage = 0)
{
#if defined _DEBUG
   if (imessage)
   {
      trx(imessage);
   }

#if defined PLATFORM_WINDOWS_PC
   _asm int 3;
#else
   assert(icondition);
#endif
#endif
}

inline void ia_assert(bool icondition, const char* imessage)
{
#if defined _DEBUG
   if (icondition == false)
   {
      ia_signal_error(imessage);
   }
#endif
}

inline void ia_assert(bool icondition)
{
#if defined _DEBUG
   ia_assert(icondition, "assertion failed");
#endif
}
