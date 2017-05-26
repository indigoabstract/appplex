#pragma once

#include "pfm.hpp"

#pragma comment (lib, "libpng17.lib")
#pragma comment (lib, "zlib.lib")


class RawImageData
{
public:
	RawImageData();
	~RawImageData();

	int width;
	int height;
	int size;
	gfx_enum gl_color_format;
	uint8* data;
};


class res_ld
{
public:
	enum flip_types
	{
		e_no_flip = 0,
		e_horizontal_flip = 1,
		e_vertical_flip = 1 << 1,
	};

	static shared_ptr<res_ld> inst();

	shared_ptr<RawImageData> load_image(shared_ptr<pfm_file> ifile);
	shared_ptr<RawImageData> load_image(std::string ifile_name);
	bool save_image(shared_ptr<pfm_file> ifile, int iwidth, int iheight, uint8* ibuffer, uint32 iflip = e_no_flip);

private:
	res_ld();

	static shared_ptr<res_ld> res_loader_inst;
};
