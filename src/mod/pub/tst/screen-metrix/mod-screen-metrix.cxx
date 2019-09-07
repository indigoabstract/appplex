#include "stdafx.hxx"

#include "mod-screen-metrix.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"


mod_screen_metrix::mod_screen_metrix() : mws_mod(mws_stringify(MOD_SCREEN_METRIX)) {}

mws_sp<mod_screen_metrix> mod_screen_metrix::nwi()
{
   return mws_sp<mod_screen_metrix>(new mod_screen_metrix());
}


namespace mod_screen_metrix_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         auto dpi = pfm_main::gi()->get_screen_dpi();
         // ready icon
         {
            test_obj = gfx_2d_sprite::nwi();
            gfx_2d_sprite& vxo = *test_obj;

            vxo.set_anchor(gfx_2d_sprite::e_top_left);
            vxo.name = "test-obj";
            vxo.visible = true;
            vxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
            vxo["u_v4_color"] = gfx_color::colors::green.to_vec4();
            vxo[MP_DEPTH_TEST] = true;
            vxo[MP_DEPTH_WRITE] = true;
            vxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
            vxo[MP_BLENDING] = MV_NONE;
            vxo[MP_CULL_BACK] = false;
            vxo[MP_CULL_FRONT] = false;
            vxo.camera_id_list.clear();
            vxo.camera_id_list.push_back("mws_cam");
            vxo.set_dimensions(1, 1);
            attach(test_obj);
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
         mws_sp<mws_font> font = i_g->get_font();
         float font_height = font->get_height();
         float horz_off = tl_margin_x.to_px().val();
         float vert_off = tl_margin_y.to_px().val() + height.to_px().val();
         std::string box_left_margin = mws_to_str_fmt("box left margin [ %2.2f ] cm", tl_margin_x.val());
         std::string box_top_margin = mws_to_str_fmt("box top margin [ %2.2f ] cm", tl_margin_y.val());
         std::string box_width = mws_to_str_fmt("box width [ %2.2f ] cm", width.val());
         std::string box_height = mws_to_str_fmt("box height [ %2.2f ] cm", height.val());
         std::string screen_width = mws_to_str_fmt("screen width [ %2.2f ] cm", pfm_main::gi()->get_screen_dim_cm().first);
         std::string screen_height = mws_to_str_fmt("screen height [ %2.2f ] cm", pfm_main::gi()->get_screen_dim_cm().second);
         std::string screen_res_width = mws_to_str_fmt("screen res width [ %d ] px", pfm_main::gi()->get_screen_res_px().first);
         std::string screen_res_height = mws_to_str_fmt("screen res height [ %d ] px", pfm_main::gi()->get_screen_res_px().second);
         float row_height = font_height;
         float y_off = vert_off - row_height;

         i_g->drawText(box_left_margin, horz_off, y_off += row_height);
         i_g->drawText(box_top_margin, horz_off, y_off += row_height);
         i_g->drawText(box_width, horz_off, y_off += row_height);
         i_g->drawText(box_height, horz_off, y_off += row_height);
         i_g->drawText(screen_width, horz_off, y_off += row_height);
         i_g->drawText(screen_height, horz_off, y_off += row_height);
         i_g->drawText(screen_res_width, horz_off, y_off += row_height);
         i_g->drawText(screen_res_height, horz_off, y_off += row_height);
      }

      virtual void on_resize() override
      {
         glm::vec2 size(width.to_px(mws_dim::horizontal).val(), height.to_px(mws_dim::vertical).val());
         glm::vec2 tl_margin(tl_margin_x.to_px(mws_dim::horizontal).val(), tl_margin_y.to_px(mws_dim::vertical).val());

         test_obj->set_scale(glm::vec2(size));
         test_obj->set_translation(tl_margin.x, tl_margin.y);
      }

      mws_cm width = 6.f;
      mws_cm height = 3.f;
      mws_cm tl_margin_x = 1.f;
      mws_cm tl_margin_y = 1.f;
      mws_sp<gfx_2d_sprite> test_obj;
   };
}


void mod_screen_metrix::init_mws()
{
   mws_root->new_page<mod_screen_metrix_ns::main_page>();
   mws_cam->clear_color = true;
}
