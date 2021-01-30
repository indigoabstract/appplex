#pragma once

#include "pfm.hxx"
#pragma warning(disable : 4201)


class gfx_tex;


union rgba_32_fmt
{
   uint32_t abgr;

   struct
   {
      uint8_t r;
      uint8_t g;
      uint8_t b;
      uint8_t a;
   };
};


class raw_img_data
{
public:
   raw_img_data();
   raw_img_data(int i_width, int i_height);
   ~raw_img_data();

   int width;
   int height;
   int size;
   gfx_enum gl_color_format;
   uint8_t* data;
};


class res_ld
{
public:
   enum flip_types
   {
      e_no_flip = 0,
      e_horizontal_flip = 1,
      e_vertical_flip = 1 << 1,
      e_horz_vert_flip = (e_horizontal_flip | e_vertical_flip),
   };

   static mws_sp<res_ld> inst();

   mws_sp<gfx_tex> load_tex(std::string i_filename);
   mws_sp<raw_img_data> load_image(mws_sp<mws_file> i_file);
   mws_sp<raw_img_data> load_image(std::string i_filename);
   bool save_image(mws_sp<mws_file> i_file, int i_width, int i_height, uint8_t* i_buffer, flip_types i_flip = e_no_flip);

private:
   res_ld();
   static mws_sp<std::vector<uint8_t>> flip_buffer(int i_width, int i_height, uint8_t* i_buffer, flip_types i_flip);
};
