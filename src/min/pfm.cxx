#include "stdafx.hxx"

#include "pfm.hxx"
#include "pfm-gl.h"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "min.hxx"
#include "mod-list.hxx"
#include "data-sequence.hxx"
#include "mws-vkb/mws-vkb.hxx"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>


mws_sp<mws_pfm_app> mws_app_inst();


// platform specific code
#if defined MWS_POSIX_API


const std::string dir_separator = "/";

uint32 mws::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      mws_app_inst()->write_text_nl("error");
   }

   return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool mws_path::make_dir() const
{
   int rval = mkdir(string().c_str(), 0777);
   return (rval == 0);
}

std::string mws_path::current_path()
{
   return "";
}


#endif


#if defined MWS_PFM_ANDROID


#include <sys/stat.h>


mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_android;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl_es;
}

#include <android/log.h>

// trace
#define os_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)
#define wos_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)



#elif defined MWS_PFM_IOS

#include <sys/stat.h>


const std::string dir_separator = "/";

mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_ios;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl_es;
}


#elif defined MWS_PFM_EMSCRIPTEN


const std::string dir_separator = "/";

mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_emscripten;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl_es;
}

uint32 mws::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      mws_app_inst()->write_text_nl("error");
   }

   return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

bool mws_path::make_dir() const
{
   return false;
}

std::string mws_path::current_path()
{
   return "";
}


#elif defined MWS_PFM_WINDOWS_PC


#include <direct.h>
#include <filesystem>


const std::string dir_separator = "\\";

//#ifdef USE_VLD
//#include <vld.h>
//#endif
//#include <new>

//void* operator new(std::size_t i_size, const std::nothrow_t& nothrow_value)
//{
//	void* ptr = 0;
//
//	if (i_size > 0)
//	{
//		ptr = _aligned_malloc(i_size, 16);
//
//		mws_assert(ptr);
//
//		if (ptr)
//		{
//			memset(ptr, 0, i_size);
//		}
//	}
//
//	return ptr;
//}
//
//void* operator new[](std::size_t i_size, const std::nothrow_t& nothrow_value)
//{
//	void* ptr = 0;
//
//	if (i_size > 0)
//	{
//		ptr = _aligned_malloc(i_size, 16);
//
//		mws_assert(ptr);
//
//		if (ptr)
//		{
//			memset(ptr, 0, i_size);
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

namespace std { const nothrow_t nothrow = nothrow_t(); }

mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_windows_pc;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl;
}

uint32 mws::time::get_time_millis()
{
   return GetTickCount();
}

bool mws_path::make_dir() const
{
   int rval = _mkdir(string().c_str());
   return (rval == 0);
}

std::string mws_path::current_path()
{
   return std::filesystem::current_path().generic_string();
}


#endif



namespace
{
   int arg_count = 0;
   unicode_string app_path;
   std::vector<unicode_string> arg_vector;
}


void mws_signal_error_impl(const char* i_file, uint32 i_line, const char* i_message)
{
#if defined MWS_DEBUG_BUILD

   const char* msg = (i_message) ? i_message : "signal error";
   mws_print_impl("[%s] at file [%s], line [%d]\n", msg, i_file, i_line);

#if defined MWS_PFM_WINDOWS_PC

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
   mws_signal_error(i_msg);
#endif
}


namespace mws_impl
{
   std::string res_files_mod_name;
   umf_list res_files_map;
   mws_sp<mws_log> mws_log_inst;
   bool mws_log_enabled = false;
   std::string mws_log_file_name = "app-log";

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
#if defined MWS_PFM_WINDOWS_PC && defined MWS_DEBUG_BUILD
      return true;
#endif

      return false;
   }

   mws_sp<mws_file> get_res_file(const std::string& i_filename)
   {
      mws_sp<mws_file> file;
      auto it = res_files_map->find(i_filename);

      if (it != res_files_map->end())
      {
         file = it->second;
      }

      return file;
   }

   void put_res_file(const std::string& i_filename, mws_sp<mws_file> i_file)
   {
      auto it = res_files_map->find(i_filename);

      if (it != res_files_map->end())
      {
         std::string msg = "duplicate filename: " + it->first;

         mws_throw mws_exception(msg.c_str());
      }

      (*res_files_map)[i_filename] = i_file;
   }

   mws_file_impl::mws_file_impl(const mws_path& i_path)
   {
      ppath = i_path;
      file_pos = 0;
      file_is_open = false;
      file_is_writable = false;
   }

   mws_file_impl::~mws_file_impl() {}

   bool mws_file_impl::exists()
   {
      if (is_opened())
      {
         return true;
      }

      if (open("r"))
      {
         close();

         return true;
      }

      return false;
   }

   bool mws_file_impl::is_opened() const
   {
      return file_is_open;
   }

   bool mws_file_impl::is_writable() const
   {
      return file_is_writable;
   }

   bool mws_file_impl::open(std::string i_open_mode)
   {
      file_is_open = open_impl(i_open_mode);

      return file_is_open;
   }

   void mws_file_impl::close()
   {
      if (file_is_open)
      {
         close_impl();
      }

      file_pos = 0;
      file_is_open = false;
   }

   void mws_file_impl::flush()
   {
      check_state();
      flush_impl();
   }

   bool mws_file_impl::reached_eof() const
   {
      check_state();
      FILE* f = get_file_impl();
      //long pos = ftell(f);
      int is_eof = feof(f);

      return is_eof != 0;
   }

   void mws_file_impl::seek(uint64 i_pos)
   {
      check_state();

      seek_impl(i_pos, SEEK_SET);
      file_pos = i_pos;
   }

   int mws_file_impl::read(std::vector<uint8>& i_buffer)
   {
      check_state();

      i_buffer.resize((size_t)length());

      return read(i_buffer.data(), i_buffer.size());
   }

   int mws_file_impl::write(const std::vector<uint8>& i_buffer)
   {
      check_state();

      return write(i_buffer.data(), i_buffer.size());
   }

   int mws_file_impl::read(uint8* i_buffer, int i_size)
   {
      check_state();

      int bytes_read = read_impl(i_buffer, i_size);

      return bytes_read;
   }

   int mws_file_impl::write(const uint8* i_buffer, int i_size)
   {
      check_state();

      int bytes_written = write_impl(i_buffer, i_size);

      return bytes_written;
   }

   void mws_file_impl::check_state() const
   {
      if (!file_is_open)
      {
         std::string msg = "file " + ppath.filename() + " is not open";

         mws_throw mws_exception(msg.c_str());
      }
   }

   void mws_file_impl::seek_impl(uint64 i_pos, int i_seek_pos)
   {
      fseek(get_file_impl(), (long)i_pos, i_seek_pos);
   }

   uint64 mws_file_impl::tell_impl()
   {
      return ftell(get_file_impl());
   }

   int mws_file_impl::read_impl(uint8* i_buffer, int i_size)
   {
      return fread(i_buffer, 1, i_size, get_file_impl());
   }

   int mws_file_impl::write_impl(const uint8* i_buffer, int i_size)
   {
      return fwrite(i_buffer, 1, i_size, get_file_impl());
   }
}
using namespace mws_impl;


mws_file::mws_file()
{
}

mws_file::~mws_file()
{
   io.close();
}

mws_sp<mws_file> mws_file::get_inst(const mws_path& i_path)
{
   mws_sp<mws_file> inst;

   // if res map initialized
   if (mws_impl::res_files_map)
   {
      auto file = mws_impl::get_res_file(i_path.filename());

      if (file)
      {
         // if i_path only contains a filename(no directories)
         if (i_path.directory().is_empty())
         {
            inst = file;
         }
         // if i_path also contains directories besides a filename,
         // check to see if it's a subpath within the found file's path
         else
         {
            std::string file_dir = file->directory().string();
            std::string path_dir = i_path.directory().string();

            if (file_dir.find(path_dir) != std::string::npos)
            {
               inst = file;
            }
         }
      }
   }

   // if path cannot be found inside the resource directories, this means it's external to the app
   if (!inst)
   {
      inst = mws_sp<mws_file>(new mws_file());
      inst->io.impl = mws_app_inst()->new_mws_file_impl(i_path);
   }

   return inst;
}

mws_sp<mws_file> mws_file::get_inst(mws_sp<mws_impl::mws_file_impl> i_impl)
{
   mws_sp<mws_file> inst;

   if (mws_impl::res_files_map)
      // res map initialized
   {
      inst = mws_impl::get_res_file(i_impl->ppath.filename());
   }

   if (!inst)
   {
      inst = mws_sp<mws_file>(new mws_file());
      inst->io.impl = i_impl;
   }

   return inst;
}

bool mws_file::exists() const
{
   return io.impl->exists();
}

bool mws_file::is_opened() const
{
   return io.impl->is_opened();
}

bool mws_file::is_writable() const
{
   return io.impl->is_writable();
}

uint64 mws_file::length()
{
   return io.impl->length();
}

uint64 mws_file::creation_time() const
{
   return io.impl->creation_time();
}

uint64 mws_file::last_write_time() const
{
   return io.impl->last_write_time();
}

const mws_path& mws_file::path() const
{
   return io.impl->ppath;
}

std::string mws_file::string_path() const
{
   return io.impl->ppath.string();
}

std::string mws_file::filename() const
{
   return io.impl->ppath.filename();
}

std::string mws_file::stem() const
{
   return io.impl->ppath.stem();
}

std::string mws_file::extension() const
{
   return io.impl->ppath.extension();
}

mws_path mws_file::directory() const
{
   return io.impl->ppath.directory();
}

FILE* mws_file::get_file_impl() const
{
   return io.impl->get_file_impl();
}

mws_file::io_op::io_op()
{
}

bool mws_file::io_op::open()
{
   bool file_opened = open("rb");

   return file_opened;
}

bool mws_file::io_op::open(std::string i_open_mode)
{
   if (impl->is_opened())
   {
      impl->close();
   }

   bool file_opened = impl->open(i_open_mode);

   if (!file_opened)
   {
      mws_print_impl("WARNING[ file [%s] NOT FOUND. ]\n", impl->ppath.string().c_str());
   }

   return file_opened;
}

void mws_file::io_op::close()
{
   impl->close();
}

void mws_file::io_op::flush()
{
   impl->flush();
}

bool mws_file::io_op::reached_eof() const
{
   return impl->reached_eof();
}

void mws_file::io_op::seek(uint64 i_pos)
{
   impl->seek(i_pos);
}

int mws_file::io_op::read(std::vector<uint8>& i_buffer)
{
   return impl->read(i_buffer);
}

int mws_file::io_op::write(const std::vector<uint8>& i_buffer)
{
   return impl->write(i_buffer);
}

int mws_file::io_op::read(uint8* i_buffer, int i_size)
{
   return impl->read(i_buffer, i_size);
}

int mws_file::io_op::write(const uint8* i_buffer, int i_size)
{
   return impl->write(i_buffer, i_size);
}


mws_path::mws_path() {}

mws_path::mws_path(const char* i_path, bool i_regular_path) : mws_path(std::string(i_path), i_regular_path) {}

mws_path::mws_path(const std::string& i_path, bool i_regular_path)
{
   path = i_path;
   regular_path = i_regular_path;
   make_standard_path();
}

mws_path::mws_path(const mws_path& i_path) : path(i_path.string()), regular_path(i_path.regular_path) {}

mws_path& mws_path::operator/=(const mws_path& i_path)
{
   if (!i_path.is_empty())
   {
      if (path.empty())
      {
         path = i_path.string();
      }
      else
      {
         if (i_path.string().front() == '/')
         {
            if (path.back() == '/')
            {
               // remove '/' from path so we're left with only one between the 2 paths
               path = path.substr(0, path.length() - 1) + i_path.string();
            }
            else
            {
               path += i_path.string();
            }
         }
         else
         {
            if (path.back() == '/')
            {
               path += i_path.string();
            }
            else
            {
               // add '/' to path to have a separator between the 2 paths
               path += "/" + i_path.string();
            }
         }
      }
   }

   // adding a non regular path makes the resulting path non regular also
   regular_path = regular_path && i_path.regular_path;

   return *this;
}

bool mws_path::remove() const
{
   mws_throw mws_exception("not implemented");

   return false;
}

bool mws_path::is_empty() const { return path.empty(); }

bool mws_path::is_directory() const
{
   struct stat info;

   if (stat(string().c_str(), &info) != 0)
   {
      return false;
   }

   if (info.st_mode & S_IFDIR)
   {
      return true;
   }

   return false;
}

bool mws_path::is_regular_file() const
{
   struct stat info;

   if (stat(string().c_str(), &info) != 0)
   {
      return false;
   }

   if (!(info.st_mode & S_IFDIR))
   {
      return true;
   }

   return false;
}

bool mws_path::exists() const
{
   // check if this is a resource file
   if (mws_impl::res_files_map)
   {
      auto pfile = mws_impl::get_res_file(filename());

      if (pfile)
      {
         return true;
      }
   }

   // external(non-resource file). use stat() to check for existence
   struct stat info;
   bool exists = (stat(string().c_str(), &info) == 0);

   return exists;
}

const std::string& mws_path::string() const { return path; }

std::string mws_path::filename() const
{
   auto pos_0 = path.find_last_of('/');
   int64 pos = (pos_0 != std::string::npos) ? pos_0 : -1;
   size_t idx = size_t(pos + 1);

   return std::string(path.begin() + idx, path.end());
}

std::string mws_path::stem() const
{
   std::string name = filename();
   size_t idx = name.find_last_of('.');

   if (idx != name.npos)
   {
      return name.substr(0, idx);
   }

   return name;
}

std::string mws_path::extension() const
{
   std::string name = filename();
   size_t idx = name.find_last_of('.');

   if (idx != name.npos && idx > 0)
   {
      return name.substr(idx, name.npos);
   }

   return name;
}

mws_path mws_path::directory() const
{
   // if path is a file, return parent path
   if (!filename().empty())
   {
      return parent_path();
   }

   return *this;
}

mws_path mws_path::parent_path() const
{
   size_t pos_0 = path.find_last_of('/');
   int64 pos = -1;

   if (pos_0 == std::string::npos)
   {
      return mws_path("", regular_path);
   }
   else if (pos_0 != std::string::npos)
   {
      pos = (int64)pos_0 + 1;
   }

   return mws_path(std::string(path.begin(), path.begin() + (size_t)pos), regular_path);
}

mws_sp<std::vector<mws_sp<mws_file>>> mws_path::list_directory(bool i_recursive) const
{
   auto file_list = std::make_shared<std::vector<mws_sp<mws_file>> >();
   std::string base_dir = directory().string();

   if (mws_str::starts_with(base_dir, mws::filesys::res_dir().string()))
   {
      list_directory_impl(base_dir, file_list, i_recursive);
   }
   else
   {
      std::string base_dir_mod = mws::filesys::res_dir().string() + "/" + base_dir;
      list_directory_impl(base_dir_mod, file_list, i_recursive);
   }

   struct pred
   {
      bool operator()(const mws_sp<mws_file> a, const mws_sp<mws_file> b) const
      {
         return a->creation_time() > b->creation_time();
      }
   };

   std::sort(file_list->begin(), file_list->end(), pred());

   return file_list;
}

bool mws_path::is_regular_path() const { return regular_path; }

void mws_path::make_standard_path()
{
   std::replace(path.begin(), path.end(), '\\', '/');
}

void mws_path::list_directory_impl(std::string i_base_dir, mws_sp<std::vector<mws_sp<mws_file>> > i_file_list, bool i_recursive) const
{
   if (i_recursive)
   {
      auto it = mws_impl::res_files_map->begin();

      for (; it != mws_impl::res_files_map->end(); ++it)
      {
         mws_sp<mws_file> file = it->second;
         std::string rdir = file->directory().string();

         if (mws_str::starts_with(rdir, i_base_dir))
         {
            i_file_list->push_back(file);
         }
      }
   }
   else
   {
      auto it = mws_impl::res_files_map->begin();

      for (; it != mws_impl::res_files_map->end(); ++it)
      {
         mws_sp<mws_file> file = it->second;

         if (file->directory().string() == i_base_dir)
         {
            i_file_list->push_back(file);
         }
      }
   }
}

mws_path operator/(const mws_path& i_lhs, const mws_path& i_rhs)
{
   mws_path path = i_lhs;
   path /= i_rhs;

   return path;
}



void mws_pfm_app::init()
{
   mws_mod_ctrl::inst()->init_app();
}

void mws_pfm_app::start()
{
   mws_mod_ctrl::inst()->start_app();
}

void mws_pfm_app::run()
{
   mws_mod_ctrl::inst()->update();
}

mws_key_types mws_pfm_app::apply_key_modifiers(mws_key_types i_key_id) const
{
   if (mod_mws_vkb_on)
   {
      mws_sp<mws_vkb> vkb = mws_vkb::gi();

      if (vkb && vkb->is_visible())
      {
         return vkb->apply_key_modifiers(i_key_id);
      }
   }

   return apply_key_modifiers_impl(i_key_id);
}

bool mws_pfm_app::back_evt() const
{
   return mws_mod_ctrl::inst()->back_evt();
}

float mws_pfm_app::get_screen_scale() const { return 1.f; }
float mws_pfm_app::get_screen_brightness() const { return 1.f; }
void mws_pfm_app::set_screen_brightness(float i_brightness) const {}
void mws_pfm_app::flip_screen() const {}
float mws_pfm_app::get_avg_screen_dpcm() const { return get_avg_screen_dpi() / 2.54f; }


// filesystem
const std::string mws::filesys::res_idx_name = "index.txt";

const umf_list mws::filesys::get_res_file_list()
{
   return mws_impl::res_files_map;
}

void mws_mod_ctrl::pre_init_app()
{
   if (!ul)
   {
      ul = mws_mod_list::nwi();
      ul->set_name("app-mws-mod-list");
      mws_mod_setup::next_crt_mod = crt_mod = ul;

      mws_mod_setup::append_mod_list(ul);
   }

   mws_sp<mws_mod> start_mod = get_app_start_mod();

   if (start_mod)
   {
      mws_app_inst()->reconfigure_directories(start_mod);
   }
}

void mws_mod_ctrl::set_current_mod(mws_sp<mws_mod> i_mod)
{
   if (i_mod)
   {
      if (!crt_mod.expired())
      {
         crt_mod.lock()->base_unload();
      }

      crt_mod = i_mod;
      mws_app_inst()->reconfigure_directories(i_mod);

      // reload resources
      if (mws_impl::res_files_mod_name != i_mod->get_name())
      {
         mws_impl::res_files_mod_name = i_mod->get_name();
         mws_impl::res_files_map = std::make_shared<umf_r>();
         mws_app_inst()->get_directory_listing(mws::filesys::res_dir().string(), mws_impl::res_files_map, true);
      }

      if (!i_mod->is_init())
      {
         i_mod->base_init();
         i_mod->set_init(true);
      }

      i_mod->base_load();
   }
   else
   {
      mws_signal_error("warning: tried to make current a null mws_mod");
   }
}

mws_sp<std::vector<uint8>> mws::filesys::load_res_byte_vect(mws_sp<mws_file> i_file)
{
   mws_sp<std::vector<uint8>> res;

   if (i_file->io.open())
   {
      int size = (int)i_file->length();

      res = mws_sp<std::vector<uint8>>(new std::vector<uint8>(size));
      i_file->io.read(res->data(), size);
      i_file->io.close();
   }

   return res;
}

mws_sp<std::vector<uint8>> mws::filesys::load_res_byte_vect(std::string i_filename)
{
   mws_sp<mws_file> fs = mws_file::get_inst(i_filename);

   return load_res_byte_vect(fs);
}

mws_sp<std::string> mws::filesys::load_res_as_string(mws_sp<mws_file> i_file)
{
   mws_sp<std::string> text;

   if (i_file->io.open("rt"))
   {
      int size = (int)i_file->length();
      auto res = std::make_shared<std::vector<uint8>>(size);
      const char* res_bytes = (const char*)res->data();
      int text_size = i_file->io.read(res->data(), size);

      i_file->io.close();
      text = std::make_shared<std::string>(res_bytes, text_size);
   }

   return text;
}

mws_sp<std::string> mws::filesys::load_res_as_string(std::string i_filename)
{
   mws_sp<mws_file> fs = mws_file::get_inst(i_filename);

   return load_res_as_string(fs);
}

const mws_path& mws::filesys::prv_dir()
{
   return mws_app_inst()->prv_dir();
}

const mws_path& mws::filesys::res_dir()
{
   return mws_app_inst()->res_dir();
}

const mws_path& mws::filesys::tmp_dir()
{
   return mws_app_inst()->tmp_dir();
}


// input
mws_key_types mws::input::translate_key(int i_pfm_key_id) { return mws_app_inst()->translate_key(i_pfm_key_id); }

mws_key_types mws::input::apply_key_modifiers(mws_key_types i_key_id) { return mws_app_inst()->apply_key_modifiers(i_key_id); }

bool mws::input::has_touchscreen()
{
   switch (get_platform_id())
   {
   case mws_pfm_android: return true;
   case mws_pfm_ios:  return true;
   case mws_pfm_emscripten: return false;
   case mws_pfm_windows_pc: return false;
   }

   return false;
}

bool mws::input::uses_touchscreen()
{
   mws_sp<mws_mod> mod = mws_mod_ctrl::inst()->get_current_mod();
   bool uses_touchscreen = (mod) ? mod->get_preferences()->emulate_mobile_screen() : false;

   return has_touchscreen() || uses_touchscreen;
}


// output
void mws::output::write_text(const char* i_text) { mws_app_inst()->write_text(i_text); }
void mws::output::write_text_nl(const char* i_text) { mws_app_inst()->write_text_nl(i_text); }
void mws::output::write_text(const wchar_t* i_text) { mws_app_inst()->write_text(i_text); }
void mws::output::write_text_nl(const wchar_t* i_text) { mws_app_inst()->write_text_nl(i_text); }


// params
int mws::args::get_app_argument_count()
{
   return arg_count;
}

const unicode_string& mws::args::get_app_path()
{
   return app_path;
}

const std::vector<unicode_string>& mws::args::get_app_argument_vector()
{
   return arg_vector;
}

void mws::args::set_app_arguments(int i_argument_count, unicode_char** i_argument_vector, bool i_app_path_included)
{
   int idx = 0;

   arg_count = i_argument_count;

   if (i_app_path_included && arg_count > 0)
   {
      arg_count--;
      idx = 1;
      app_path.assign(i_argument_vector[0]);
   }

   for (int k = 0; k < arg_count; k++, idx++)
   {
      arg_vector.push_back(i_argument_vector[idx]);
   }
}


// screen
bool mws::screen::is_gfx_available() { return mws_mod_ctrl::is_gfx_available(); }
int mws::screen::get_target_fps() { return 30; }
void mws::screen::flip_screen() { mws_app_inst()->flip_screen(); }
bool mws::screen::is_full_screen_mode() { return mws_app_inst()->is_full_screen_mode(); }
void mws::screen::set_full_screen_mode(bool i_enabled) { mws_app_inst()->set_full_screen_mode(i_enabled); }
float mws::screen::get_brightness() { return mws_app_inst()->get_screen_brightness(); }
void mws::screen::set_brightness(float i_brightness) { mws_app_inst()->set_screen_brightness(i_brightness); }
uint32 mws::screen::get_width() { return mws_mod_ctrl::get_screen_width(); }
uint32 mws::screen::get_height() { return mws_mod_ctrl::get_screen_height(); }
float mws::screen::get_scale() { return mws_app_inst()->get_screen_scale(); }
float mws::screen::get_scaled_width() { return get_width() * get_scale(); }
float mws::screen::get_scaled_height() { return get_height() * get_scale(); }
std::pair<uint32, uint32> mws::screen::get_res_px() { return mws_app_inst()->get_screen_res_px(); }
float mws::screen::get_avg_dpi() { return mws_app_inst()->get_avg_screen_dpi(); }
std::pair<float, float> mws::screen::get_dpi() { return mws_app_inst()->get_screen_dpi(); }
std::pair<float, float> mws::screen::get_dim_inch() { return mws_app_inst()->get_screen_dim_inch(); }
float mws::screen::get_avg_dpcm() { return mws_app_inst()->get_avg_screen_dpcm(); }
std::pair<float, float> mws::screen::get_dpcm() { return mws_app_inst()->get_screen_dpcm(); }
std::pair<float, float> mws::screen::get_dim_cm() { return mws_app_inst()->get_screen_dim_cm(); }


// time
std::string mws::time::get_timezone_id()
{
   return mws_app_inst()->get_timezone_id();
}

std::string mws::time::get_current_date(const std::string& i_fmt)
{
   static const char* def_fmt = "%a %b %d %H:%M:%S %Y";
   const char* fmt = (i_fmt.empty()) ? def_fmt : i_fmt.c_str();
   std::time_t t = std::time(nullptr);
#pragma warning(suppress : 4996)
   std::tm tm = *std::localtime(&t);
   std::stringstream ss;

   ss << std::put_time(&tm, fmt);

   auto s = ss.str();

   return s;
}

std::string mws::time::get_duration_as_string(uint32 i_duration)
{
   std::string duration;

   uint32 millis = i_duration % 1000;
   uint32 seconds = i_duration / 1000;
   uint32 minutes = seconds / 60;
   uint32 seconds_remainder = seconds % 60;

   duration += std::to_string(minutes);
   duration += ":";
   duration += std::to_string(seconds_remainder);
   //duration += ":";
   //duration += std::to_string(millis);

   return duration;
}


class mws_log_impl : public mws_log
{
public:
   mws_log_impl()
   {
      //auto leapseconds_file = mws_file::get_inst("leapseconds");
      //std::string tzdb_path = leapseconds_file->directory().string();
      //date::set_install(tzdb_path);

      load();
   }

   virtual const std::vector<std::string> get_log() override
   {
      std::lock_guard<std::mutex> lock(sync_mx);
      return log;
   }

   virtual void push(const char* i_msg) override
   {
      std::lock_guard<std::mutex> lock(sync_mx);
      push_impl(i_msg);
   }

   virtual void pushf(const char* i_fmt, ...) override
   {
      char dest[16000];
      va_list arg_ptr;

      va_start(arg_ptr, i_fmt);
      vsnprintf(dest, 16000 - 1, i_fmt, arg_ptr);
      va_end(arg_ptr);

      push(dest);
   }

   virtual void clear() override
   {
      std::lock_guard<std::mutex> lock(sync_mx);
      check_log_file();

      if (log_file->is_opened())
      {
         log_file->io.close();
      }

      log_file->io.open("wb");
      log_file->io.close();

      log.clear();

      if (text_buffer)
      {
         text_buffer->clear();
      }

//#ifdef MWS_DEBUG_BUILD
//
//      if (app_i())
//      {
//         im& app_im = app_i()->i_m<im>();
//
//         if (app_im.log_pg->text_box)
//         {
//            app_im.log_pg->text_box->set_text("");
//         }
//      }
//
//#endif
   }

   virtual void set_text_buffer(mws_sp<mws_text_buffer> i_text_buffer) override
   {
      text_buffer = i_text_buffer;
   }

private:
   void load()
   {
      std::lock_guard<std::mutex> lock(sync_mx);
      mws_path log_path = mws::filesys::tmp_dir() / mws_log_file_name;
      log_file = mws_file::get_inst(log_path);

      if (log_file && log_file->exists())
      {
         log_file->io.open("rb");

         auto res_rw = rw_file_sequence::nwi(log_file);
         uint64 file_length = log_file->length();

         while (res_rw->get_read_position() < file_length)
         {
            std::string line = res_rw->r.read_string();
            log.push_back(line);
         }

         log_file->io.close();
      }
   }

   void push_impl(const char* i_msg)
   {
      //if (app_i() && !app_i()->i_m_is_null())
      //{
      //   auto st = app_settings::i();
      //   auto tz = st->get_current_timezone();
      //   auto tp_now = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
      //   std::string date = time_utils::get_date(tp_now, tz);
      //   std::string msg = date + "[ " + i_msg + std::string(" ]\n");

      //   push_verbatim(msg);
      //}
      //else
      //{
      //   trx("error[ mws_mod instance is null. msg[ {} ] ]", i_msg);
      //}
      std::string msg = mws::time::get_current_date("%Y-%m-%d %H:%M:%S") + "[ " + i_msg + std::string(" ]\n");
      push_verbatim(msg);
   }

   void push_verbatim(const std::string& i_msg)
   {
      check_log_file();
      log.push_back(i_msg);

      if (log_file && log_file->is_opened())
      {
         auto res_rw = rw_file_sequence::nwi(log_file);

         res_rw->w.write_string(i_msg);
         log_file->io.flush();
      }

      if (text_buffer)
      {
         text_buffer->push_front(i_msg.c_str());
      }
     
//#ifdef MWS_DEBUG_BUILD
//
//      if (app_i() && !app_i()->i_m_is_null())
//      {
//         im& app_im = app_i()->i_m<im>();
//
//         if (app_im.log_pg)
//         {
//            if (!console_active)
//            {
//               if (app_im.log_pg->text_box)
//               {
//                  std::string buf;
//                  console_active = true;
//
//                  // most recent entries are top most in the console
//                  for (int k = log.size() - 1; k >= 0; k--)
//                  {
//                     buf += log[k];
//                  }
//
//                  app_im.log_pg->text_box->set_text(buf);
//               }
//            }
//            else
//            {
//               if (app_im.log_pg->text_box)
//               {
//                  app_im.log_pg->text_box->push_front_text(i_msg);
//               }
//            }
//         }
//      }
//
//#endif
   }

   void check_log_file()
   {
      if (!log_file)
      {
         mws_path log_path = mws::filesys::tmp_dir() / mws_log_file_name;
         log_file = mws_file::get_inst(log_path);
      }

      if (log_file)
      {
         if (!log_file->is_opened())
         {
            log_file->io.open("ab");
         }
      }
   }

   bool console_active = false;
   std::vector<std::string> log;
   mws_sp<mws_file> log_file;
   std::mutex sync_mx;
   mws_sp<mws_text_buffer> text_buffer;
};


bool mws_log::is_enabled() { return mws_log_enabled; }

void mws_log::set_enabled(bool i_is_enabled)
{
   mws_log_enabled = i_is_enabled;
   mws_log_inst = nullptr;
}

mws_sp<mws_log> mws_log::i()
{
   if (!mws_log_inst)
   {
      if (mws_log_enabled)
      {
         mws_log_inst = mws_sp<mws_log>(new mws_log_impl());
      }
      else
      {
         mws_log_inst = mws_sp<mws_log>(new mws_log());
      }
   }

   return mws_log_inst;
}

const std::vector<std::string> mws_log::get_log()
{
   if (mws_log_enabled)
   {
      return mws_log_inst->get_log();
   }

   return std::vector<std::string>();
}

void mws_log::push(const char*) {}
void mws_log::pushf(const char*, ...) {}
void mws_log::clear() {}
void mws_log::set_text_buffer(mws_sp<mws_text_buffer> i_text_buffer) {}
mws_log::mws_log() {}


std::string mws_to_str_fmt(const char* i_format, ...)
{
   char dest[16000];
   va_list arg_ptr;

   va_start(arg_ptr, i_format);
   vsnprintf(dest, 16000 - 1, i_format, arg_ptr);
   va_end(arg_ptr);

   return std::string(dest);
}

void mws_nl_impl()
{
   mws_app_inst()->write_text("\n");
}

void mws_print_impl(const char* i_format, ...)
{
   char dest[16000];
   va_list arg_ptr;

   va_start(arg_ptr, i_format);
   vsnprintf(dest, 16000 - 1, i_format, arg_ptr);
   va_end(arg_ptr);
   mws_app_inst()->write_text(dest);
}
