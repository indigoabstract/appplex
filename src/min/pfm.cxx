#include "stdafx.hxx"

#include "pfm.hxx"
#include "pfm-gl.h"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "min.hxx"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

using std::string;
using std::wstring;
using std::vector;


void mws_signal_error_impl(const char* i_file, uint32 i_line, const char* i_message)
{
#if defined MWS_DEBUG_BUILD

   const char* msg = (i_message) ? i_message : "signal error";
   mws_print("[%s] at file [%s], line [%d]\n", msg, i_file, i_line);

#if defined PLATFORM_WINDOWS_PC

   _asm int 3;

#else

   assert(false);

#endif
#endif
}

void mws_assert_impl(const char* i_file, uint32 i_line, bool i_condition)
{
#if defined MWS_DEBUG_BUILD

   if (!i_condition)
   {
      mws_signal_error_impl(i_file, i_line, "assertion failed");
   }

#endif
}


mws_exception::mws_exception()
{
   set_msg("");
}

mws_exception::mws_exception(const std::string& i_msg)
{
   set_msg(i_msg.c_str());
}

mws_exception::mws_exception(const char* i_msg)
{
   set_msg(i_msg);
}

mws_exception::~mws_exception()
{
}

const char* mws_exception::what() const noexcept
{
   return msg.c_str();
}

void mws_exception::set_msg(const char* i_msg)
{
   msg = i_msg;

#ifndef MWS_USES_EXCEPTIONS
   mws_assert(false);
#endif
}


// platform specific code
#if defined PLATFORM_ANDROID


#include "main.hxx"
#include <sys/stat.h>

#define pfm_app_inst		android_main::get_instance()

const std::string dir_separator = "/";

platform_id pfm::get_platform_id()
{
   return platform_android;
}

gfx_type_id pfm::get_gfx_type_id()
{
   return gfx_type_opengl_es;
}

#include <android/log.h>

// trace
#define os_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)
#define wos_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)

uint32 pfm::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      pfm_app_inst->write_text_nl("error");
   }

   return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool pfm_file::make_dir()
{
   int rval = mkdir(get_full_path().c_str(), 0777);
   return (rval == 0);
}


#elif defined PLATFORM_IOS

#include "main.hxx"
#include <sys/stat.h>

#define pfm_app_inst        ios_main::get_instance()


const std::string dir_separator = "/";

platform_id pfm::get_platform_id()
{
   return platform_ios;
}

gfx_type_id pfm::get_gfx_type_id()
{
   return gfx_type_opengl_es;
}

uint32 pfm::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      pfm_app_inst->write_text_nl("error");
   }

   return uint32(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

bool pfm_file::make_dir()
{
   int rval = mkdir(get_full_path().c_str(), 0777);
   return (rval == 0);
}


#elif defined PLATFORM_EMSCRIPTEN


#include "main.hxx"

#define pfm_app_inst emst_main::get_instance()

const std::string dir_separator = "/";

platform_id pfm::get_platform_id()
{
   return platform_emscripten;
}

gfx_type_id pfm::get_gfx_type_id()
{
   return gfx_type_opengl_es;
}

uint32 pfm::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      pfm_app_inst->write_text_nl("error");
   }

   return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool pfm_file::make_dir()
{
   return false;
}


#elif defined PLATFORM_WINDOWS_PC


#include "main.hxx"
#include <direct.h>

#define pfm_app_inst		msvc_main::get_instance()

const std::string dir_separator = "\\";

//#ifdef USE_VLD
//#include <vld.h>
//#endif
//#include <new>

namespace std { const nothrow_t nothrow = nothrow_t(); }

platform_id pfm::get_platform_id()
{
   return platform_windows_pc;
}

gfx_type_id pfm::get_gfx_type_id()
{
   return gfx_type_opengl;
}

uint32 pfm::time::get_time_millis()
{
   return GetTickCount();
}

bool pfm_file::make_dir()
{
   int rval = _mkdir(get_full_path().c_str());
   return (rval == 0);
}


#endif



int arg_count = 0;
unicodestring app_path;
vector<unicodestring> arg_vector;


namespace pfm_impl
{
   umf_list res_files_map;

   void print_type_sizes()
   {
      static_assert(sizeof(int8) == 1);
      static_assert(sizeof(sint8) == 1);
      static_assert(sizeof(uint8) == 1);
      static_assert(sizeof(int16) == 2);
      static_assert(sizeof(sint16) == 2);
      static_assert(sizeof(uint16) == 2);
      static_assert(sizeof(int32) == 4);
      static_assert(sizeof(sint32) == 4);
      static_assert(sizeof(uint32) == 4);
      static_assert(sizeof(int64) == 8);
      static_assert(sizeof(sint64) == 8);
      static_assert(sizeof(uint64) == 8);
      static_assert(sizeof(real32) == 4);
      static_assert(sizeof(real64) == 8);
      
      mws_print("print type sizes\n");
      mws_print("sizeof int8 [%d]\n", sizeof(int8));
      mws_print("sizeof sint8 [%d]\n", sizeof(sint8));
      mws_print("sizeof uint8 [%d]\n", sizeof(uint8));
      mws_print("sizeof int16 [%d]\n", sizeof(int16));
      mws_print("sizeof sint16 [%d]\n", sizeof(sint16));
      mws_print("sizeof uint16 [%d]\n", sizeof(uint16));
      mws_print("sizeof int32 [%d]\n", sizeof(int32));
      mws_print("sizeof sint32 [%d]\n", sizeof(sint32));
      mws_print("sizeof uint32 [%d]\n", sizeof(uint32));
      mws_print("sizeof int64 [%d]\n", sizeof(int64));
      mws_print("sizeof sint64 [%d]\n", sizeof(sint64));
      mws_print("sizeof uint64 [%d]\n", sizeof(uint64));
      mws_print("sizeof real32 [%d]\n", sizeof(real32));
      mws_print("sizeof real64 [%d]\n", sizeof(real64));
   }

   //true if res is in the same dir as src
   bool res_is_bundled_with_src()
   {
#if defined PLATFORM_WINDOWS_PC && defined MWS_DEBUG_BUILD
      return true;
#endif

      return false;
   }

   std::string get_concat_path(std::string iroot_path, ::string iname)
   {
      std::string p = iroot_path;

      if (iname[0] == '/' || (pfm::get_platform_id() == platform_android))
      {
         p += iname;
      }
      else
      {
         p = p + "/" + iname;
      }

      return p;
   }

   const std::string& get_appplex_proj_path()
   {
      std::string p;

      switch (pfm::get_platform_id())
      {
      case platform_android:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_ios:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_emscripten:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_qt_windows_pc:
      {
         static std::string res_path = "..";
         return res_path;
      }

      case platform_windows_pc:
      {
         static std::string res_path = "../../..";
         return res_path;
      }
      }

      mws_throw mws_exception("undefined platform");
   }

   const std::string& get_common_res_path()
   {
      switch (pfm::get_platform_id())
      {
      case platform_android:
      {
         static std::string res_path = "res";
         return res_path;
      }

      case platform_ios:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_emscripten:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_qt_windows_pc:
      {
         static std::string res_path = "../src/res";
         return res_path;
      }

      case platform_windows_pc:
      {
         if (res_is_bundled_with_src())
         {
            static std::string res_path = get_appplex_proj_path() + "/src/res";
            return res_path;
         }
         else
         {
            static std::string res_path = "res";
            return res_path;
         }
      }
      }

      mws_throw mws_exception("undefined platform");
   }

   const std::string& get_mod_res_path(mws_sp<mws_mod> i_mod)
   {
      switch (pfm::get_platform_id())
      {
      case platform_android:
      {
         return i_mod->get_name();
      }

      case platform_ios:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_emscripten:
      {
         static std::string res_path = "";
         return res_path;
      }

      case platform_qt_windows_pc:
      {
         static std::string res_path = "../src/res";
         return res_path;
      }

      case platform_windows_pc:
      {
         if (res_is_bundled_with_src())
         {
            static std::string res_path = get_appplex_proj_path() + "/" + i_mod->get_proj_rel_path() + "/res";
            return res_path;
         }
         else
         {
            static std::string res_path = "res";
            return res_path;
         }
      }
      }

      mws_throw mws_exception("undefined platform");
   }

   mws_sp<pfm_file> get_res_file(const std::string& i_filename)
   {
      mws_sp<pfm_file> file;

      if (!res_files_map)
      {
         pfm::filesystem::load_res_file_map();
      }

      auto it = res_files_map->find(i_filename);

      if (it != res_files_map->end())
      {
         file = it->second;
      }

      return file;
   }

   void put_res_file(const std::string& i_filename, mws_sp<pfm_file> ifile)
   {
      if (!res_files_map)
      {
         pfm::filesystem::load_res_file_map();
      }

      auto it = res_files_map->find(i_filename);

      if (it != res_files_map->end())
      {
         std::string msg = "duplicate filename: " + it->first;

         mws_throw mws_exception(msg.c_str());
      }

      (*res_files_map)[i_filename] = ifile;
   }

   pfm_file_impl::pfm_file_impl(const std::string& i_filename, const std::string& iroot_dir)
   {
      ppath.filename = i_filename;
      ppath.aux_root_dir = iroot_dir;
      ppath.make_standard_path();
      file_pos = 0;
      file_is_open = false;
      file_is_writable = false;
   }

   pfm_file_impl::~pfm_file_impl()
   {
   }

   bool pfm_file_impl::exists()
   {
      if (open("r"))
      {
         close();

         return true;
      }

      return false;
   }

   bool pfm_file_impl::is_opened()const
   {
      return file_is_open;
   }

   bool pfm_file_impl::is_writable()const
   {
      return file_is_writable;
   }

   bool pfm_file_impl::open(std::string iopen_mode)
   {
      file_is_open = open_impl(iopen_mode);

      return file_is_open;
   }

   void pfm_file_impl::close()
   {
      if (file_is_open)
      {
         close_impl();
      }

      file_pos = 0;
      file_is_open = false;
   }

   void pfm_file_impl::flush()
   {
      check_state();
      flush_impl();
   }

   bool pfm_file_impl::reached_eof() const
   {
      check_state();
      FILE* f = get_file_impl();
      //long pos = ftell(f);
      int is_eof = feof(f);

      return is_eof != 0;
   }

   void pfm_file_impl::seek(uint64 ipos)
   {
      check_state();

      seek_impl(ipos, SEEK_SET);
      file_pos = ipos;
   }

   int pfm_file_impl::read(std::vector<uint8>& ibuffer)
   {
      check_state();

      ibuffer.resize((size_t)length());

      return read(begin_ptr(ibuffer), ibuffer.size());
   }

   int pfm_file_impl::write(const std::vector<uint8>& ibuffer)
   {
      check_state();

      return write(begin_ptr(ibuffer), ibuffer.size());
   }

   int pfm_file_impl::read(uint8* ibuffer, int isize)
   {
      check_state();

      int bytesread = read_impl(ibuffer, isize);

      return bytesread;
   }

   int pfm_file_impl::write(const uint8* ibuffer, int isize)
   {
      check_state();

      int byteswritten = write_impl(ibuffer, isize);

      return byteswritten;
   }

   void pfm_file_impl::check_state()const
   {
      if (!file_is_open)
      {
         std::string msg = "file " + ppath.filename + " is not open";

         mws_throw mws_exception(msg.c_str());
      }
   }

   void pfm_file_impl::seek_impl(uint64 ipos, int iseek_pos)
   {
      fseek(get_file_impl(), (long)ipos, iseek_pos);
   }

   uint64 pfm_file_impl::tell_impl()
   {
      return ftell(get_file_impl());
   }

   int pfm_file_impl::read_impl(uint8* ibuffer, int isize)
   {
      return fread(ibuffer, 1, isize, get_file_impl());
   }

   int pfm_file_impl::write_impl(const uint8* ibuffer, int isize)
   {
      return fwrite(ibuffer, 1, isize, get_file_impl());
   }
}


pfm_file::pfm_file()
{
}

pfm_file::~pfm_file()
{
   io.close();
}

mws_sp<pfm_file> pfm_file::get_inst(std::string i_filename, std::string iroot_dir)
{
   mws_sp<pfm_file> inst;
   auto ppath = pfm_path::get_inst(i_filename, iroot_dir);

   // if res map initialized
   if (pfm_impl::res_files_map)
   {
      auto pfile = pfm_impl::get_res_file(ppath->get_file_name());

      if (pfile)
      {
         if (ppath->get_root_directory().empty())
         {
            inst = pfile;
         }
         else
         {
            if (pfile->get_root_directory().find(ppath->get_root_directory()) != std::string::npos)
            {
               inst = pfile;
            }
         }
      }
   }

   if (!inst)
   {
      inst = mws_sp<pfm_file>(new pfm_file());
      inst->io.impl = pfm_app_inst->new_pfm_file_impl(ppath->get_file_name(), ppath->get_root_directory());
   }

   return inst;
}

mws_sp<pfm_file> pfm_file::get_inst(mws_sp<pfm_impl::pfm_file_impl> iimpl)
{
   mws_sp<pfm_file> inst;

   if (pfm_impl::res_files_map)
      // res map initialized
   {
      inst = pfm_impl::get_res_file(iimpl->ppath.get_file_name());
   }

   if (!inst)
   {
      inst = mws_sp<pfm_file>(new pfm_file());
      inst->io.impl = iimpl;
   }

   return inst;
}

bool pfm_file::remove()
{
   mws_throw mws_exception("not implemented");

   return false;
}

bool pfm_file::is_directory() const
{
   struct stat info;

   if (stat(get_full_path().c_str(), &info) != 0)
   {
      return false;
   }

   if (info.st_mode & S_IFDIR)
   {
      return true;
   }

   return false;
}

bool pfm_file::is_regular_file() const
{
   struct stat info;

   if (stat(get_full_path().c_str(), &info) != 0)
   {
      return false;
   }

   if (!(info.st_mode & S_IFDIR))
   {
      return true;
   }

   return false;
}

bool pfm_file::exists() const
{
   struct stat info;

   if (stat(get_full_path().c_str(), &info) != 0)
   {
      return io.impl->exists();
   }

   return true;
}

bool pfm_file::is_opened()const
{
   return io.impl->is_opened();
}

bool pfm_file::is_writable()const
{
   return io.impl->is_writable();
}

uint64 pfm_file::length()
{
   return io.impl->length();
}

uint64 pfm_file::creation_time()const
{
   return io.impl->creation_time();
}

uint64 pfm_file::last_write_time()const
{
   return io.impl->last_write_time();
}

std::string pfm_file::get_full_path()const
{
   return io.impl->ppath.get_full_path();
}

const std::string& pfm_file::get_file_name()const
{
   return io.impl->ppath.get_file_name();
}

std::string pfm_file::get_file_stem()const
{
   return io.impl->ppath.get_file_stem();
}

std::string pfm_file::get_file_extension()const
{
   return io.impl->ppath.get_file_extension();
}

const std::string& pfm_file::get_root_directory()const
{
   return io.impl->ppath.get_root_directory();
}

FILE* pfm_file::get_file_impl()const
{
   return io.impl->get_file_impl();
}

pfm_file::io_op::io_op()
{
}

bool pfm_file::io_op::open()
{
   bool file_opened = open("rb");

   return file_opened;
}

bool pfm_file::io_op::open(std::string iopen_mode)
{
   if (impl->is_opened())
   {
      impl->close();
   }

   bool file_opened = impl->open(iopen_mode);

   if (!file_opened)
   {
      mws_print_impl("WARNING[ file [%s] NOT FOUND. ]", impl->ppath.get_file_name().c_str());
   }

   return file_opened;
}

void pfm_file::io_op::close()
{
   impl->close();
}

void pfm_file::io_op::flush()
{
   impl->flush();
}

bool pfm_file::io_op::reached_eof() const
{
   return impl->reached_eof();
}

void pfm_file::io_op::seek(uint64 ipos)
{
   impl->seek(ipos);
}

int pfm_file::io_op::read(std::vector<uint8>& ibuffer)
{
   return impl->read(ibuffer);
}

int pfm_file::io_op::write(const std::vector<uint8>& ibuffer)
{
   return impl->write(ibuffer);
}

int pfm_file::io_op::read(uint8* ibuffer, int isize)
{
   return impl->read(ibuffer, isize);
}

int pfm_file::io_op::write(const uint8* ibuffer, int isize)
{
   return impl->write(ibuffer, isize);
}


mws_sp<pfm_path> pfm_path::get_inst(std::string ifile_path, std::string i_aux_root_dir)
{
   struct make_shared_enabler : public pfm_path {};
   auto inst = std::make_shared<make_shared_enabler>();

   inst->filename = ifile_path;
   inst->aux_root_dir = i_aux_root_dir;
   inst->make_standard_path();

   return inst;
}

std::string pfm_path::get_full_path() const
{
   if (aux_root_dir.empty())
   {
      return filename;
   }

   return aux_root_dir + dir_separator + filename;
}

const std::string& pfm_path::get_file_name() const
{
   return filename;
}

std::string pfm_path::get_file_stem() const
{
   std::string filename = get_file_name();
   size_t idx = filename.find_last_of('.');

   if (idx != filename.npos)
   {
      return filename.substr(0, idx);
   }

   return filename;
}

std::string pfm_path::get_file_extension() const
{
   std::string filename = get_file_name();
   size_t idx = filename.find_last_of('.');

   if (idx != filename.npos)
   {
      return filename.substr(idx, filename.npos);
   }

   return "";
}

const std::string& pfm_path::get_root_directory() const
{
   return aux_root_dir;
}

mws_sp<std::vector<mws_sp<pfm_file> > > pfm_path::list_directory(mws_sp<mws_mod> i_mod, bool recursive) const
{
   auto file_list = std::make_shared<std::vector<mws_sp<pfm_file> > >();
   std::string base_dir = aux_root_dir;
   std::replace(base_dir.begin(), base_dir.end(), '\\', '/');

   if (mws_str::starts_with(base_dir, pfm_impl::get_common_res_path()) || (i_mod && mws_str::starts_with(base_dir, pfm_impl::get_mod_res_path(i_mod))))
   {
      list_directory_impl(base_dir, file_list, recursive);
   }
   else
   {
      {
         std::string base_dir_common = pfm_impl::get_common_res_path() + "/" + base_dir;
         std::replace(base_dir_common.begin(), base_dir_common.end(), '\\', '/');
         list_directory_impl(base_dir_common, file_list, recursive);
      }

      if (i_mod)
      {
         std::string base_dir_mod = pfm_impl::get_mod_res_path(i_mod) + "/" + base_dir;
         std::replace(base_dir_mod.begin(), base_dir_mod.end(), '\\', '/');
         list_directory_impl(base_dir_mod, file_list, recursive);
      }
   }

   struct pred
   {
      bool operator()(const mws_sp<pfm_file> a, const mws_sp<pfm_file> b) const
      {
         return a->creation_time() > b->creation_time();
      }
   };

   std::sort(file_list->begin(), file_list->end(), pred());

   return file_list;
}

void pfm_path::make_standard_path()
{
   // remove trailing directory separator
   if (!aux_root_dir.empty())
   {
      char lc = aux_root_dir.back();

      if (lc == '\\' || lc == '/')
      {
         aux_root_dir = aux_root_dir.substr(0, aux_root_dir.length() - 1);
      }
   }

   int idx1 = filename.find_last_of('\\');
   int idx2 = filename.find_last_of('/');

   if (idx1 >= 0 || idx2 >= 0)
   {
      int idx = (idx1 >= 0) ? idx1 : idx2;
      std::string file_root = filename.substr(0, idx);
      filename = filename.substr(idx + 1, filename.length() - idx - 1);

      if (!aux_root_dir.empty())
      {
         aux_root_dir = aux_root_dir + dir_separator + file_root;
      }
      else
      {
         aux_root_dir = file_root;
      }
   }
}
void pfm_path::list_directory_impl(std::string ibase_dir, mws_sp<std::vector<mws_sp<pfm_file> > > ifile_list, bool irecursive) const
{
   if (irecursive)
   {
      auto it = pfm_impl::res_files_map->begin();

      for (; it != pfm_impl::res_files_map->end(); it++)
      {
         mws_sp<pfm_file> file = it->second;
         std::string rdir = file->get_root_directory();

         if (mws_str::starts_with(rdir, ibase_dir))
         {
            ifile_list->push_back(file);
         }
      }
   }
   else
   {
      auto it = pfm_impl::res_files_map->begin();

      for (; it != pfm_impl::res_files_map->end(); it++)
      {
         mws_sp<pfm_file> file = it->second;

         if (file->get_root_directory() == ibase_dir)
         {
            ifile_list->push_back(file);
         }
      }
   }
}


pfm_data::pfm_data()
{
   screen_width = 1280;
   screen_height = 720;
   gfx_available = true;
}


pfm_data pfm::data;


mws_sp<pfm_main> pfm_main::gi()
{
   return pfm_app_inst;
}

void pfm_main::init() {}
void pfm_main::start() {}
void pfm_main::run() {}
float pfm_main::get_screen_scale() const { return 1.f; }
float pfm_main::get_screen_brightness() const { return 1.f; }
void pfm_main::set_screen_brightness(float i_brightness) {}
float pfm_main::get_screen_dpcm()const { return get_screen_dpi() / 2.54f; }

bool pfm_main::back_evt()
{
   return mws_mod_ctrl::inst()->back_evt();
}


int pfm::params::get_app_argument_count()
{
   return arg_count;
}

const unicodestring& pfm::params::get_app_path()
{
   return app_path;
}

const vector<unicodestring>& pfm::params::get_app_argument_vector()
{
   return arg_vector;
}

void pfm::params::set_app_arguments(int iargument_count, unicodechar** iargument_vector, bool iapp_path_included)
{
   int idx = 0;

   arg_count = iargument_count;

   if (iapp_path_included && arg_count > 0)
   {
      arg_count--;
      idx = 1;
      app_path.assign(iargument_vector[0]);
   }

   for (int k = 0; k < arg_count; k++, idx++)
   {
      arg_vector.push_back(iargument_vector[idx]);
   }
}


uint32 pfm::screen::get_width()
{
   return data.screen_width;
}

uint32 pfm::screen::get_height()
{
   return data.screen_height;
}

float pfm::screen::get_scale()
{
   return pfm_app_inst->get_screen_scale();
}

float pfm::screen::get_scaled_width()
{
   return get_width() * get_scale();
}

float pfm::screen::get_scaled_height()
{
   return get_height() * get_scale();
}

int pfm::screen::get_target_fps()
{
   return 30;
}

float pfm::screen::get_screen_dpi()
{
   return pfm_app_inst->get_screen_dpi();
}

bool pfm::screen::is_full_screen_mode()
{
   return pfm_app_inst->is_full_screen_mode();
}

void pfm::screen::set_full_screen_mode(bool ienabled)
{
   pfm_app_inst->set_full_screen_mode(ienabled);
}

bool pfm::screen::is_gfx_available()
{
   return data.gfx_available;
}

void pfm::screen::flip_screen()
{
   pfm_app_inst->flip_screen();
}


//shared_array<uint8> pfm::storage::load_res_byte_array(string i_filename, int& isize)
//{
//	shared_array<uint8> res;
//	path p(i_filename.c_str());
//
//	if(p.is_relative())
//	{
//		p = get_path(i_filename.c_str());
//	}
//
//	if(exists(p))
//	{
//		mws_sp<random_access_file> fs = get_random_access(p);
//		isize = file_size(p);
//
//		res = shared_array<uint8>(new uint8[isize]);
//		fs->read(res.get(), isize);
//	}
//
//	return res;
//}


const umf_list pfm::filesystem::get_res_file_list()
{
   return pfm_impl::res_files_map;
}

std::string pfm::filesystem::get_writable_path(std::string iname)
{
   std::string p;

   if (pfm::get_platform_id() == platform_windows_pc && pfm_impl::res_is_bundled_with_src())
   {
      auto mod = mws_mod_ctrl::inst()->get_current_mod();

      if (mod)
      {
         p = pfm_impl::get_mod_res_path(mod);
      }
      else
      {
         p = pfm_impl::get_common_res_path();
      }
   }
   else
   {
      p = pfm_app_inst->get_writable_path();
   }

   if (iname[0] == '/')
   {
      p += iname;
   }
   else
   {
      p = p + "/" + iname;
   }

   return p;
}

std::string pfm::filesystem::get_path(std::string iname)
{
   auto f = pfm_file::get_inst(iname);

   if (f)
   {
      return f->get_full_path();
   }

   return "";
}

void pfm::filesystem::load_res_file_map(mws_sp<mws_mod> i_mod)
{
   pfm_impl::res_files_map = std::make_shared<umf_r>();
   pfm_app_inst->get_directory_listing(pfm_impl::get_common_res_path(), pfm_impl::res_files_map, true);

   if (i_mod)
   {
      pfm_app_inst->get_directory_listing(pfm_impl::get_mod_res_path(i_mod), pfm_impl::res_files_map, true);
   }
}

mws_sp<std::vector<uint8> > pfm::filesystem::load_res_byte_vect(mws_sp<pfm_file> ifile)
{
   mws_sp<vector<uint8> > res;

   if (ifile->io.open())
   {
      int size = (int)ifile->length();

      res = mws_sp<vector<uint8> >(new vector<uint8>(size));
      ifile->io.read(begin_ptr(res), size);
      ifile->io.close();
   }

   return res;
}

mws_sp<std::vector<uint8> > pfm::filesystem::load_res_byte_vect(string i_filename)
{
   mws_sp<pfm_file> fs = pfm_file::get_inst(i_filename);

   return load_res_byte_vect(fs);
}

mws_sp<std::string> pfm::filesystem::load_res_as_string(mws_sp<pfm_file> ifile)
{
   mws_sp<std::string> text;

   if (ifile->io.open("rt"))
   {
      int size = (int)ifile->length();
      auto res = std::make_shared<vector<uint8> >(size);
      const char* res_bytes = (const char*)begin_ptr(res);
      int text_size = ifile->io.read(begin_ptr(res), size);

      ifile->io.close();
      text = std::make_shared<std::string>(res_bytes, text_size);
   }

   return text;
}

mws_sp<std::string> pfm::filesystem::load_res_as_string(std::string i_filename)
{
   mws_sp<pfm_file> fs = pfm_file::get_inst(i_filename);

   return load_res_as_string(fs);
}

//shared_array<uint8> pfm::storage::load_mod_byte_array(mws_sp<mws_mod> i_mod, string i_filename, int& isize)
//{
//	if(!i_mod)
//	{
//		return shared_array<uint8>();
//	}
//
//	i_filename = trs("mod-data/%1%-%2%") % i_mod->get_name() % i_filename;
//
//	return load_res_byte_array(i_filename, isize);
//}

mws_sp<std::vector<uint8> > pfm::filesystem::load_mod_byte_vect(mws_sp<mws_mod> i_mod, string i_filename)
{
   if (!i_mod)
   {
      return mws_sp<std::vector<uint8> >();
   }

   //i_filename = trs("mod-data/%1%-%2%") % i_mod->get_name() % i_filename;

   return load_res_byte_vect(i_filename);
}

bool pfm::filesystem::store_mod_byte_array(mws_sp<mws_mod> i_mod, string i_filename, const uint8* ires, int isize)
{
   if (!i_mod)
   {
      return false;
   }

   //i_filename = trs("mod-data/%1%-%2%") % i_mod->get_name() % i_filename;

   //path p = get_path(i_filename.c_str());
   //mws_sp<random_access_file> fs = get_random_access(p, true);

   //if(exists(p))
   //{
   //	fs->write(ires, isize);

   //	return true;
   //}

   return false;
}

bool pfm::filesystem::store_mod_byte_vect(mws_sp<mws_mod> i_mod, string i_filename, const vector<uint8>& ires)
{
   if (!i_mod)
   {
      return false;
   }

   //i_filename = trs("mod-data/%1%-%2%") % i_mod->get_name() % i_filename;

   //path p = get_path(i_filename.c_str());
   //mws_sp<random_access_file> fs = get_random_access(p, true);

   //if(exists(p))
   //{
   //	fs->write(begin_ptr(ires), ires.size());

   //	return true;
   //}

   return false;
}

mws_sp<pfm_file> pfm::filesystem::random_access(mws_sp<mws_mod> i_mod, std::string i_filename)
{
   if (!i_mod)
   {
      return mws_sp<pfm_file>();
   }

   //i_filename = trs("mod-data/%1%-%2%") % i_mod->get_name() % i_filename;

   //return get_random_access(get_path(i_filename.c_str()), true);
   return mws_sp<pfm_file>();
}

bool pfm::has_touchscreen()
{
   switch (get_platform_id())
   {
   case platform_android: return true;
   case platform_ios:  return true;
   case platform_emscripten: return false;
   case platform_qt_windows_pc: return false;
   case platform_windows_pc: return false;
   }

   return false;
}

mws_sp<pfm_main> pfm::get_pfm_main_inst()
{
   if (!pfm_impl::res_files_map)
   {
      pfm::filesystem::load_res_file_map();
   }

   return pfm_app_inst;
}


std::string mws_to_str(const char* i_format, ...)
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, i_format);
   vsnprintf(dest, 1024 * 16 - 1, i_format, arg_ptr);
   va_end(arg_ptr);

   return std::string(dest);
}

void mws_nl_impl()
{
   pfm::get_pfm_main_inst()->write_text("\n");
}

void mws_print_impl(const char* i_format, ...)
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, i_format);
   vsnprintf(dest, 1024 * 16 - 1, i_format, arg_ptr);
   va_end(arg_ptr);
   pfm::get_pfm_main_inst()->write_text(dest);
}


#if defined PLATFORM_WINDOWS_PC

//void* operator new(std::size_t isize, const std::nothrow_t& nothrow_value)
//{
//	void* ptr = 0;
//
//	if (isize > 0)
//	{
//		ptr = _aligned_malloc(isize, 16);
//
//		mws_assert(ptr);
//
//		if (ptr)
//		{
//			memset(ptr, 0, isize);
//		}
//	}
//
//	return ptr;
//}
//
//void* operator new[](std::size_t isize, const std::nothrow_t& nothrow_value)
//{
//	void* ptr = 0;
//
//	if (isize > 0)
//	{
//		ptr = _aligned_malloc(isize, 16);
//
//		mws_assert(ptr);
//
//		if (ptr)
//		{
//			memset(ptr, 0, isize);
//		}
//	}
//
//	return ptr;
//}
//
//void operator delete(void* iptr, const std::nothrow_t& nothrow_constant)
//{
//   _aligned_free(iptr);
//}
//
//void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant)
//{
//   _aligned_free(iptr);
//}

#endif
