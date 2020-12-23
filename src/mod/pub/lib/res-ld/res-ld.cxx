#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "res-ld.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"


raw_img_data::raw_img_data(int i_width, int i_height)
{
   width = i_width;
   height = i_height;
   size = width * height * 4;
   gl_color_format = 1;
   data = (uint8_t*)malloc(size);
   memset(data, 0, size);
}

raw_img_data::~raw_img_data()
{
   free(data);
   data = nullptr;
}


res_ld::res_ld()
{
}

mws_sp<std::vector<uint8_t>> res_ld::flip_buffer(int i_width, int i_height, uint8_t* i_buffer, flip_types i_flip)
{
   size_t buffer_size = i_width * i_height * 4;
   mws_sp<std::vector<uint8_t>> buffer = std::make_shared<std::vector<uint8_t>>(buffer_size);
   uint8_t* buf_data = buffer->data();
   size_t row_size = i_width * 4;
   const auto horizontal_flip = [&]()
   {
      for (int i = 0; i < i_height; i++)
      {
         for (int j = 0, l = i_width - 1; j < i_width; j++, l--)
         {
            uint8_t* src = i_buffer + i * row_size + j * 4;
            uint8_t* dst = buf_data + i * row_size + l * 4;
            *dst = *src;
         }
      }
   };
   const auto vertical_flip = [&]()
   {
      for (int i = 0, j = i_height - 1; i < i_height; i++, j--)
      {
         uint8_t* src = i_buffer + i * row_size;
         uint8_t* dst = buf_data + j * row_size;
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

namespace
{
   mws_sp<res_ld> res_loader_inst;
}

mws_sp<res_ld> res_ld::inst()
{
   if (!res_loader_inst)
   {
      res_loader_inst = mws_sp<res_ld>(new res_ld());
   }

   return res_loader_inst;
}


#if MOD_PNG

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

mws_sp<raw_img_data> res_ld::load_image(mws_sp<mws_file> i_file)
{
   mws_sp<raw_img_data> rd(new raw_img_data());
   std::vector<uint8_t> png_data = mws_mod_ctrl::inst()->app_storage().load_as_byte_vect(i_file);
   {
      uint32_t error = 0;
      uint8_t* img_data = nullptr;
      uint32_t width = 0, height = 0;
      LodePNGState state;
      int bpp = 0;

      lodepng_state_init(&state);
      error = lodepng_decode(&img_data, &width, &height, &state, png_data.data(), png_data.size());

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

   mws_println("loading image[ %s ], size[ %d ]", i_file->filename().c_str(), png_data.size());

   return rd;
}

mws_sp<raw_img_data> res_ld::load_image(std::string i_filename)
{
   std::string img_name = i_filename;

   if (!ends_with(img_name, ".png"))
   {
      img_name += ".png";
   }

   mws_sp<mws_file> file = mws_file::get_inst(img_name);

   return load_image(file);
}

bool res_ld::save_image(mws_sp<mws_file> i_file, int i_width, int i_height, uint8_t * i_buffer, flip_types i_flip)
{
   uint8_t* png = nullptr;
   size_t png_size = 0;
   mws_sp<std::vector<uint8_t>> flip_buff;

   if (i_flip != flip_types::e_no_flip)
   {
      flip_buff = flip_buffer(i_width, i_height, i_buffer, i_flip);
   }

   const uint8_t* buffer = (flip_buff) ? flip_buff->data() : i_buffer;
   uint32_t error = lodepng_encode32(&png, &png_size, buffer, i_width, i_height);

   if (!error)
   {
      lodepng_save_file(png, png_size, i_file->string_path().c_str());
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

mws_sp<raw_img_data> res_ld::load_image(mws_sp<mws_file> i_file)
{
   mws_print("img [%s] not loaded. MOD_PNG is disabled.\n", i_file->filename().c_str());
   return std::make_shared<raw_img_data>(32, 32);
}

mws_sp<raw_img_data> res_ld::load_image(std::string i_filename)
{
   mws_print("img [%s] not loaded. MOD_PNG is disabled.\n", i_filename.c_str());
   return std::make_shared<raw_img_data>(32, 32);
}

bool res_ld::save_image(mws_sp<mws_file> i_file, int i_width, int i_height, uint8_t* i_buffer, flip_types i_flip)
{
   mws_print("img [%s] not saved. MOD_PNG is disabled.\n", i_file->filename().c_str());
   return false;
}

raw_img_data::raw_img_data()
{
   data = nullptr;
}

#if defined MWS_PFM_IOS

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
