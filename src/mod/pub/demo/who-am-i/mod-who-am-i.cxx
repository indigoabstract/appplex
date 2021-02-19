#include "stdafx.hxx"

#include "mod-who-am-i.hxx"
#include "gfx.hxx"
#include "gfx-pbo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "fonts/mws-font-db.hxx"
#include "fonts/mws-font.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_who_am_i::mod_who_am_i() : mws_mod(mws_stringify(MOD_WHO_AM_I))
{
   settings_v.show_fps = false;
}

mws_sp<mod_who_am_i> mod_who_am_i::nwi()
{
   return mws_sp<mod_who_am_i>(new mod_who_am_i());
}

void mod_who_am_i::init()
{
   // set brightness
   mws::screen::set_brightness(1.f);
}


namespace mod_who_am_i_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
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
            font = mws_font::nwi(font_file->filename(), mws_cm(0.35f));
            font->set_color(gfx_color::colors::white);
            mws_font_db::inst()->set_global_font(font);
         }
         {
            letter_font = mws_font::nwi(font, mws_px(590));
            letter_font->set_color(gfx_color::colors::white);
            text_font = mws_font::nwi(font, mws_px(120));
            text_font->set_color(gfx_color::colors::white);
            glm::vec2 letter_dim = letter_font->get_text_dim("I");
            //struct { mws_px x; mws_px y; } letter_dim = { mws_pt(letter_dim_pt.x).to_px(),  mws_pt(letter_dim_pt.y).to_px() };
            mws_sp<mws_camera> cam = get_mod()->mws_cam;
            glm::ivec2 tex_dim(256, 512);
            glm::ivec2 scr_dim(mws::screen::get_width(), mws::screen::get_height());

            gfx_tex_params prm;
            prm.max_anisotropy = 0.f;
            prm.regen_mipmaps = true;
            font_tex.init("mws-vkb-keys-border-tex", tex_dim.x, tex_dim.y, &prm);
            gfx::i()->rt.set_current_render_target(font_tex.get_rt());
            gfx_rt::clear_buffers();
            //gfx_rt::clear_buffers(true, true, true, gfx_color::colors::blue);
            cam->drawText("I", (tex_dim.x - letter_dim.x) / 2.f, (tex_dim.y - 0.7f * letter_dim.y) / 2.f, letter_font);
            cam->update_camera_state();
            gfx::i()->rt.set_current_render_target();

            font_quad = gfx_quad_2d::nwi();
            font_quad->camera_id_list = { "mws_cam" };
            (*font_quad)[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            (*font_quad)[MP_BLENDING] = MV_NONE;
            (*font_quad)[MP_DEPTH_TEST] = false;
            (*font_quad)[MP_DEPTH_WRITE] = false;
            (*font_quad)["u_s2d_tex"][MP_TEXTURE_INST] = font_tex.get_tex();
            font_quad->set_anchor(gfx_quad_2d::e_center);
            font_quad->set_translation(scr_dim.x / 2.f, scr_dim.y / 2.f);
            font_quad->set_scale(1.f, 1.f);
            font_quad->set_v_flip(true);
            //attach(font_quad);
            slider_seconds_vect =
            {
               5.f, 4.f, 3.f, 2.5f, 2.f, 1.66f, 1.33f, 1.f, 0.9f, 0.8f, 0.7f, 0.6f, 0.55f,
               0.5f, 0.45f, 0.4f, 0.35f, 0.3f, 0.25f, 0.2f, 0.15f, 0.1f, 0.1f, 0.1f, 0.1f,
               0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f, 0.05f,
               0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f
            };
         }

         text_0_width = glm::vec3(text_font->get_text_width(who_text), text_font->get_text_width(am_text), text_font->get_text_width(iq_text));
         text_1_width = glm::vec3(text_font->get_text_width(deep_text), text_font->get_text_width(dreamless_text), text_font->get_text_width(sleep_text));
         last_stage_switch_time = mws::time::get_time_millis();
         //stage = stages_types::e_whoami_white_bg_stage;
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
      {
         mws_page::receive(i_dp);
      }

      virtual void update_state() override
      {
      }

      enum stages_types
      {
         e_i_i_stage,
         e_whoami_white_bg_stage,
         e_whoami_bg_fade_stage,
         e_whoami_stay_stage,
         e_whoami_fg_fade_stage,
         e_inter_text_stage,
         e_deep_stage,
         e_dreamless_stage,
         e_sleep_stage,
         e_final_stay_stage,
         e_final_fade_stage,
         e_final_stage,
      };

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         uint32_t crt_time = mws::time::get_time_millis();

         switch (stage)
         {
         case stages_types::e_i_i_stage:
         {
            glm::vec2 scr_dim((float)mws::screen::get_width(), (float)mws::screen::get_height());
            double v = 1.f;
            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            if (!slider.is_enabled())
            {
               if (slider_idx >= slider_seconds_vect.size())
               {
                  slider_idx = 0;
                  stage = stages_types::e_whoami_white_bg_stage;
                  last_stage_switch_time = crt_time;
               }
               else
               {
                  float span = slider_seconds_vect[slider_idx];
                  slider.start(span);
                  slider_idx++;
                  slider.update();
                  v = slider.get_value();
               }
            }
            else
            {
               slider.update();
               v = slider.get_value();
            }

            // https://sol.gfxile.net/interpolation/
            v = v * v * v * v * v * v * v * v * v * v * v;
            // 'I' width is around 16% of the texture width
            float max_width = scr_dim.x / 0.16f;
            float mixf = glm::mix(0.1f, max_width, (float)v);
            mws_sp<gfx_tex> tex = font_tex.get_tex();
            glm::vec2 tex_dim(mixf, mixf * 2.f);
            glm::vec2 off((scr_dim - tex_dim) / 2.f);

            i_g->drawImage(tex, off.x, off.y, tex_dim.x, tex_dim.y);
            break;
         }
         case stages_types::e_whoami_white_bg_stage:
         {
            uint32_t delta_t = crt_time - last_stage_switch_time;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::white);

            if (delta_t > 4000)
            {
               stage = stages_types::e_whoami_bg_fade_stage;
               last_stage_switch_time = crt_time;
               text_font->set_color(gfx_color::colors::white);
            }
            break;
         }
         case stages_types::e_whoami_bg_fade_stage:
         {
            const uint32_t duration = 5000;
            uint32_t delta_t = crt_time - last_stage_switch_time;
            float v = (float)delta_t / duration;
            v = glm::clamp<float>(v, 0.f, 1.f);
            v = 1.f - v * v * v * v * v * v;
            gfx_color bg = gfx_color::from_float(v, v, v);
            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_0_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, bg);

            i_g->drawText(who_text, text_xoff.x, y_off, text_font);
            i_g->drawText(am_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(iq_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_whoami_stay_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_whoami_stay_stage:
         {
            const uint32_t duration = 3000;
            uint32_t delta_t = crt_time - last_stage_switch_time;
            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_0_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            i_g->drawText(who_text, text_xoff.x, y_off, text_font);
            i_g->drawText(am_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(iq_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_whoami_fg_fade_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_whoami_fg_fade_stage:
         {
            const uint32_t duration = 5000;
            uint32_t delta_t = crt_time - last_stage_switch_time;
            float v = (float)delta_t / duration;
            v = glm::clamp<float>(v, 0.f, 1.f);
            v = 1.f - v * v * v * v * v;
            gfx_color fg = gfx_color::from_float(v, v, v);
            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_0_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);
            text_font->set_color(fg);

            i_g->drawText(who_text, text_xoff.x, y_off, text_font);
            i_g->drawText(am_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(iq_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_inter_text_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_inter_text_stage:
         {
            const uint32_t duration = 3000;
            uint32_t delta_t = crt_time - last_stage_switch_time;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            if (delta_t > duration)
            {
               stage = stages_types::e_deep_stage;
               last_stage_switch_time = crt_time;
               text_font->set_color(gfx_color::colors::white);
            }
            break;
         }
         case stages_types::e_deep_stage:
         {
            const uint32_t duration = 1500;
            uint32_t delta_t = crt_time - last_stage_switch_time;

            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_1_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            i_g->drawText(deep_text, text_xoff.x, y_off, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_dreamless_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_dreamless_stage:
         {
            const uint32_t duration = 1500;
            uint32_t delta_t = crt_time - last_stage_switch_time;

            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_1_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            i_g->drawText(deep_text, text_xoff.x, y_off, text_font);
            i_g->drawText(dreamless_text, text_xoff.y, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_sleep_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_sleep_stage:
         {
            const uint32_t duration = 1500;
            uint32_t delta_t = crt_time - last_stage_switch_time;

            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_1_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            i_g->drawText(deep_text, text_xoff.x, y_off, text_font);
            i_g->drawText(dreamless_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(sleep_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_final_stay_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_final_stay_stage:
         {
            const uint32_t duration = 5000;
            uint32_t delta_t = crt_time - last_stage_switch_time;
            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_1_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            i_g->drawText(deep_text, text_xoff.x, y_off, text_font);
            i_g->drawText(dreamless_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(sleep_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_final_fade_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_final_fade_stage:
         {
            const uint32_t duration = 5000;
            uint32_t delta_t = crt_time - last_stage_switch_time;
            float v = (float)delta_t / duration;
            v = glm::clamp<float>(v, 0.f, 1.f);
            v = 1.f - v * v * v * v * v;
            gfx_color fg = gfx_color::from_float(v, v, v);
            float font_height = text_font->get_height();
            float y_off = (mws::screen::get_height() - 3.f * font_height) / 2.f;
            glm::vec3 text_xoff = (glm::vec3((float)mws::screen::get_width()) - text_1_width) / 2.f;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);
            text_font->set_color(fg);

            i_g->drawText(deep_text, text_xoff.x, y_off, text_font);
            i_g->drawText(dreamless_text, text_xoff.y, y_off += font_height, text_font);
            i_g->drawText(sleep_text, text_xoff.z, y_off += font_height, text_font);

            if (delta_t > duration)
            {
               stage = stages_types::e_final_stage;
               last_stage_switch_time = crt_time;
            }
            break;
         }
         case stages_types::e_final_stage:
         {
            const uint32_t duration = 5000;
            uint32_t delta_t = crt_time - last_stage_switch_time;

            gfx_rt::clear_buffers(true, false, false, gfx_color::colors::black);

            if (delta_t > duration)
            {
               stage = stages_types::e_i_i_stage;
               last_stage_switch_time = crt_time;
               text_font->set_color(gfx_color::colors::white);
            }
            break;
         }
         }
      }

      stages_types stage = e_i_i_stage;
      std::string who_text = "Who";
      std::string am_text = "Am";
      std::string iq_text = "I?";
      std::string deep_text = "Deep";
      std::string dreamless_text = "Dreamless";
      std::string sleep_text = "Sleep";
      glm::vec3 text_0_width;
      glm::vec3 text_1_width;
      uint32_t last_stage_switch_time = 0;
      mws_sp<mws_font> font;
      mws_sp<mws_font> letter_font;
      mws_sp<mws_font> text_font;
      mws_gfx_ppb font_tex;
      mws_sp<gfx_quad_2d> font_quad;
      basic_time_slider<float> slider;
      std::vector<float> slider_seconds_vect;
      uint32_t slider_idx = 0;
   };
}


void mod_who_am_i::init_mws()
{
   mws_root->new_page<mod_who_am_i_ns::main_page>();
   mws_cam->clear_color = false;
}

void mod_who_am_i::load()
{
}
