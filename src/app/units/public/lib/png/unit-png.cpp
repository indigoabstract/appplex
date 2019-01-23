#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_PNG

#include "unit-png.hpp"
#include "lodepng.h"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"


unit_png::unit_png() : unit(mws_stringify(UNIT_PNG)) {}

mws_sp<unit_png> unit_png::nwi()
{
   return mws_sp<unit_png>(new unit_png());
}

void unit_png::init()
{
}


namespace unit_png_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         std::vector<uint8> png_data = decode("pentagon.png");
         encode("pentagon-out.png", png_data, img->get_width(), img->get_height());
      }

      virtual void receive(mws_sp<iadp> i_dp) override
      {
         mws_page::receive(i_dp);
      }

      virtual void update_state() override
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         mws_page::update_view(i_g);

         const std::string& text = get_unit()->get_name();

         i_g->drawImage(img, 50, 50);
         i_g->drawText(text, 10, 20);
      }

      std::vector<uint8> decode(const std::string& i_img_name)
      {
         lodepng::State state; //optionally customize this one
         uint32 error = 0;
         uint32 width = 0, height = 0;
         mws_sp<std::vector<uint8> > img_data = pfm::filesystem::load_res_byte_vect(i_img_name);
         std::vector<uint8> png;
         error = lodepng::decode(png, width, height, state, *img_data);

         if (error)
         {
            mws_println("png decode error %u: %s\n", error, lodepng_error_text(error));
         }

         img = gfx::i()->tex.nwi("penta", width, height);
         img->update(0, (char*)png.data());

         return png;
      }

      void encode(const std::string& i_img_name, const std::vector<uint8>& i_img_data, uint32 i_width, uint32 i_height)
      {
         lodepng::State state; //optionally customize this one
         uint32 error = 0;
         std::string out_file = pfm::filesystem::get_writable_path(i_img_name);
         std::vector<uint8> png;

         error = lodepng::encode(png, i_img_data, i_width, i_height, state);

         if (error)
         {
            mws_println("png encode error %u: %s\n", error, lodepng_error_text(error));
         }

         if (!error)
         {
            lodepng::save_file(png, out_file);
         }

         if (error)
         {
            mws_println("png encode error %u: %s\n", error, lodepng_error_text(error));
         }
      }

      mws_sp<gfx_tex> img;
   };
}


void unit_png::init_mws()
{
   mws_root->new_page<unit_png_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}

void unit_png::load()
{
}

#endif
