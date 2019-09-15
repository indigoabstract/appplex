#include "stdafx.hxx"

#include "mod-kawase-bloom.hxx"
#include "kawase-bloom.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"
#include "mws/font-db.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include <array>


mod_kawase_bloom::mod_kawase_bloom() : mws_mod(mws_stringify(MOD_KAWASE_BLOOM)) {}

mws_sp<mod_kawase_bloom> mod_kawase_bloom::nwi()
{
   return mws_sp<mod_kawase_bloom>(new mod_kawase_bloom());
}


namespace mod_kawase_bloom_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         // font
         {
            std::string font_name = "consolas.ttf";
            mws_sp<pfm_file> font_file = pfm_file::get_inst(font_name);

            if (font_file->exists())
            {
               font_db::inst()->store_font_height(font_file->get_file_name(), mws_pt(12), mws_px(12), mws_pt(120), mws_px(121));
               mws_sp<mws_font> font = mws_font::nwi(font_file->get_file_name(), mws_cm(0.2f));
               font_db::inst()->set_global_font(font);
            }
            else
            {
               mws_println("font [ %s ] not found, using default", font_name.c_str());
            }
         }
         // ortho_cam
         {
            ortho_cam = mws_camera::nwi();
            ortho_cam->camera_id = "ortho_cam";
            ortho_cam->rendering_priority = 1;
            ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
            ortho_cam->near_clip_distance = -100;
            ortho_cam->far_clip_distance = 100;
            ortho_cam->clear_color = true;
            ortho_cam->clear_color_value = gfx_color::colors::blue;
            ortho_cam->clear_depth = true;
         }

         mws_sp<mws_font> fnt = ortho_cam->get_font();
         mws_sp<mws_font> fnt_big = mws_font::nwi(fnt, mws_cm(1.2f));
         std::string text = "glowing text";
         float text_width = fnt_big->get_text_width(text);

         // input pbb
         {
            input_ppb.init("tex-in", 512, 512);
            fnt_big->set_color(font_glow_color);

            // put the input texture in input_tex
            {
               mws_sp<gfx_tex> tex = input_ppb.get_tex();
               gfx::i()->rt.set_current_render_target(input_ppb.get_rt());
               ortho_cam->drawText(text, (tex->get_height() - text_width) / 2.f, (tex->get_height() - fnt_big->get_height()) / 2.f, fnt_big);
               ortho_cam->update_camera_state();
               gfx::i()->rt.set_current_render_target();
            }
         }

         mws_sp<gfx_tex> tex = input_ppb.get_tex();
         bloom = mws_kawase_bloom::nwi(tex);
         bloom->update();

         // output pbb
         {
            mws_sp<gfx_tex> bloom_tex = bloom->get_bloom_tex();
            output_ppb.init("tex-out", 512, 512);
            auto& rvxo = *output_ppb.get_quad();

            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo[MP_BLENDING] = MV_ADD;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = bloom_tex;
            rvxo.set_v_flip(true);

            // set color to white
            {
               fnt_big->set_color(gfx_color::colors::white);
               gfx::i()->rt.set_current_render_target(output_ppb.get_rt());
               fnt_big->set_color(gfx_color::colors::white);
               ortho_cam->drawText(text, (bloom_tex->get_height() - text_width) / 2.f, (bloom_tex->get_height() - fnt_big->get_height()) / 2.f, fnt_big);
               ortho_cam->update_camera_state();
               output_ppb.get_quad()->draw_out_of_sync(ortho_cam);
               gfx::i()->rt.set_current_render_target();
            }
         }
         // input_quad
         {
            input_quad = gfx_quad_2d::nwi();
            auto& rvxo = *input_quad;

            rvxo.camera_id_list = { "mws_cam" };
            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            rvxo.set_translation(50, 50);
            rvxo.set_scale((float)tex->get_width(), (float)tex->get_width());
            rvxo.set_v_flip(true);

            attach(input_quad);
         }
         // output bg
         {
            output_bg = gfx_quad_2d::nwi();
            auto& rvxo = *output_bg;

            rvxo.camera_id_list = { "mws_cam" };
            rvxo[MP_SHADER_NAME] = gfx::black_sh_id;
            rvxo.set_translation(100 + (float)tex->get_width(), 50);
            rvxo.set_scale((float)tex->get_width(), (float)tex->get_width());

            attach(output_bg);
         }
         // output quad
         {
            mws_sp<gfx_tex> bloom_tex = output_ppb.get_tex();
            output_quad = gfx_quad_2d::nwi();
            auto& rvxo = *output_quad;

            rvxo.camera_id_list = { "mws_cam" };
            rvxo[MP_BLENDING] = MV_ADD;
            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = bloom_tex;
            rvxo.set_translation(100 + (float)bloom_tex->get_width(), 50);
            rvxo.set_scale((float)bloom_tex->get_width(), (float)bloom_tex->get_width());
            rvxo.set_v_flip(true);

            attach(output_quad);
         }
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override { mws_page::receive(i_dp); }
      virtual void update_state() override {}
      virtual void update_view(mws_sp<mws_camera> i_g) override {}

      mws_sp<mws_kawase_bloom> bloom;
      static const inline gfx_color font_glow_color = gfx_color::from_argb(0xff007fff);
      mws_sp<mws_camera> ortho_cam;
      mws_gfx_ppb input_ppb;
      mws_gfx_ppb output_ppb;
      mws_sp<gfx_quad_2d> input_quad;
      mws_sp<gfx_quad_2d> output_bg;
      mws_sp<gfx_quad_2d> output_quad;
   };
}


void mod_kawase_bloom::init_mws()
{
   mws_root->new_page<mod_kawase_bloom_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}
