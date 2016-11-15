#include "stdafx.h"

#include "res-ld.hpp"
#include "min.hpp"
#include <libpng/png.h>


png_byte color_type = PNG_COLOR_TYPE_RGBA;
png_byte bit_depth = 8;

png_structp png_ptr;
png_infop info_ptr;
png_bytep * row_pointers;


void abort_(const char * s, ...)
{
	//va_list args;
	//va_start(args, s);
	//vfprintf(stderr, s, args);
	//fprintf(stderr, "\n");
	//va_end(args);
	abort();
}

void write_png_file(shared_ptr<pfm_file> file_name, int iwidth, int iheight, uint8* ibuffer, uint32 iflip)
{
	int width = iwidth;
	int height = iheight;
	file_name->io.open("wb");
	/* create file */
	if (!file_name->is_opened())
		abort_("[write_png_file] File %s could not be opened for writing", file_name->get_file_name().c_str());


	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		abort_("[write_png_file] png_create_write_struct failed");

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		abort_("[write_png_file] png_create_info_struct failed");

	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during init_io");
	FILE* fff = (FILE*)file_name->get_file_impl();

	png_init_io(png_ptr, (FILE*)file_name->get_file_impl());


	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing header");

	png_set_IHDR(png_ptr, info_ptr, width, height,
		bit_depth, color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);


	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during writing bytes");

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	int row_bytes = png_get_rowbytes(png_ptr,info_ptr);

	if ((iflip & res_ld::e_horizontal_flip) != 0)
	{

	}

	int y_start = 0;
	int y_end = height - 1;
	int y_add = 1;

	if ((iflip & res_ld::e_vertical_flip) != 0)
	{
		y_start = height - 1;
		y_end = 0;
		y_add = -1;
	}

	for (int y = 0; y<height; y++)
	{
		row_pointers[y] = (png_byte*) malloc(row_bytes);
	}

	int l = 0;
	for (int y = y_start, yc = 0; yc < height; y += y_add, yc++)
	{
		png_byte* row = row_pointers[y];

		for (int x=0; x<width; x++)
		{
			png_byte* ptr = &(row[x*4]);
			////printf("Pixel at position [ %d - %d ] has RGBA values: %d - %d - %d - %d\n", x, y, ptr[0], ptr[1], ptr[2], ptr[3]);

			///* set red value to 0 and green value to the blue one */
			//ptr[0] = 0;
			//ptr[1] = ptr[2];
			ptr[0] = ibuffer[l++];
			ptr[1] = ibuffer[l++];
			ptr[2] = ibuffer[l++];
			ptr[3] = ibuffer[l++];
			//int r= ptr[0];
			//int g= ptr[1];
			//int b= ptr[2];
			//int a= ptr[3];
			//int xy = 3;
		}
	}

	png_write_image(png_ptr, row_pointers);


	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		abort_("[write_png_file] Error during end of write");

	png_write_end(png_ptr, NULL);

	/* cleanup heap allocation */
	for (int y=0; y<height; y++)
		free(row_pointers[y]);
	free(row_pointers);

	file_name->io.close();
}


void process_file(void)
{
	if (png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGB)
		abort_("[process_file] input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA "
		"(lacks the alpha channel)");

	if (png_get_color_type(png_ptr, info_ptr) != PNG_COLOR_TYPE_RGBA)
		abort_("[process_file] color_type of input file must be PNG_COLOR_TYPE_RGBA (%d) (is %d)",
		PNG_COLOR_TYPE_RGBA, png_get_color_type(png_ptr, info_ptr));

}


shared_ptr<RawImageData> get_raw_image_data_from_png(const void* png_data, const int png_data_size);
void release_raw_image_data(const RawImageData* data);


RawImageData::RawImageData()
{
	data = 0;
}

RawImageData::~RawImageData()
{
	release_raw_image_data(this);
}


shared_ptr<res_ld> res_ld::res_loader_inst;

res_ld::res_ld()
{
}

shared_ptr<res_ld> res_ld::inst()
{
	if(!res_loader_inst)
	{
		res_loader_inst = shared_ptr<res_ld>(new res_ld());
	}

	return res_loader_inst;
}

shared_ptr<RawImageData> res_ld::load_image(shared_ptr<pfm_file> ifile)
{
	shared_ptr<std::vector<uint8> > img_data = pfm::filesystem::load_res_byte_vect(ifile);
	shared_ptr<RawImageData> rid = get_raw_image_data_from_png(begin_ptr(img_data), img_data->size());
	vprint("loading img file [%s], size [%d]\n", ifile->get_file_name().c_str(), img_data->size());

	return rid;
}

shared_ptr<RawImageData> res_ld::load_image(std::string ifile_name)
{
	std::string img_name = ifile_name;

	if(!ends_with(img_name, ".png"))
	{
		img_name += ".png";
	}

	shared_ptr<std::vector<uint8> > img_data = pfm::filesystem::load_res_byte_vect(img_name);
	shared_ptr<RawImageData> rid = get_raw_image_data_from_png(begin_ptr(img_data), img_data->size());
	vprint("loading img file [%s], size [%d]\n", ifile_name.c_str(), img_data->size());

	return rid;
}

bool res_ld::save_image(shared_ptr<pfm_file> ifile, int iwidth, int iheight, uint8* ibuffer, uint32 iflip)
{
	write_png_file(ifile, iwidth, iheight, ibuffer, iflip);

	return true;
}




struct DataHandle
{
	const png_byte* data;
	const png_size_t size;
};

struct ReadDataHandle
{
	const DataHandle data;
	png_size_t offset;
};

struct PngInfo
{
	const png_uint_32 width;
	const png_uint_32 height;
	const int color_type;
};

static void read_png_data_callback(png_structp png_ptr, png_byte* png_data, png_size_t read_length);
static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr);
static DataHandle read_entire_png_image(const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height);
static gfx_enum get_gl_color_format(const int png_color_format);

shared_ptr<RawImageData> get_raw_image_data_from_png(const void* png_data, const int png_data_size)
{
	assert(png_data != NULL && png_data_size > 8);
	assert(png_check_sig((png_const_bytep)png_data, 8));

	png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	assert(png_ptr != NULL);
	png_infop info_ptr = png_create_info_struct(png_ptr);
	assert(info_ptr != NULL);

	ReadDataHandle png_data_handle = {{(const png_byte*)png_data, (png_size_t)png_data_size}, 0};
	png_set_read_fn(png_ptr, &png_data_handle, read_png_data_callback);

	if (setjmp(png_jmpbuf(png_ptr))) {
		trx("Error reading PNG file!");
	}

	const PngInfo png_info = read_and_update_info(png_ptr, info_ptr);
	const DataHandle raw_image = read_entire_png_image(png_ptr, info_ptr, png_info.height);

	png_read_end(png_ptr, info_ptr);
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	shared_ptr<RawImageData> rd(new RawImageData());
	rd->width = png_info.width;
	rd->height = png_info.height;
	rd->size = raw_image.size;
	rd->gl_color_format = get_gl_color_format(png_info.color_type);
	rd->data = (uint8*)raw_image.data;

	return rd;
}

void release_raw_image_data(const RawImageData* data) {
	assert(data != NULL);
	free((void*)data->data);
}

static void read_png_data_callback(png_structp png_ptr, png_byte* raw_data, png_size_t read_length) {
	ReadDataHandle* handle = (ReadDataHandle*)png_get_io_ptr(png_ptr);
	const png_byte* png_src = handle->data.data + handle->offset;

	memcpy(raw_data, png_src, read_length);
	handle->offset += read_length;
}

static PngInfo read_and_update_info(const png_structp png_ptr, const png_infop info_ptr) {
	png_uint_32 width, height;
	int bit_depth, color_type;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, NULL, NULL, NULL);

	// Convert transparency to full alpha
	if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);

	// Convert grayscale, if needed.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	// Convert paletted images, if needed.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	// Add alpha channel, if there is none (rationale: GL_RGBA is faster than GL_RGB on many GPUs)
	if (color_type == PNG_COLOR_TYPE_PALETTE || color_type == PNG_COLOR_TYPE_RGB)
		png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);

	// Ensure 8-bit packing
	if (bit_depth < 8)
		png_set_packing(png_ptr);
	else if (bit_depth == 16)
		png_set_scale_16(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	// Read the new color type after updates have been made.
	color_type = png_get_color_type(png_ptr, info_ptr);

	PngInfo pngi = {width, height, color_type};
	return pngi;
}

static DataHandle read_entire_png_image(const png_structp png_ptr, const png_infop info_ptr, const png_uint_32 height) {
	const png_size_t row_size = png_get_rowbytes(png_ptr, info_ptr);
	const int data_length = row_size * height;
	assert(row_size > 0);

	png_byte* raw_image = (png_byte*)malloc(data_length);
	assert(raw_image != NULL);

	png_byte** row_ptrs = (png_byte**)malloc(height * sizeof(png_byte*));

	png_uint_32 i;
	for (i = 0; i < height; i++) {
		row_ptrs[i] = raw_image + i * row_size;
	}

	png_read_image(png_ptr, &row_ptrs[0]);

	DataHandle dh = {raw_image, (png_size_t)data_length};
	free(row_ptrs);
	return dh;
}

static gfx_enum get_gl_color_format(const int png_color_format) {
	assert(png_color_format == PNG_COLOR_TYPE_GRAY
		|| png_color_format == PNG_COLOR_TYPE_RGB_ALPHA
		|| png_color_format == PNG_COLOR_TYPE_GRAY_ALPHA);

	switch (png_color_format) {
	case PNG_COLOR_TYPE_GRAY:
		return 0;//GL_LUMINANCE;
	case PNG_COLOR_TYPE_RGB_ALPHA:
		return 1;//GL_RGBA;
	case PNG_COLOR_TYPE_GRAY_ALPHA:
		return 2;//GL_LUMINANCE_ALPHA;
	}

	return 0;
}
