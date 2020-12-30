#include "stdafx.hxx"

#include "mod-png.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "lodepng.h"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_png::mod_png() : mws_mod(mws_stringify(MOD_PNG)) {}

mws_sp<mod_png> mod_png::nwi()
{
   return mws_sp<mod_png>(new mod_png());
}

void mod_png::init()
{
}


namespace mod_png_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         std::vector<uint8_t> png_data = decode("pentagon.png");
         encode("pentagon-out.png", png_data, img->get_width(), img->get_height());
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
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

         const std::string& text = get_mod()->name();

         i_g->drawImage(img, 50, 50);
         i_g->drawText(text, 10, 20);
      }

      std::vector<uint8_t> decode(const std::string& i_img_name)
      {
         lodepng::State state; //optionally customize this one
         uint32_t error = 0;
         uint32_t width = 0, height = 0;
         std::vector<std::byte> img_data = get_mod()->storage.load_as_byte_vect(i_img_name);
         std::vector<uint8_t> png;
         error = lodepng::decode(png, width, height, state, reinterpret_cast<uint8_t*>(img_data.data()), img_data.size());

         if (error)
         {
            mws_println("png decode error %u: %s\n", error, lodepng_error_text(error));
         }

         img = gfx::i()->tex.nwi("penta", width, height);
         img->update(0, (char*)png.data());

         return png;
      }

      void encode(const std::string& i_img_name, const std::vector<uint8_t>& i_img_data, uint32_t i_width, uint32_t i_height)
      {
         lodepng::State state; //optionally customize this one
         uint32_t error = 0;
         mws_path out_file = get_mod()->storage.prv_dir() / i_img_name;
         std::vector<uint8_t> png;

         error = lodepng::encode(png, i_img_data, i_width, i_height, state);

         if (error)
         {
            mws_println("png encode error %u: %s\n", error, lodepng_error_text(error));
         }

         if (!error)
         {
            lodepng::save_file(png, out_file.string());
         }

         if (error)
         {
            mws_println("png encode error %u: %s\n", error, lodepng_error_text(error));
         }
      }

      mws_sp<gfx_tex> img;
   };
}


void mod_png::init_mws()
{
   mws_root->new_page<mod_png_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}

void mod_png::load()
{
}
