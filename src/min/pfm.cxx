#include "stdafx.hxx"

#include "pfm.hxx"
#include "mws-impl.hxx"
#include "pfm-gl.h"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "min.hxx"
#include "mod-list.hxx"
#include "mws-vkb/mws-vkb.hxx"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <ctime>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>


mws_sp<mws_app> mws_app_inst();


// platform specific code
#if defined MWS_POSIX_API


#include <dirent.h>


uint32_t mws::time::get_time_millis()
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
   int ret_val = mkdir(string().c_str(), 0777);

   return (ret_val == 0);
}

std::string mws_path::current_path()
{
   return "";
}

static void mws_list_external_directory(const mws_path& i_directory, std::vector<mws_sp<mws_file>>& i_file_list, bool i_recursive)
{
   DIR* dir = opendir(i_directory.string().c_str());

   if (dir != nullptr)
   {
      dirent* ent = readdir(dir);

      while (ent != nullptr)
      {
         if (ent->d_type == DT_REG)
         {
            std::string filename(ent->d_name);
            mws_path path = i_directory / filename;
            mws_sp<mws_file_impl> file_impl = mws_app_inst()->new_mws_file_impl(path);

            i_file_list.push_back(mws_file::get_inst(file_impl));
         }
         else if (ent->d_type == DT_DIR && i_recursive && ent->d_name[0] != '.')
         {
            mws_path sub_dir = i_directory / ent->d_name;

            mws_list_external_directory(sub_dir, i_file_list, true);
         }

         ent = readdir(dir);
      }

      closedir(dir);
   }
   else
   {
      mws_println("WARNING[ could not open directory [ %s ] ]", i_directory.string().c_str());
   }
}

std::vector<mws_sp<mws_file>> mws_app::list_external_directory(const mws_path& i_directory, bool i_recursive) const
{
   std::vector<mws_sp<mws_file>> list;

   mws_list_external_directory(i_directory, list, i_recursive);

   return list;
}


#else


std::vector<mws_sp<mws_file>> mws_app::list_external_directory(const mws_path& i_directory, bool i_recursive) const
{
   return std::vector<mws_sp<mws_file>>();
}


#endif


#if defined MWS_PFM_ANDROID


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


mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_ios;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl_es;
}


#elif defined MWS_PFM_EMSCRIPTEN


mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_emscripten;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl_es;
}

uint32_t mws::time::get_time_millis()
{
   struct timespec ts;

   if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
   {
      mws_app_inst()->write_text_nl("error");
   }

   return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}


#elif defined MWS_PFM_WINDOWS_PC


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
//
//namespace std { const nothrow_t nothrow = nothrow_t(); }

mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_windows_pc;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl;
}

uint32_t mws::time::get_time_millis()
{
   return GetTickCount();
}

#elif defined MWS_PFM_LINUX

mws_pfm_id mws::get_platform_id()
{
   return mws_pfm_linux;
}

mws_gfx_type mws::get_gfx_type_id()
{
   return mws_gfx_opengl;
}

#endif


void mws_signal_error_impl(const char* i_file, uint32_t i_line, const char* i_message)
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

void mws_assert_impl(const char* i_file, uint32_t i_line, bool i_condition)
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


mws_file_map mws_res_index::read_file_map(mws_sp<mws_file> i_index_file)
{
   mws_file_map file_map;
   const bool is_internal = true;
   const mws_path& res_dir = mws_app_inst()->res_dir();

   i_index_file->io.open("rb");

   if (i_index_file->is_open())
   {
      mws_sp<mws_rw_file_seqv> fs = mws_rw_file_seqv::nwi(i_index_file, false);
      uint32_t size = fs->r.read_u32();

      for (uint32_t k = 0; k < size; k++)
      {
         std::string file_path = fs->r.read_text();
         mws_path path = res_dir / file_path;
         std::string filename = path.filename();

         // check for duplicate file names
         mws_assert(file_map.find(filename) == file_map.end());
         file_map[filename] = mws_file::get_inst(mws_app_inst()->new_mws_file_impl(path, is_internal));
      }

      file_map[mws_app::res_idx_name()] = i_index_file;
      i_index_file->io.close();
   }
   else
   {
      mws_println("mws_res_index::read_file_map[ no resources found ]");
   }

   return file_map;
}

void mws_res_index::write_file_map(mws_sp<mws_file> i_index_file, const mws_file_map& i_file_map)
{
   i_index_file->io.open("wb");
   mws_sp<mws_rw_file_seqv> fs = mws_rw_file_seqv::nwi(i_index_file, true);

   fs->w.write_u32(i_file_map.size());

   for (auto it : i_file_map)
   {
      std::string path = it.second->string_path();

      fs->w.write_text(path);
   }

   i_index_file->io.close();
}


mws_file_impl::mws_file_impl(const mws_path& i_path, bool i_is_internal)
{
   file_is_internal = i_is_internal;
   ppath = i_path;
}

mws_file_impl::~mws_file_impl() {}

bool mws_file_impl::exists()
{
   if (is_open())
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

bool mws_file_impl::is_open() const
{
   return file_is_open;
}

bool mws_file_impl::is_writable() const
{
   return file_is_writable;
}

bool mws_file_impl::is_internal() const
{
   return file_is_internal;
}

bool mws_file_impl::open(std::string i_open_mode)
{
   open_mode = i_open_mode;
   file_is_open = open_impl(i_open_mode);

   if (file_is_open && !file_is_writable)
   {
      check_file_is_writable();
   }

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

uint64_t mws_file_impl::io_position() const { return file_pos; }

void mws_file_impl::set_io_position(uint64_t i_pos)
{
   check_state();

   set_io_position_impl(i_pos, SEEK_SET);
   file_pos = i_pos;
}

int mws_file_impl::read(std::vector<std::byte>& i_buffer)
{
   check_state();

   i_buffer.resize((size_t)length());

   return read(i_buffer.data(), i_buffer.size());
}

int mws_file_impl::write(const std::vector<std::byte>& i_buffer)
{
   check_state();

   return write(i_buffer.data(), i_buffer.size());
}

int mws_file_impl::read(std::byte* i_buffer, uint32_t i_size)
{
   check_state();

   int bytes_read = read_impl(i_buffer, i_size);

   return bytes_read;
}

int mws_file_impl::write(const std::byte* i_buffer, uint32_t i_size)
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

void mws_file_impl::set_io_position_impl(uint64_t i_pos, int i_io_pos)
{
   fseek(get_file_impl(), static_cast<long>(i_pos), i_io_pos);
}

uint64_t mws_file_impl::tell_impl()
{
   return ftell(get_file_impl());
}

int mws_file_impl::read_impl(std::byte* i_buffer, uint32_t i_size)
{
   return fread(i_buffer, 1, i_size, get_file_impl());
}

int mws_file_impl::write_impl(const std::byte* i_buffer, uint32_t i_size)
{
   return fwrite(i_buffer, 1, i_size, get_file_impl());
}

void mws_file_impl::check_file_is_writable()
{
   bool file_opened = get_file_impl();

   if (file_opened && (open_mode.find('a') != std::string::npos || open_mode.find('w') != std::string::npos))
   {
      file_is_writable = true;
   }
}


namespace mws_impl
{
   mws_sp<mws_log> mws_log_inst;
   bool mws_log_enabled = false;
   std::string mws_log_file_name = "app-log";

   //true if res is in the same dir as src
   bool res_is_bundled_with_src()
   {
#if defined MWS_PFM_WINDOWS_PC && defined MWS_DEBUG_BUILD
      return true;
#else
      return false;
#endif
   }

   mws_sp<mws_file> get_res_file(const mws_file_map& i_file_map, const std::string& i_filename)
   {
      mws_sp<mws_file> file;
      auto it = i_file_map.find(i_filename);

      if (it != i_file_map.end())
      {
         file = it->second;
      }

      return file;
   }

   // lists a directory located inside the resources directory
   std::vector<mws_sp<mws_file>> list_res_directory(const mws_path& i_base_dir, bool i_recursive)
   {
      std::vector<mws_sp<mws_file>> file_list;
      mws_sp<mws_mod> mod = mws_mod_ctrl::inst()->get_current_mod();
      const mws_file_map& file_map = mod->storage.get_res_file_list();

      if (i_recursive)
      {
         auto it = file_map.begin();

         for (; it != file_map.end(); ++it)
         {
            mws_sp<mws_file> file = it->second;
            std::string rdir = file->directory().string();

            if (mws_str::starts_with(rdir, i_base_dir.string()))
            {
               file_list.push_back(file);
            }
         }
      }
      else
      {
         auto it = file_map.begin();

         for (; it != file_map.end(); ++it)
         {
            mws_sp<mws_file> file = it->second;

            if (file->directory().string() == i_base_dir.string())
            {
               file_list.push_back(file);
            }
         }
      }

      return file_list;
   }

}
using namespace mws_impl;


mws_path::mws_path() {}

mws_path::mws_path(const char* i_path) : mws_path(std::string(i_path)) {}

mws_path::mws_path(const std::string& i_path)
{
   path = i_path;
   make_standard_path();
}

mws_path::mws_path(const mws_path& i_path) : path(i_path.string()) {}

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

   if (is_empty())
   {
      return false;
   }

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

   if (is_empty())
   {
      return false;
   }

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

bool mws_path::is_absolute() const
{
   if (path.empty())
   {
      return false;
   }

#if defined MWS_PFM_WINDOWS_PC
   return (path.size() > 2 && (std::isalpha(path[0])) && (path[1] == ':') && (path[2] == '/'));
#else
   return (path[0] == '/');
#endif
}

bool mws_path::is_relative() const
{
   return !is_absolute();
}

bool mws_path::exists() const
{
   mws_sp<mws_mod> mod = mws_mod_ctrl::inst()->get_current_mod();
   const mws_file_map& file_map = mod->storage.get_res_file_list();

   // check if this is a resource file
   if (!file_map.empty())
   {
      mws_sp<mws_file> file = mws_impl::get_res_file(file_map, filename());

      if (file)
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
   int64_t pos = (pos_0 != std::string::npos) ? pos_0 : -1;
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
   if (path.length() <= 1)
   {
      if (path[0] == '/')
      {
         return *this;
      }

      return mws_path("");
   }

   // consider the case when last char is '/'
   size_t pos_0 = path.find_last_of('/', path.length() - 2);
   int64_t pos = -1;

   if (pos_0 == std::string::npos)
   {
      return mws_path("");
   }
   else if (pos_0 != std::string::npos)
   {
      pos = (int64_t)pos_0 + 1;
   }

   return mws_path(std::string(path.begin(), path.begin() + (size_t)pos));
}

std::vector<mws_sp<mws_file>> mws_path::list_directory(bool i_recursive) const
{
   std::vector<mws_sp<mws_file>> file_list;

   // if file's directory is empty, search resource files for this filename
   if (directory().is_empty())
   {
      std::string file_name = filename();

      // if the file's name is not empty, find it and list files in that directory
      if (!file_name.empty())
      {
         mws_sp<mws_file> file = mws_file::get_inst(file_name);

         file_list = list_res_directory(file->directory(), i_recursive);
      }
   }
   else
   {
      // if path is absolute, it's considered external(not inside the resources directory), so call the external directory listing function
      if (is_absolute())
      {
         file_list = mws_app_inst()->list_external_directory(*this, i_recursive);
      }
      // if the path is relative, check if the path starts with the resouces directory path
      else
      {
         const mws_path& res_dir = mws_app_inst()->res_dir();

         // if the path starts with the resouces directory path, then search inside it
         if (mws_str::starts_with(string(), res_dir.string()))
         {
            file_list = list_res_directory(*this, i_recursive);
         }
         // if not, it's probably an external path after all. try the external directory listing function
         else
         {
            file_list = mws_app_inst()->list_external_directory(*this, i_recursive);
         }
      }
   }

   const auto cmp = [](const mws_sp<mws_file>& i_left, const mws_sp<mws_file>& i_right)
   {
      return i_left->creation_time() > i_right->creation_time();
   };

   std::sort(file_list.begin(), file_list.end(), cmp);

   return file_list;
}

bool mws_path::is_internal() const
{
   if (is_empty())
   {
      return false;
   }

   // if we only have a filename, check if it's inside the resource directory
   if (directory().is_empty())
   {
      mws_sp<mws_file> file = mws_file::get_inst(filename());

      if (file->is_internal())
      {
         return true;
      }
   }

   const mws_path& res_dir = mws_app_inst()->res_dir();

   return mws_str::starts_with(path, res_dir.string());
}

void mws_path::make_standard_path() { std::replace(path.begin(), path.end(), '\\', '/'); }

mws_path operator/(const mws_path& i_lhs, const mws_path& i_rhs)
{
   mws_path path = i_lhs;
   path /= i_rhs;

   return path;
}


mws_file::mws_file() {}
mws_file::~mws_file() { io.close(); }

mws_sp<mws_file> mws_file::get_inst(const mws_path& i_path)
{
   mws_sp<mws_file> inst;
   mws_sp<mws_mod> mod = mws_mod_ctrl::inst()->get_current_mod();
   const mws_file_map& file_map = mod->storage.get_res_file_list();

   // if res map initialized
   if (!file_map.empty())
   {
      mws_path file_dir = i_path.directory();

      if (file_dir.is_empty())
      {
         inst = mws_impl::get_res_file(file_map, i_path.filename());
      }
   }

   // this is an external file
   if (!inst)
   {
      inst = mws_sp<mws_file>(new mws_file());
      inst->io.impl = mws_app_inst()->new_mws_file_impl(i_path);
   }

   return inst;
}

mws_sp<mws_file> mws_file::get_inst(mws_sp<mws_file_impl> i_impl)
{
   mws_sp<mws_file> inst;
   mws_sp<mws_mod> mod = mws_mod_ctrl::inst()->get_current_mod();
   const mws_file_map& file_map = mod->storage.get_res_file_list();

   // if res map initialized
   if (!file_map.empty())
   {
      mws_path file_dir = i_impl->ppath.directory();

      if (file_dir.is_empty())
      {
         inst = mws_impl::get_res_file(file_map, i_impl->ppath.filename());
      }
   }

   // this is an external file
   if (!inst)
   {
      inst = mws_sp<mws_file>(new mws_file());
      inst->io.impl = i_impl;
   }

   return inst;
}

bool mws_file::exists() const { return io.impl->exists(); }
bool mws_file::is_open() const { return io.impl->is_open(); }
bool mws_file::is_writable() const { return io.impl->is_writable(); }
uint64_t mws_file::length() { return io.impl->length(); }
uint64_t mws_file::creation_time() const { return io.impl->creation_time(); }
uint64_t mws_file::last_write_time() const { return io.impl->last_write_time(); }
const mws_path& mws_file::path() const { return io.impl->ppath; }
std::string mws_file::string_path() const { return io.impl->ppath.string(); }
std::string mws_file::filename() const { return io.impl->ppath.filename(); }
std::string mws_file::stem() const { return io.impl->ppath.stem(); }
std::string mws_file::extension() const { return io.impl->ppath.extension(); }
mws_path mws_file::directory() const { return io.impl->ppath.directory(); }
bool mws_file::is_internal() const { return io.impl->is_internal(); }
FILE* mws_file::get_file_impl() const { return io.impl->get_file_impl(); }
mws_file::io_op::io_op() {}
bool mws_file::io_op::open() { return open("rb"); }

bool mws_file::io_op::open(std::string i_open_mode)
{
   if (impl->is_open())
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

void mws_file::io_op::close() { impl->close(); }
void mws_file::io_op::flush() { impl->flush(); }
bool mws_file::io_op::reached_eof() const { return impl->reached_eof(); }
uint64_t mws_file::io_op::io_position() const { return impl->io_position(); }
void mws_file::io_op::set_io_position(uint64_t i_pos) { impl->set_io_position(i_pos); }
int mws_file::io_op::read(std::vector<std::byte>& i_buffer) { return impl->read(i_buffer); }
int mws_file::io_op::write(const std::vector<std::byte>& i_buffer) { return impl->write(i_buffer); }
int mws_file::io_op::read(std::byte* i_buffer, uint32_t i_size) { return impl->read(i_buffer, i_size); }
int mws_file::io_op::write(const std::byte* i_buffer, uint32_t i_size) { return impl->write(i_buffer, i_size); }


// mws_file_wrapper
mws_file_wrapper::mws_file_wrapper() {}
mws_file_wrapper::mws_file_wrapper(std::shared_ptr<mws_file> i_file) : file_v(i_file) {}
bool mws_file_wrapper::is_open() const { return file_v->is_open(); }
bool mws_file_wrapper::is_writable() const { return file_v->is_writable(); }
uint64_t mws_file_wrapper::length() const { return file_v->length(); }
void mws_file_wrapper::close() { file_v->io.close(); }
void mws_file_wrapper::set_io_position(uint64_t i_position) { file_v->io.set_io_position(i_position); }

int mws_file_wrapper::read_bytes(std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return file_v->io.read(i_seqv + i_offset, i_elem_count);
}

int mws_file_wrapper::write_bytes(const std::byte* i_seqv, uint32_t i_elem_count, uint32_t i_offset)
{
   return file_v->io.write(i_seqv + i_offset, i_elem_count);
}


// mws_file_data_seqv
mws_file_data_seqv::mws_file_data_seqv(const mws_file_wrapper& i_file, bool i_is_writable)
{
   assert(i_file.is_open());
   assert((i_is_writable) ? i_file.is_writable() : true);
   file_v = i_file;
   is_writable = i_is_writable;
}

mws_file_data_seqv::mws_file_data_seqv() {}
void mws_file_data_seqv::set_file_wrapper(const mws_file_wrapper& i_file) { file_v = i_file; }


// mws_fsv
mws_fsv::mws_fsv() {}
mws_fsv::mws_fsv(const mws_file_wrapper& i_file, bool i_is_writable) : mws_file_data_seqv(i_file, i_is_writable) {}


// mws_rw_file_seqv
std::shared_ptr<mws_rw_file_seqv> mws_rw_file_seqv::nwi(const mws_file_wrapper& i_file, bool i_is_writable)
{
   std::shared_ptr<mws_rw_file_seqv> inst(new mws_rw_file_seqv(i_file, i_is_writable));
   inst->r.set_data_sequence(inst);
   if (i_is_writable) { inst->w.set_data_sequence(inst); }
   return inst;
}

mws_rw_file_seqv::mws_rw_file_seqv(const mws_file_wrapper& i_file, bool i_is_writable) : mws_file_data_seqv(i_file, i_is_writable) {}


const std::string& mws_app::res_idx_name()
{
   static std::string res_index = "res-idx.str";
   return res_index;
}

void mws_app::init()
{
   mws_mod_ctrl::inst()->init_app();
}

void mws_app::start()
{
   mws_mod_ctrl::inst()->start_app();
}

void mws_app::run()
{
   mws_mod_ctrl::inst()->update();
}

mws_key_types mws_app::apply_key_modifiers(mws_key_types i_key_id) const
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

bool mws_app::back_evt() const
{
   return mws_mod_ctrl::inst()->back_evt();
}

float mws_app::get_screen_scale() const { return 1.f; }
float mws_app::get_screen_brightness() const { return 1.f; }
void mws_app::set_screen_brightness(float i_brightness) const {}
void mws_app::flip_screen() const {}
float mws_app::get_avg_screen_dpcm() const { return get_avg_screen_dpi() / 2.54f; }

mws_file_map mws_app::list_internal_directory() const
{
   mws_file_map file_map;
   const bool is_internal = true;
   const mws_path& res_dir = mws_app_inst()->res_dir();
   mws_sp<mws_file_impl> res_idx_impl = mws_app_inst()->new_mws_file_impl(res_dir / mws_app::res_idx_name(), is_internal);
   mws_sp<mws_file> res_idx = mws_file::get_inst(res_idx_impl);

   file_map = mws_res_index::read_file_map(res_idx);

   return file_map;
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
   case mws_pfm_linux: return false;
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
namespace
{
   // std string arguments
   int mws_str_argc = 0;
   const char** mws_str_argv;
   std::string mws_str_app_path;
   std::vector<std::string> mws_str_arg_vector;

   // std wstring(unicode) arguments
   int mws_wstr_argc = 0;
   unicode_string mws_wstr_app_path;
   std::vector<unicode_string> mws_wstr_arg_vector;
}


int mws::args::get_str_arg_count()
{
   return mws_str_argc;
}

const std::string& mws::args::get_str_path()
{
   return mws_str_app_path;
}

const char** mws::args::get_str_arg_vect()
{
   return mws_str_argv;
}

const std::vector<std::string>& mws::args::get_str_arg_str_vect()
{
   return mws_str_arg_vector;
}

void mws::args::set_str_args(int i_argument_count, const char** i_argument_vector, bool i_app_path_included)
{
   mws_str_argc = i_argument_count;
   mws_str_argv = i_argument_vector;

   if (i_app_path_included)
   {
      mws_str_app_path.assign(i_argument_vector[0]);
   }

   for (int k = 0; k < mws_wstr_argc; k++)
   {
      mws_str_arg_vector.push_back(i_argument_vector[k]);
   }
}

int mws::args::get_unicode_arg_count()
{
   return mws_wstr_argc;
}

const unicode_string& mws::args::get_unicode_path()
{
   return mws_wstr_app_path;
}

const std::vector<unicode_string>& mws::args::get_unicode_arg_vect()
{
   return mws_wstr_arg_vector;
}

void mws::args::set_unicode_args(int i_argument_count, unicode_char** i_argument_vector, bool i_app_path_included)
{
   mws_wstr_argc = i_argument_count;

   if (i_app_path_included)
   {
      mws_wstr_app_path.assign(i_argument_vector[0]);
   }

   for (int k = 0; k < mws_wstr_argc; k++)
   {
      mws_wstr_arg_vector.push_back(i_argument_vector[k]);
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
uint32_t mws::screen::get_width() { return mws_mod_ctrl::get_screen_width(); }
uint32_t mws::screen::get_height() { return mws_mod_ctrl::get_screen_height(); }
float mws::screen::get_scale() { return mws_app_inst()->get_screen_scale(); }
float mws::screen::get_scaled_width() { return get_width() * get_scale(); }
float mws::screen::get_scaled_height() { return get_height() * get_scale(); }
std::pair<uint32_t, uint32_t> mws::screen::get_res_px() { return mws_app_inst()->get_screen_res_px(); }
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

std::string mws::time::get_duration_as_string(uint32_t i_duration)
{
   std::string duration;

   uint32_t millis = i_duration % 1000;
   uint32_t seconds = i_duration / 1000;
   uint32_t minutes = seconds / 60;
   uint32_t seconds_remainder = seconds % 60;

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

      if (log_file->is_open())
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
      std::string buff;
      text_buffer = i_text_buffer;
      text_buffer->clear();
      console_active = true;

      if (!log.empty())
      {
         // most recent entries are top most in the console
         for (int32_t k = (int32_t)log.size() - 1; k >= 0; k--)
         {
            buff += log[k];
         }

         text_buffer->push_front(buff);
      }
   }

private:
   void load()
   {
      std::lock_guard<std::mutex> lock(sync_mx);
      mws_path log_path = mws_app_inst()->tmp_dir() / mws_log_file_name;
      log_file = mws_file::get_inst(log_path);

      if (log_file && log_file->exists())
      {
         log_file->io.open("rb");

         auto res_rw = mws_rw_file_seqv::nwi(log_file, false);
         uint64_t file_length = log_file->length();

         while (res_rw->read_position() < file_length)
         {
            std::string line = res_rw->r.read_text();
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

      if (log_file && log_file->is_open())
      {
         auto res_rw = mws_rw_file_seqv::nwi(log_file, true);

         res_rw->w.write_text(i_msg);
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
         mws_path log_path = mws_app_inst()->tmp_dir() / mws_log_file_name;
         log_file = mws_file::get_inst(log_path);
      }

      if (log_file)
      {
         if (!log_file->is_open())
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
