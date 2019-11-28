#include "stdafx.hxx"

#include "mod-font-metrix.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-pbo.hxx"
#include "gfx-rt.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"


mod_font_metrix::mod_font_metrix() : mws_mod(mws_stringify(MOD_font_METRIX)) {}

mws_sp<mod_font_metrix> mod_font_metrix::nwi()
{
   return mws_sp<mod_font_metrix>(new mod_font_metrix());
}


namespace mod_font_metrix_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         std::string text = " !#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";

         {
            mws_sp<mws_camera> g = get_mod()->mws_cam;
            smallest_font = g->get_font();
            smallest_font = mws_font::nwi(smallest_font, 12.f);
            largest_font = mws_font::nwi(smallest_font, 64.f);
            font_tex.init("font-tex", 256, 256);
            glm::vec2 tex_dim(font_tex.get_tex()->get_width(), font_tex.get_tex()->get_height());
            glm::vec2 letter_dim = smallest_font->get_text_dim("M");

            gfx::i()->rt.set_current_render_target(font_tex.get_rt());
            gfx_rt::clear_buffers();
            g->set_color(gfx_color::colors::cyan);
            glm::vec2 pos = (tex_dim - letter_dim) / 2.f;

            for (auto c : text)
            {
               g->drawText(std::string(1, c), pos.x, pos.y, smallest_font);
            }

            g->update_camera_state();
            gfx::i()->rt.set_current_render_target();

            font_quad = gfx_2d_sprite::nwi();
            gfx_2d_sprite& vxo = *font_quad;

            vxo.set_anchor(gfx_2d_sprite::e_top_left);
            vxo.name = "font-quad";
            vxo.visible = true;
            vxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            vxo["u_s2d_tex"] = font_tex.get_tex()->get_name();
            vxo[MP_DEPTH_TEST] = true;
            vxo[MP_DEPTH_WRITE] = true;
            vxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
            vxo[MP_BLENDING] = MV_NONE;
            vxo[MP_CULL_BACK] = false;
            vxo[MP_CULL_FRONT] = false;
            vxo.camera_id_list.clear();
            vxo.camera_id_list.push_back("mws_cam");
            vxo.set_dimensions(1, 1);
            vxo.set_scale(tex_dim.x, tex_dim.y);
            vxo.set_translation(20, 20);
            vxo.set_v_flip(true);
            attach(font_quad);
         }
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
      }

      virtual void on_resize() override
      {
      }

      mws_gfx_ppb font_tex;
      mws_sp<gfx_2d_sprite> font_quad;
      mws_sp<mws_font> smallest_font;
      mws_sp<mws_font> largest_font;
   };
}


void mod_font_metrix::init_mws()
{
   mws_root->new_page<mod_font_metrix_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}
