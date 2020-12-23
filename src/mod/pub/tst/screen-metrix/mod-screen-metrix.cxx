#include "stdafx.hxx"

#include "mod-screen-metrix.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"


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
         auto dpi = mws::screen::get_dpi();
         test_font = mws_font::nwi("consolas.ttf", height);
         test_font->set_color(gfx_color::colors::red);
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
         std::string screen_width = mws_to_str_fmt("screen width [ %2.2f ] cm", mws::screen::get_dim_cm().first);
         std::string screen_height = mws_to_str_fmt("screen height [ %2.2f ] cm", mws::screen::get_dim_cm().second);
         std::string screen_res_width = mws_to_str_fmt("screen res width [ %d ] px", mws::screen::get_res_px().first);
         std::string screen_res_height = mws_to_str_fmt("screen res height [ %d ] px", mws::screen::get_res_px().second);
         std::string screen_avg_dpi = mws_to_str_fmt("screen average dpi [ %f ] px", mws::screen::get_avg_dpi());
         std::string screen_avg_dpcm = mws_to_str_fmt("screen average dpcm [ %f ] px", mws::screen::get_avg_dpcm());
         std::string screen_h_dpcm = mws_to_str_fmt("screen e_horizontal dpcm [ %f ] px", mws::screen::get_dpcm().first);
         std::string screen_v_dpcm = mws_to_str_fmt("screen e_vertical dpcm [ %f ] px", mws::screen::get_dpcm().second);
         float row_height = font_height;
         float y_off = vert_off;// - row_height;

         i_g->drawText("_|{g", horz_off, tl_margin_y.to_px().val(), test_font);
         i_g->drawText(box_left_margin, horz_off, y_off += row_height);
         i_g->drawText(box_top_margin, horz_off, y_off += row_height);
         i_g->drawText(box_width, horz_off, y_off += row_height);
         i_g->drawText(box_height, horz_off, y_off += row_height);
         i_g->drawText(screen_width, horz_off, y_off += row_height);
         i_g->drawText(screen_height, horz_off, y_off += row_height);
         i_g->drawText(screen_res_width, horz_off, y_off += row_height);
         i_g->drawText(screen_res_height, horz_off, y_off += row_height);
         i_g->drawText(screen_avg_dpi, horz_off, y_off += row_height);
         i_g->drawText(screen_avg_dpcm, horz_off, y_off += row_height);
         i_g->drawText(screen_h_dpcm, horz_off, y_off += row_height);
         i_g->drawText(screen_v_dpcm, horz_off, y_off += row_height);
      }

      virtual void on_resize() override
      {
         glm::vec2 size(width.to_px(mws_dim::e_horizontal).val(), height.to_px(mws_dim::e_vertical).val());
         glm::vec2 tl_margin(tl_margin_x.to_px(mws_dim::e_horizontal).val(), tl_margin_y.to_px(mws_dim::e_vertical).val());

         test_obj->set_scale(glm::vec2(size));
         test_obj->set_translation(tl_margin.x, tl_margin.y);
      }

      mws_cm width = 6.f;
      mws_cm height = 3.f;
      mws_cm tl_margin_x = 1.f;
      mws_cm tl_margin_y = 1.f;
      mws_sp<mws_font> test_font;
      mws_sp<gfx_2d_sprite> test_obj;
   };
}


void mod_screen_metrix::init_mws()
{
   mws_sp<mws_file> font_file = mws_file::get_inst("consolas.ttf");

   if (font_file->exists())
   {
      std::pair<float, float> metrix[] =
      {
         {0.000000f, 2.f}, {0.002291f, 4.f}, {0.004582f, 6.f}, {0.005727f, 8.f}, {0.008018f, 10.f}, {0.011455f, 12.f}, {0.012600f, 14.f},
      {0.016037f, 16.f}, {0.017182f, 18.f}, {0.020619f, 20.f}, {0.032073f, 30.f}, {0.043528f, 40.f}, {0.054983f, 50.f}, {0.066438f, 60.f},
      {0.077892f, 70.f}, {0.089347f, 80.f}, {0.100802f, 90.f}, {0.112257f, 100.f}, {0.229095f, 200.f}, {0.343643f, 300.f}, {0.459336f, 400.f},
      {0.573883f, 500.f}, {0.689576f, 600.f}, {0.802978f, 700.f}, {0.915235f, 800.f}, {0.957617f, 900.f}, {1.000000f, 1000.f}
      };
      uint32_t size = sizeof(metrix) / sizeof(std::pair<float, float>);

      mws_font_db::inst()->store_font_metrix(font_file->filename(), mws_pt(2), mws_px(2), mws_pt(1000), mws_px(875), metrix, size);
      mws_sp<mws_font> font = mws_font::nwi(font_file->filename(), mws_cm(0.24f));
      mws_font_db::inst()->set_global_font(font);
   }

   mws_root->new_page<mod_screen_metrix_ns::main_page>();
   mws_cam->clear_color = true;
}
