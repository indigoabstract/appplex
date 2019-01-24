#include "stdafx.h"

#include "appplex-conf.hpp"
#include "res-ld.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"


raw_img_data::raw_img_data(int i_width, int i_height)
{
   width = i_width;
   height = i_height;
   size = width * height * 4;
   gl_color_format = 1;
   data = (uint8*)malloc(size);
   memset(data, 0, size);
}

raw_img_data::~raw_img_data()
{
   free(data);
   data = nullptr;
}


mws_sp<res_ld> res_ld::res_loader_inst;

res_ld::res_ld()
{
}

mws_sp<res_ld> res_ld::inst()
{
   if (!res_loader_inst)
   {
      res_loader_inst = mws_sp<res_ld>(new res_ld());
   }

   return res_loader_inst;
}


#if defined MOD_PNG

#include "min.hpp"
#include "public/lib/png/lodepng.h"


static gfx_enum get_gl_color_format(const LodePNGInfo& i_info_png)
{
   switch (i_info_png.color.colortype)
   {
   case LCT_GREY: return 0;//GL_LUMINANCE;
   case LCT_RGB: return -1;
   case LCT_PALETTE: return -1;
   case LCT_GREY_ALPHA: return 2;//GL_LUMINANCE_ALPHA;
   case LCT_RGBA: return 1;//GL_RGBA;
   }

   return -1;
}


raw_img_data::raw_img_data()
{
   data = nullptr;
}


mws_sp<gfx_tex> res_ld::load_tex(std::string i_filename)
{
   return gfx::i()->tex.nwi(i_filename);
}

mws_sp<raw_img_data> res_ld::load_image(mws_sp<pfm_file> i_file)
{
   mws_sp<raw_img_data> rd(new raw_img_data());
   mws_sp<std::vector<uint8> > png_data = pfm::filesystem::load_res_byte_vect(i_file);
   {
      uint32 error = 0;
      uint8* img_data = nullptr;
      uint32 width = 0, height = 0;
      LodePNGState state;
      int bpp = 0;

      lodepng_state_init(&state);
      error = lodepng_decode(&img_data, &width, &height, &state, png_data->data(), png_data->size());

      if (error)
      {
         mws_println("res_ld::load_image error[ %u, %s ]", error, lodepng_error_text(error));
      }

      switch (state.info_png.color.colortype)
      {
      case LCT_GREY: bpp = 1; break;
      case LCT_RGB: bpp = 3; break;
      case LCT_PALETTE: bpp = 1; break;
      case LCT_GREY_ALPHA: bpp = 2; break;
      case LCT_RGBA: bpp = 4; break;
      }

      rd->width = width;
      rd->height = height;
      rd->size = width * height * bpp;
      rd->gl_color_format = get_gl_color_format(state.info_png);
      rd->data = img_data;

      lodepng_state_cleanup(&state);
   }

   mws_println("loading image[ %s ], size[ %d ]", i_file->get_file_name().c_str(), png_data->size());

   return rd;
}

mws_sp<raw_img_data> res_ld::load_image(std::string i_filename)
{
   std::string img_name = i_filename;

   if (!ends_with(img_name, ".png"))
   {
      img_name += ".png";
   }

   mws_sp<pfm_file> file = pfm_file::get_inst(img_name);

   return load_image(file);
}

bool res_ld::save_image(mws_sp<pfm_file> i_file, int i_width, int i_height, uint8* i_buffer, uint32 i_flip)
{
   uint8* png = nullptr;
   size_t png_size = 0;

   unsigned error = lodepng_encode32(&png, &png_size, i_buffer, i_width, i_height);

   if (!error)
   {
      lodepng_save_file(png, png_size, i_file->get_full_path().c_str());
   }

   /*if there's an error, display it*/
   if (error)
   {
      mws_println("res_ld::save_image error[ %u, %s ]", error, lodepng_error_text(error));
   }

   free(png);

   return true;
}

#else

raw_img_data::raw_img_data()
{
   data = nullptr;
}

#if defined PLATFORM_IOS

#include "objc-cxx-bridge.hpp"

mws_sp<gfx_tex> res_ld::load_tex(std::string i_filename)
{
   return cxx_2_objc_load_tex_by_name(i_filename, gfx::i());
}

#else

mws_sp<gfx_tex> res_ld::load_tex(std::string i_filename)
{
   return nullptr;
}

#endif

#endif
