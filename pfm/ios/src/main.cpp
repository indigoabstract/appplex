#include "main.hpp"

#include "min.hpp"
#include "unit-ctrl.hpp"
#include "com/unit/input-ctrl.hpp"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <vector>

std::string writable_path;


class ios_file_impl : public pfm_impl::pfm_file_impl
{
public:
	ios_file_impl(const std::string& ifilename, const std::string& iroot_dir) : pfm_impl::pfm_file_impl(ifilename, iroot_dir)
	{
		is_external = false;
	}

	virtual ~ios_file_impl() {}

	virtual uint64 length()
	{
		uint64 size = 0;

		if (!file)
		{
			open("rb");

			if (file)
			{
				if(is_external)
				{
                    FILE* f = (FILE*)file;

                    fseek(f, 0, SEEK_END );
                    size = ftell(f);
				}
				else
				{
					//size = AAsset_getLength64((AAsset*)file);
				}
				
				close();
			}
		}
		else
		{
			if(is_external)
			{
                FILE* f = (FILE*)file;
                long crt_pos = ftell(f);

                fseek(f, 0, SEEK_END );
                size = ftell(f);
                fseek(f, crt_pos, SEEK_SET );
			}
			else
			{
				//size = AAsset_getLength64((AAsset*)file);
			}
		}

		return size;
	}

	virtual uint64 creation_time()const
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual uint64 last_write_time()const
	{
		std::string path = ppath.get_full_path();

		return 0;
	}

	virtual void* open_impl(std::string iopen_mode)
	{
		std::string path = ppath.get_full_path();
		is_external = false;

		if (iopen_mode[0] == 'w' && path[0] != '/')
		{
			return nullptr;
		}

		if(path[0] == '/')
		{
			// external path
			FILE* file = fopen(path.c_str(), iopen_mode.c_str());
			is_external = true;
			mws_print("open_impl: opening external file %s\n", path.c_str());
//			fseek( file, 0, SEEK_END );
//			uint64 len = ftell((FILE*)file);
//			fseek(file, 0, SEEK_SET );

			return file;
		}

		//AAsset* asset = AAssetManager_open(asset_manager, path.c_str(), 0);
		//mws_print("open_impl: opening asset file %s\n", path.c_str());

        return nullptr;//asset;
	}

	virtual void close_impl()
	{
		if(is_external)
		{
			fclose((FILE*)file);
		}
		else
		{
			//AAsset_close((AAsset*)file);
		}
		
		file = nullptr;
	}

	virtual void seek_impl(uint64 ipos, int iseek_pos)
	{
		if(is_external)
		{
			fseek((FILE*)file, ipos, iseek_pos);
		}
		else
		{
			//AAsset_seek64((AAsset*)file, ipos, iseek_pos);
		}
	}

	virtual uint64 tell_impl()
	{
		if(is_external)
		{
			return ftell((FILE*)file);
		}
		
		throw ia_exception("unsupported op");

		return 0;
	}

	virtual int read_impl(uint8* ibuffer, int isize)
	{
		if(is_external)
		{
			return fread(ibuffer, 1, isize, (FILE*)file);
		}
		
		return 0;//AAsset_read((AAsset*)file, ibuffer, isize);
	}

	virtual int write_impl(const uint8* ibuffer, int isize)
	{
		if(is_external)
		{
			return fwrite(ibuffer, 1, isize, (FILE*)file);
		}
		
		throw ia_exception("unsupported op");

		return 0;
	}

	bool is_external;
};


shared_ptr<ios_main> ios_main::instance;

ios_main::ios_main()
{
	plist = std::make_shared<umf_r>();
	is_started = false;
}

ios_main::~ios_main()
{
}

shared_ptr<ios_main> ios_main::get_instance()
{
	if (!instance)
	{
		instance = shared_ptr<ios_main>(new ios_main());
	}

	return instance;
}

shared_ptr<pfm_impl::pfm_file_impl> ios_main::new_pfm_file_impl(const std::string& ifilename, const std::string& iroot_dir)
{
	return std::make_shared<ios_file_impl>(ifilename, iroot_dir);
}

int ios_main::get_screen_dpi()const
{
    return 0;
}

void ios_main::write_text(const char* text)const
{
	printf("%s", text);
}

void ios_main::write_text_nl(const char* text)const
{
	printf("%s\n", text);
}

void ios_main::write_text(const wchar_t* text)const
{
}

void ios_main::write_text_nl(const wchar_t* text)const
{
}

void ios_main::write_text_v(const char* iformat, ...)const
{
   char dest[1024 * 16];
   va_list arg_ptr;

   va_start(arg_ptr, iformat);
    vsnprintf(dest, 1024 * 16 - 1, iformat, arg_ptr);
   va_end(arg_ptr);
   
   printf("%s", dest);
}

std::string ios_main::get_writable_path()const
{
	return writable_path;
}

void get_directory_listing_helper(umf_list iplist, shared_ptr<pfm_file> ifile)
{
	if (iplist->find(ifile->get_file_name()) != iplist->end())
	{
		mws_print("ios_main::get_directory_listing. duplicate filename: %s", ifile->get_full_path().c_str());
		throw ia_exception("duplicate filename: " + ifile->get_full_path());
	}

	(*iplist)[ifile->get_file_name()] = ifile;
}

umf_list ios_main::get_directory_listing(const std::string& idirectory, umf_list iplist, bool is_recursive)
{
	if (!idirectory.empty())
	{
		if (is_recursive)
		{
			for (auto& e : apk_file_list)
			{
				if (mws_str::starts_with(e->get_root_directory(), idirectory))
				{
					get_directory_listing_helper(iplist, e);
				}
			}
		}
		else
		{
			for (auto& e : apk_file_list)
			{
				if (idirectory == e->get_root_directory())
				{
					get_directory_listing_helper(iplist, e);
				}
			}
		}
	}

	return iplist;
}

bool ios_main::is_full_screen_mode()
{
	return true;
}

void ios_main::set_full_screen_mode(bool ienabled)
{
}

void ios_main::load_apk_file_list()
{
}

void ios_main::init()
{
	load_apk_file_list();

	unit_ctrl::inst()->pre_init_app();
	unit_ctrl::inst()->set_gfx_available(true);
	unit_ctrl::inst()->init_app();

	is_started = true;
}

void ios_main::start()
{
	unit_ctrl::inst()->start_app();
}

void ios_main::run()
{
	unit_ctrl::inst()->update();
}
