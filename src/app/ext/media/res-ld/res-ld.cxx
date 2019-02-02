#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "res-ld.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"


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

mws_sp<std::vector<uint8>> res_ld::flip_buffer(int i_width, int i_height, uint8* i_buffer, flip_types i_flip)
{
   size_t buffer_size = i_width * i_height * 4;
   mws_sp<std::vector<uint8>> buffer = std::make_shared<std::vector<uint8>>(buffer_size);
   uint8* buf_data = buffer->data();
   size_t row_size = i_width * 4;
   auto horizontal_flip = [&]()
   {
      for (int i = 0; i < i_height; i++)
      {
         for (int j = 0, l = i_width - 1; j < i_width; j++, l--)
         {
            uint8* src = i_buffer + i * row_size + j * 4;
            uint8* dst = buf_data + i * row_size + l * 4;
            *dst = *src;
         }
      }
   };
   auto vertical_flip = [&]()
   {
      for (int i = 0, j = i_height - 1; i < i_height; i++, j--)
      {
         uint8* src = i_buffer + i * row_size;
         uint8* dst = buf_data + j * row_size;
         memcpy(dst, src, row_size);
      }
   };

   switch (i_flip)
   {
   case flip_types::e_no_flip:
      memcpy(buf_data, i_buffer, buffer_size);
      break;

   case flip_types::e_horizontal_flip:
      horizontal_flip();
      break;

   case flip_types::e_vertical_flip:
      vertical_flip();
      break;

   case (flip_types::e_horizontal_flip | flip_types::e_vertical_flip):
      horizontal_flip();
      vertical_flip();
      break;
   }

   return buffer;
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

#include "min.hxx"
#include "png/lodepng.h"


static gfx_enum get_gl_color_format(const LodePNGInfo & i_info_png)
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

bool res_ld::save_image(mws_sp<pfm_file> i_file, int i_width, int i_height, uint8 * i_buffer, flip_types i_flip)
{
   uint8* png = nullptr;
   size_t png_size = 0;
   mws_sp<std::vector<uint8>> flip_buff;

   if (i_flip != flip_types::e_no_flip)
   {
      flip_buff = flip_buffer(i_width, i_height, i_buffer, i_flip);
   }

   const uint8* buffer = (flip_buff) ? flip_buff->data() : i_buffer;
   uint32 error = lodepng_encode32(&png, &png_size, buffer, i_width, i_height);

   if (!error)
   {
      lodepng_save_file(png, png_size, i_file->get_full_path().c_str());
   }

   //if there's an error, display it
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

#include "objc-cxx-bridge.hxx"

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
