#include "stdafx.hxx"

#include "mod-test-freeimage.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "ovg-obj.hxx"
#include "cmd/recursive-dir.hxx"
#include "gfx.hxx"
#include <freeimage/freeimage.h>
#include <openvg.h>
#include <filesystem.hpp>

#pragma comment (lib, "freeimage.lib")

namespace bfs = ::boost::filesystem;
using std::string;
using std::vector;


mod_test_freeimage::mod_test_freeimage() : mws_mod(mws_stringify(MOD_TEST_FREEIMAGE)) {}

mws_sp<mod_test_freeimage> mod_test_freeimage::nwi()
{
	return mws_sp<mod_test_freeimage>(new mod_test_freeimage());
}

void mod_test_freeimage::init()
{
}

void mod_test_freeimage::save_image(string i_filename, mws_sp<vector<uint32> > ibgra)
{
	int width = pfm::screen::get_width();
	int height = pfm::screen::get_height();
	int pitch = 4 * width;
	unsigned bpp = 32;
	unsigned red_mask = FI_RGBA_RED_MASK;
	unsigned green_mask = FI_RGBA_GREEN_MASK;
	unsigned blue_mask = FI_RGBA_BLUE_MASK;
	BOOL topdown = false;
	BYTE* bits = (uint8*)begin_ptr(ibgra);
	bool result = false;

	// create image
	FIBITMAP* fbmp = FreeImage_ConvertFromRawBits(bits, width, height, pitch, bpp, red_mask, green_mask, blue_mask, topdown);
	// bgr -> rgb conversion
	FIBITMAP* red_channel = FreeImage_GetChannel(fbmp, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);
	FIBITMAP* blue_channel = FreeImage_GetChannel(fbmp, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
	result = FreeImage_SetChannel(fbmp, red_channel, FREE_IMAGE_COLOR_CHANNEL::FICC_BLUE);
	result = FreeImage_SetChannel(fbmp, blue_channel, FREE_IMAGE_COLOR_CHANNEL::FICC_RED);

	// save image
	result = FreeImage_Save(FIF_PNG, fbmp, i_filename.c_str());

	// clean-up
	FreeImage_Unload(fbmp);
	FreeImage_Unload(red_channel);
	FreeImage_Unload(blue_channel);

	if(result)
	{
		trx("screenshot saved as %1%") % i_filename;
	}
	else
	{
		trx("error saving screenshot as %1%") % i_filename;
	}
}


class memio : public FreeImageIO
{
public :
	memio(BYTE *data) : _start(data), _cp(data)
	{
		read_proc  = _ReadProc;
		write_proc = _WriteProc;
		tell_proc  = _TellProc;
		seek_proc  = _SeekProc;
	}

	void reset()
	{
		_cp = _start;
	}

	static unsigned int DLL_CALLCONV _ReadProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
	{
		memio *memIO = (memio*)handle;
		BYTE *tmp = (BYTE *)buffer;

		for (unsigned c = 0; c < count; c++)
		{
			memcpy(tmp, memIO->_cp, size);
			memIO->_cp = memIO->_cp + size;
			tmp += size;
		}

		return count;
	}

	static unsigned DLL_CALLCONV _WriteProc(void *buffer, unsigned size, unsigned count, fi_handle handle)
	{
		mws_signal_error();
		return size;
	}

	static int DLL_CALLCONV _SeekProc(fi_handle handle, long offset, int origin)
	{
		memio *memIO = (memio*)handle;

		if (origin == SEEK_SET)
		{
			memIO->_cp = memIO->_start + offset;
		}
		else
		{
			memIO->_cp = memIO->_cp + offset;
		}

		return 0;
	}

	static long DLL_CALLCONV _TellProc(fi_handle handle)
	{
		memio *memIO = (memio*)handle;

		return memIO->_cp - memIO->_start;
	}

private:
	BYTE * const _start;
	BYTE *_cp;
};


mws_sp<std::vector<uint8> > loadByteVect(bfs::path& p)
{
	mws_sp<vector<uint8> > res;

	if(exists(p))
	{
		mws_sp<pfm_file> fs = pfm_file::get_inst(p.string());
		fs->io.open();
		int size = file_size(p);

		res = mws_sp<vector<uint8> >(new vector<uint8>(size));
		fs->io.read(begin_ptr(res), size);
	}

	return res;
}

mws_sp<gfx_tex> newImage(const char *filename, mws_sp<vector<uint8> > dt)
{
	mws_sp<gfx_tex> img;
	FIBITMAP* fbmp = 0;

	if(!dt)
	{
		return img;
	}

	BYTE* d = begin_ptr(dt);
	int dSize = dt->size();
	memio memIO(d);

	//check the file signature and deduce its format
	FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromHandle(&memIO, (fi_handle)&memIO, dSize);

	if(fif == FIF_UNKNOWN)
		//if unknown, try_ to guess the file format from the file extension
	{
		fif = FreeImage_GetFIFFromFilename(filename);
	}

	if(fif == FIF_UNKNOWN)
		//if still unkown, return failure
	{
		return img;
	}

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
	{
		fbmp = FreeImage_LoadFromHandle(fif, &memIO, (fi_handle)&memIO);
	}

	if(!fbmp)
		//if the image failed to load, return failure
	{
		return img;
	}

	if(FreeImage_FlipVertical(fbmp))
	{
		VGubyte *data = 0;
		unsigned int width = FreeImage_GetWidth(fbmp);
		unsigned int height = FreeImage_GetHeight(fbmp);
		//vg::image_format::e_image_format rgbaFormat = vg::image_format::sARGB_8888;

		if(FreeImage_GetBPP(fbmp) != 32)
		{
			FIBITMAP* tbmp = FreeImage_ConvertTo32Bits(fbmp);

			FreeImage_Unload(fbmp);
			fbmp = tbmp;
		}

		data = FreeImage_GetBits(fbmp);
		//img = vg_image::nwi(data, width, height, rgbaFormat);
	}

	FreeImage_Unload(fbmp);

	return img;
}


namespace mod_freeimage_page1
{
	class main_page : public mws_page
	{
	public:
		main_page(mws_sp<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();

			string s = "openvg/test_img_guitar.png";
			imgxxx = gfx::i()->tex.nwi(s);
		}

		virtual void on_destroy()
		{
		}

		virtual void receive(mws_sp<mws_dp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(mws_sp<mws_camera> g)
		{
			mws_page::update_view(g);

			const string& text = get_mod()->get_name();

			g->drawImage(imgxxx, 0, 0);
			g->drawText(text, 10, 10);
		}

		mws_sp<gfx_tex> imgxxx;
	};
}


namespace mod_freeimage_page2
{
	class main_page : public mws_page
	{
	public:
		main_page(mws_sp<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();

			bfs::path p("C:\\Users\\indigoabstract\\Desktop\\tmedia2");
			dirtree = directory_tree::new_directory_tree(p);
			fidx = 0;

			load_img();
		}

		virtual void on_destroy()
		{
		}

		virtual void receive(mws_sp<mws_dp> idp)
		{
			if(idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
			{
			}
			else if(idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				mws_sp<key_evt> ke = key_evt::as_key_evt(idp);

				if(ke->get_type() != key_evt::KE_RELEASED)
				{
					bool do_action = true;

					switch(ke->get_key())
					{
					case KEY_LEFT:
						if(fidx > 0)
						{
							fidx--;
							load_img();
						}
						break;

					case KEY_RIGHT:
						if(fidx < dirtree->get_root_node()->files.size() - 1)
						{
							fidx++;
							load_img();
						}
						break;

					default:
						do_action = false;
					}

					if(do_action)
					{
						ke->process();
					}
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(mws_sp<mws_camera> g)
		{
			mws_page::update_view(g);

			const string& text = get_mod()->get_name();

			g->drawImage(imgxxx, 0, 0);
			g->drawText(text, 10, 10);
		}

		void load_img()
		{
			mws_sp<dir_node> rootNode = dirtree->get_root_node();
			bfs::path p2 = rootNode->files[fidx]->abs_file_path;
			mws_sp<vector<uint8> > dt = loadByteVect(p2);

			imgxxx = newImage(p2.string().c_str(), dt);
		}

		mws_sp<gfx_tex> imgxxx;
		mws_sp<directory_tree> dirtree;
		int fidx;
	};
}


void mod_test_freeimage::init_mws()
{
   mws_root->new_page<mod_freeimage_page1::mainpage>();
   mws_root->new_page<mod_freeimage_page2::mainpage>();
}

void mod_test_freeimage::load()
{
}
