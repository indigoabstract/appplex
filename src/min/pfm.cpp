#include "stdafx.h"

#include "pfm.hpp"
#include "pfmgl.h"
#include "unit.hpp"
#include "unit-ctrl.hpp"
#include "min.hpp"
#include <glm/glm.hpp>

#if defined MOD_BOOST
#include <boost/date_time.hpp>
//#include <boost/filesystem.hpp>

//using namespace boost::filesystem;
boost::posix_time::ptime time_start;
#endif

#include <stdio.h>
#include <cstdlib>
#include <cstring>

using std::string;
using std::wstring;
using std::vector;



// platform specific code
#if defined PLATFORM_ANDROID


const std::string dir_separator = "/";

platform_id pfm::get_platform_id()
{
	return platform_android;
}

gfx_type_id pfm::get_gfx_type_id()
{
	return gfx_type_opengl_es;
}

#include "main.hpp"
#include <android/log.h>

// trace
#define os_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)
#define wos_trace(arg)		__android_log_write(ANDROID_LOG_INFO, "appplex", arg)
#define pfm_app_inst		android_main::get_instance()

#if !defined MOD_BOOST

uint32 pfm::time::get_time_millis()
{
	struct timespec ts;

	if(clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
	{
		pfm_app_inst->write_text_nl("error");
	}

	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#endif


#elif defined PLATFORM_IOS

const std::string dir_separator = "/";

#elif defined PLATFORM_EMSCRIPTEN

const std::string dir_separator = "/";

#elif defined PLATFORM_WINDOWS_PC


const std::string dir_separator = "\\";

#ifdef USE_VLD
#include <vld.h>
#endif
#include <new>

namespace std { const nothrow_t nothrow = nothrow_t(); }

platform_id pfm::get_platform_id()
{
	return platform_windows_pc;
}

gfx_type_id pfm::get_gfx_type_id()
{
	return gfx_type_opengl;
}

#include "../../pfm/msvc/src/main.hpp"

#define pfm_app_inst		msvc_main::get_instance()

#if !defined MOD_BOOST

uint32 pfm::time::get_time_millis()
{
	return GetTickCount();
}

#endif


#endif



int arg_count = 0;
unicodestring app_path;
vector<unicodestring> arg_vector;
//boost::posix_time::ptime time_start;


glm::vec2 pfm_touch_event::touch_pos(uint32 touch_id) const
{
	const touch_point* p = this->find_point(touch_id);
	ia_assert(p);

	return glm::vec2(p->x, p->y);
}


namespace pfm_impl
{
	umf_list res_files_map;

	const std::string& get_res_path()
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
			static std::string res_path = "../res";
			return res_path;
		}

		case platform_windows_pc:
		{
			static std::string res_path = "../../../res";
			return res_path;
		}
		}

		throw ia_exception("undefined platform");
	}

	shared_ptr<pfm_file> get_res_file(const std::string& ifilename)
	{
		shared_ptr<pfm_file> file;

		if (!res_files_map)
		{
			pfm::filesystem::load_res_file_map();
		}

		auto it = res_files_map->find(ifilename);

		if (it != res_files_map->end())
		{
			file = it->second;
		}

		return file;
	}

	void put_res_file(const std::string& ifilename, shared_ptr<pfm_file> ifile)
	{
		if (!res_files_map)
		{
			pfm::filesystem::load_res_file_map();
		}

		auto it = res_files_map->find(ifilename);

		if (it != res_files_map->end())
		{
			std::string msg = "duplicate filename: " + it->first;

			throw ia_exception(msg);
		}

		(*res_files_map)[ifilename] = ifile;
	}

	pfm_file_impl::pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
	{
		ppath.filename = ifilename;
		ppath.aux_root_dir = iroot_dir;
		ppath.make_standard_path();
		file = 0;
		file_pos = 0;
		file_is_open = false;
		file_is_writable = false;
	}

	pfm_file_impl::~pfm_file_impl()
	{
	}

	void* pfm_file_impl::get_file_impl()const
	{
		return file;
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
		file = open_impl(iopen_mode);
		file_is_open = (file != 0);

		return file_is_open;
	}

	void pfm_file_impl::close()
	{
		if (file_is_open)
		{
			close_impl();
		}

		file = 0;
		file_pos = 0;
		file_is_open = false;
	}

	void pfm_file_impl::seek(uint64 ipos)
	{
		check_state();

		seek_impl(ipos, SEEK_SET);
		file_pos = ipos;
	}

	int pfm_file_impl::read(shared_ptr<std::vector<uint8> > ibuffer)
	{
		check_state();

		ibuffer->resize(length());

		return read(begin_ptr(ibuffer), ibuffer->size());
	}

	int pfm_file_impl::write(const shared_ptr<std::vector<uint8> > ibuffer)
	{
		check_state();

		return write(begin_ptr(ibuffer), ibuffer->size());
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

			throw ia_exception(msg);
		}
	}

	void* pfm_file_impl::open_impl(std::string iopen_mode)
	{
		std::string path = ppath.get_full_path();

		return fopen(path.c_str(), iopen_mode.c_str());
	}

	void pfm_file_impl::close_impl()
	{
		fclose((FILE*)file);
	}

	void pfm_file_impl::seek_impl(uint64 ipos, int iseek_pos)
	{
		fseek((FILE*)file, ipos, iseek_pos);
	}

	uint64 pfm_file_impl::tell_impl()
	{
		return ftell((FILE*)file);
	}

	int pfm_file_impl::read_impl(uint8* ibuffer, int isize)
	{
		return fread(ibuffer, 1, isize, (FILE*)file);
	}

	int pfm_file_impl::write_impl(const uint8* ibuffer, int isize)
	{
		return fwrite(ibuffer, 1, isize, (FILE*)file);
	}
}


pfm_file::pfm_file()
{
}

pfm_file::~pfm_file()
{
	io.close();
}

shared_ptr<pfm_file> pfm_file::get_inst(const std::string& ifilename, const std::string& iroot_dir)
{
	shared_ptr<pfm_file> inst;
	auto ppath = pfm_path::get_inst(ifilename, iroot_dir);

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
		inst = shared_ptr<pfm_file>(new pfm_file());
		inst->io.impl = pfm_app_inst->new_pfm_file_impl(ppath->get_file_name(), ppath->get_root_directory());
	}

	return inst;
}

shared_ptr<pfm_file> pfm_file::get_inst(shared_ptr<pfm_impl::pfm_file_impl> iimpl)
{
	shared_ptr<pfm_file> inst;

	if (pfm_impl::res_files_map)
		// res map initialized
	{
		inst = pfm_impl::get_res_file(iimpl->ppath.get_file_name());
	}

	if (!inst)
	{
		inst = shared_ptr<pfm_file>(new pfm_file());
		inst->io.impl = iimpl;
	}

	return inst;
}

bool pfm_file::remove()
{
	throw ia_exception("not implemented");

	return false;
}

bool pfm_file::exists()
{
	return io.impl->exists();
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

void* pfm_file::get_file_impl()const
{
	return io.impl->get_file_impl();
}

pfm_file::io_op::io_op()
{
}

bool pfm_file::io_op::open()
{
	bool file_opened = open("r+b");

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
		vprint("warning - file [%s] not found/opened\n", impl->ppath.get_file_name().c_str());
	}

	return file_opened;
}

void pfm_file::io_op::close()
{
	impl->close();
}

void pfm_file::io_op::seek(uint64 ipos)
{
	impl->seek(ipos);
}

int pfm_file::io_op::read(shared_ptr<std::vector<uint8> > ibuffer)
{
	return impl->read(ibuffer);
}

int pfm_file::io_op::write(const shared_ptr<std::vector<uint8> > ibuffer)
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


shared_ptr<pfm_path> pfm_path::get_inst(const std::string& ifile_path, const std::string& iaux_root_dir)
{
	shared_ptr<pfm_path> inst(new pfm_path());

	inst->filename = ifile_path;
	inst->aux_root_dir = iaux_root_dir;
	inst->make_standard_path();

	return inst;
}

std::string pfm_path::get_full_path()const
{
	if (aux_root_dir.empty())
	{
		return filename;
	}

	return aux_root_dir + dir_separator + filename;
}

const std::string& pfm_path::get_file_name()const
{
	return filename;
}

std::string pfm_path::get_file_stem()const
{
	std::string filename = get_file_name();
	size_t idx = filename.find_last_of('.');

	if (idx != filename.npos)
	{
		return filename.substr(0, idx);
	}

	return filename;
}

std::string pfm_path::get_file_extension()const
{
	std::string filename = get_file_name();
	size_t idx = filename.find_last_of('.');

	if (idx != filename.npos)
	{
		return filename.substr(idx + 1, filename.npos);
	}

	return "";
}

const std::string& pfm_path::get_root_directory()const
{
	return aux_root_dir;
}

shared_ptr<std::vector<shared_ptr<pfm_file> > > pfm_path::list_directory(bool recursive)const
{
	auto file_list = std::make_shared<std::vector<shared_ptr<pfm_file> > >();
	auto it = pfm_impl::res_files_map->begin();
	std::string base_dir = aux_root_dir;

	std::replace(base_dir.begin(), base_dir.end(), '\\', '/');

	if (!starts_with(base_dir, pfm_impl::get_res_path()))
	{
		if (!pfm_impl::get_res_path().empty())
		{
			base_dir = pfm_impl::get_res_path() + dir_separator + base_dir;
			std::replace(base_dir.begin(), base_dir.end(), '\\', '/');
		}
	}

	if (recursive)
	{
		for (; it != pfm_impl::res_files_map->end(); it++)
		{
			shared_ptr<pfm_file> file = it->second;
			std::string rdir = file->get_root_directory();

			if (starts_with(rdir, base_dir))
			{
				file_list->push_back(file);
			}
		}
	}
	else
	{
		for (; it != pfm_impl::res_files_map->end(); it++)
		{
			shared_ptr<pfm_file> file = it->second;

			if (file->get_root_directory() == base_dir)
			{
				file_list->push_back(file);
			}
		}
	}

	struct pred
	{
		bool operator()(const shared_ptr<pfm_file> a, const shared_ptr<pfm_file> b) const
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


pfm_data::pfm_data()
{
	screen_width = 1280;
	screen_height = 720;
	gfx_available = true;
	//console = shared_ptr<ia_console>(new ia_console());

#if defined MOD_BOOST
	time_start = boost::posix_time::ptime(boost::gregorian::day_clock::local_day());
#endif
}


pfm_data pfm::data;


bool pfm_main::back_evt()
{
	return unit_ctrl::inst()->back_evt();
}

//shared_ptr<ia_console> pfm::get_console()
//{
//	return data.console;
//}


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


int pfm::screen::get_width()
{
	return data.screen_width;
}

int pfm::screen::get_height()
{
	return data.screen_height;
}

int pfm::screen::get_target_fps()
{
	return 30;
}

int pfm::screen::get_screen_dpi()
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


//shared_array<uint8> pfm::storage::load_res_byte_array(string ifilename, int& isize)
//{
//	shared_array<uint8> res;
//	path p(ifilename.c_str());
//
//	if(p.is_relative())
//	{
//		p = get_path(ifilename.c_str());
//	}
//
//	if(exists(p))
//	{
//		shared_ptr<random_access_file> fs = get_random_access(p);
//		isize = file_size(p);
//
//		res = shared_array<uint8>(new uint8[isize]);
//		fs->read(res.get(), isize);
//	}
//
//	return res;
//}


std::string pfm::filesystem::get_writable_path(const std::string& iname)
{
	std::string p = pfm_app_inst->get_writable_path();

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

std::string pfm::filesystem::get_path(const std::string& iname)
{
	std::string p = pfm_impl::get_res_path();

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

void pfm::filesystem::load_res_file_map(shared_ptr<unit> iu)
{
	//if (!res_files_map)
	{
		pfm_impl::res_files_map = std::make_shared<umf_r>();
		pfm_app_inst->get_directory_listing(pfm_impl::get_res_path(), pfm_impl::res_files_map, false);
		pfm_app_inst->get_directory_listing(pfm::filesystem::get_path("common"), pfm_impl::res_files_map, true);

		if (iu)
		{
			pfm_app_inst->get_directory_listing(pfm::filesystem::get_path(iu->get_name()), pfm_impl::res_files_map, true);
		}
	}
}

shared_ptr<std::vector<uint8> > pfm::filesystem::load_res_byte_vect(shared_ptr<pfm_file> ifile)
{
	shared_ptr<vector<uint8> > res;

	if (ifile->io.open())
	{
		int size = ifile->length();

		res = shared_ptr<vector<uint8> >(new vector<uint8>(size));
		ifile->io.read(begin_ptr(res), size);
		ifile->io.close();
	}

	return res;
}

shared_ptr<std::vector<uint8> > pfm::filesystem::load_res_byte_vect(string ifilename)
{
	shared_ptr<pfm_file> fs = pfm_file::get_inst(ifilename);

	return load_res_byte_vect(fs);
}

shared_ptr<std::string> pfm::filesystem::load_res_as_string(shared_ptr<pfm_file> ifile)
{
	shared_ptr<std::string> text;

	if (ifile->io.open("rt"))
	{
		int size = ifile->length();
		auto res = std::make_shared<vector<uint8> >(size);
		const char* res_bytes = (const char*)begin_ptr(res);
		int text_size = ifile->io.read(begin_ptr(res), size);

		ifile->io.close();
		text = std::make_shared<std::string>(res_bytes, text_size);
	}

	return text;
}

shared_ptr<std::string> pfm::filesystem::load_res_as_string(std::string ifilename)
{
	shared_ptr<pfm_file> fs = pfm_file::get_inst(ifilename);

	return load_res_as_string(fs);
}

//shared_array<uint8> pfm::storage::load_unit_byte_array(shared_ptr<unit> iu, string ifilename, int& isize)
//{
//	if(!iu)
//	{
//		return shared_array<uint8>();
//	}
//
//	ifilename = trs("unit-data/%1%-%2%") % iu->get_name() % ifilename;
//
//	return load_res_byte_array(ifilename, isize);
//}

shared_ptr<std::vector<uint8> > pfm::filesystem::load_unit_byte_vect(shared_ptr<unit> iu, string ifilename)
{
	if (!iu)
	{
		return shared_ptr<std::vector<uint8> >();
	}

	//ifilename = trs("unit-data/%1%-%2%") % iu->get_name() % ifilename;

	return load_res_byte_vect(ifilename);
}

bool pfm::filesystem::store_unit_byte_array(shared_ptr<unit> iu, string ifilename, const uint8* ires, int isize)
{
	if (!iu)
	{
		return false;
	}

	//ifilename = trs("unit-data/%1%-%2%") % iu->get_name() % ifilename;

	//path p = get_path(ifilename.c_str());
	//shared_ptr<random_access_file> fs = get_random_access(p, true);

	//if(exists(p))
	//{
	//	fs->write(ires, isize);

	//	return true;
	//}

	return false;
}

bool pfm::filesystem::store_unit_byte_vect(shared_ptr<unit> iu, string ifilename, const vector<uint8>& ires)
{
	if (!iu)
	{
		return false;
	}

	//ifilename = trs("unit-data/%1%-%2%") % iu->get_name() % ifilename;

	//path p = get_path(ifilename.c_str());
	//shared_ptr<random_access_file> fs = get_random_access(p, true);

	//if(exists(p))
	//{
	//	fs->write(begin_ptr(ires), ires.size());

	//	return true;
	//}

	return false;
}

shared_ptr<pfm_file> pfm::filesystem::random_access(shared_ptr<unit> iu, std::string ifilename)
{
	if (!iu)
	{
		return shared_ptr<pfm_file>();
	}

	//ifilename = trs("unit-data/%1%-%2%") % iu->get_name() % ifilename;

	//return get_random_access(get_path(ifilename.c_str()), true);
	return shared_ptr<pfm_file>();
}

shared_ptr<pfm_main> pfm::get_pfm_main_inst()
{
	if (!pfm_impl::res_files_map)
	{
		pfm::filesystem::load_res_file_map();
	}

	return pfm_app_inst;
}


void trx(const char* format, fmt::ArgList args)
{
   std::string s = fmt::format(format, args);
   pfm::get_pfm_main_inst()->write_text_nl(s.c_str());
}

void wtrx(const wchar_t* format, fmt::ArgList args)
{
   std::wstring s = fmt::format(format, args);
   pfm::get_pfm_main_inst()->write_text_nl(s.c_str());
}

void trc(const char* format, fmt::ArgList args)
{
   std::string s = fmt::format(format, args);
   pfm::get_pfm_main_inst()->write_text(s.c_str());
}

void wtrc(const wchar_t* format, fmt::ArgList args)
{
   std::wstring s = fmt::format(format, args);
   pfm::get_pfm_main_inst()->write_text(s.c_str());
}

std::string trs(const char* format, fmt::ArgList args)
{
   return fmt::format(format, args);
}

std::wstring wtrs(const wchar_t* format, fmt::ArgList args)
{
   return fmt::format(format, args);
}


void* operator new(std::size_t isize, const std::nothrow_t& nothrow_value) throw()
{
	void* ptr = 0;

	if (isize > 0)
	{
		ptr = malloc(isize);

		ia_assert(ptr);

		if (ptr)
		{
			memset(ptr, 0, isize);
		}
	}

	return ptr;
}

void* operator new[](std::size_t isize, const std::nothrow_t& nothrow_value) throw()
{
	void* ptr = 0;

	if (isize > 0)
	{
		ptr = malloc(isize);

		ia_assert(ptr);

		if (ptr)
		{
			memset(ptr, 0, isize);
		}
	}

	return ptr;
}

void operator delete(void* iptr, const std::nothrow_t& nothrow_constant) throw()
{
	free(iptr);
}

void operator delete[](void* iptr, const std::nothrow_t& nothrow_constant) throw()
{
	free(iptr);
}
