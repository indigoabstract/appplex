#include "stdafx.hxx"

#include "mod-font-metrix.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-pbo.hxx"
#include "gfx-rt.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"
#include <vector>


mod_font_metrix::mod_font_metrix() : mws_mod(mws_stringify(MOD_FONT_METRIX))
{
   settings_v.font_db_pow_of_two_size = 13;
}

mws_sp<mod_font_metrix> mod_font_metrix::nwi()
{
   return mws_sp<mod_font_metrix>(new mod_font_metrix());
}


namespace mod_font_metrix_ns
{
   struct font_pixel_info
   {
      // font size in points(pt) used to generate the font
      uint32_t font_size;
      // first non zero pixel's vertical coord
      uint32_t vert_start;
      // last non zero pixel's vertical coord
      uint32_t vert_end;
      // max measured height in pixels
      uint32_t height;
   };


   struct font_step
   {
      uint32_t start_size;
      uint32_t end_size;
      uint32_t step;
   };


   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_sp<mws_camera> cam = get_mod()->mws_cam;
         gfx_tex_params params;

         smallest_font_size = font_steps.front().start_size;
         largest_font_size = font_steps.back().end_size;
         current_font_size = smallest_font_size;

         if (!font_name.empty())
         {
            ref_font = mws_font::nwi(font_name, mws_pt((float)smallest_font_size));
         }

         if (!ref_font)
         {
            ref_font = mws_font::nwi(cam->get_font());
         }

         params.set_rt_params();
         params.mag_filter = gfx_tex_params::e_tf_nearest;
         params.min_filter = gfx_tex_params::e_tf_nearest;
         font_tex.init("font-tex", 512, 1024);
         tex_dim = glm::vec2(font_tex.get_tex()->get_width(), font_tex.get_tex()->get_height());
         cam->set_color(gfx_color::colors::cyan);
         size_height_mixer.set_edges(0.f, 0.f);
         height_size_mixer.set_edges(0.f, 0.f);

         {
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

         if (!finished)
         {
            if (current_font_size >= largest_font_size)
            {
               finished = true;
            }

            mws_sp<mws_camera> cam = get_mod()->mws_cam;
            mws_sp<mws_font> font = mws_font::nwi(ref_font, (float)current_font_size);
            const font_step& crt_font_step = font_steps[crt_font_step_idx];
            uint32_t font_step_size = crt_font_step.step;

            {
               font_pixel_info fpi = draw_letters(cam, font);
               fpi_vect.push_back(fpi);
               float alpha = float(current_font_size - smallest_font_size) / (largest_font_size - smallest_font_size);
               float height = (float)fpi.height;

               size_height_mixer.set_val_at(height, alpha);
               current_font_size += font_step_size;

               if (current_font_size >= crt_font_step.end_size)
               {
                  if (current_font_size - font_step_size == crt_font_step.end_size)
                  {
                     if (!finished)
                     {
                        crt_font_step_idx++;
                        const font_step& next_font_step = font_steps[crt_font_step_idx];
                        current_font_size = next_font_step.start_size;
                     }
                     else
                     {
                        current_font_size = largest_font_size;
                     }
                  }
                  else
                  {
                     current_font_size = crt_font_step.end_size;
                  }
               }
            }

            if (finished)
            {
               font_pixel_info& start = fpi_vect.front();
               font_pixel_info& end = fpi_vect.back();
               uint32_t smallest_height = start.height;
               uint32_t largest_height = end.height;

               mws_nl();

               for (uint32_t k = 0; k < fpi_vect.size(); k++)
               {
                  font_pixel_info& fpi = fpi_vect[k];
                  float size_alpha = float(fpi.font_size - start.font_size) / (end.font_size - start.font_size);
                  float height_alpha = float(fpi.height - smallest_height) / (largest_height - smallest_height);
                  float interpolated_height = end.height * size_alpha + start.height * (1.f - size_alpha);
                  float height = size_height_mixer.get_val_at(size_alpha);

                  height_size_mixer.set_val_at((float)fpi.font_size, height_alpha);
                  mws_println("font size[ %d ] v-start[ %d ] v-end[ %d ] height[ %d ] interpolated height[ %2.2f ] interpolated height 2[ %2.2f ]",
                     fpi.font_size, fpi.vert_start, fpi.vert_end, fpi.height, interpolated_height, height);
               }

               mws_nl();
               mws_print("font-sizes[ %d, %d ], pixel-heights[ %d, %d ] font-size/pixel-height =\n{ ",
                  start.font_size, end.font_size, smallest_height, largest_height);

               for (uint32_t k = 0; k < size_height_mixer.size(); k++)
               {
                  auto& pos_val = size_height_mixer[k];
                  mws_print("{%ff, %.0f.f}", pos_val.pos, pos_val.val);
                  if (k < size_height_mixer.size() - 1) { mws_print(", "); }
               }
               mws_println(" };");

               mws_nl();
               mws_print("*copy this data* into mws_font_db:\npixel-heights[ %d, %d ], font-sizes[ %d, %d ], pixel-height/font-size =\n{ ",
                  smallest_height, largest_height, start.font_size, end.font_size);

               for (uint32_t k = 0; k < height_size_mixer.size(); k++)
               {
                  auto& pos_val = height_size_mixer[k];
                  mws_print("{%ff, %.0f.f}", pos_val.pos, pos_val.val);
                  if (k < height_size_mixer.size() - 1) { mws_print(", "); }
               }
               mws_println(" };\n");
            }
         }
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         i_g->drawText(mws_to_str_fmt("current font size[ %d ]", current_font_size), 50, 50);

         if (!finished)
         {
            i_g->update_camera_state();
            mws_font_db::inst()->clear_db();
         }
      }

      virtual void on_resize() override
      {
      }

      font_pixel_info draw_letters(mws_sp<mws_camera> i_g, mws_sp<mws_font> i_font)
      {
         font_pixel_info fpi;
         glm::vec2 letter_dim = i_font->get_text_dim("M");

         gfx::i()->rt.set_current_render_target(font_tex.get_rt());
         mws_sp<gfx_rt> rt = gfx::i()->rt.get_current_render_target();
         gfx_rt::clear_buffers();

         //glm::vec2 pos(0.f, 0.f);
         glm::vec2 pos((tex_dim - letter_dim) / 2.f);

         for (auto c : text)
         {
            i_g->drawText(std::string(1, c), pos.x, pos.y, i_font);
         }

         i_g->update_camera_state();
         mws_sp<std::vector<uint32_t>> pixels = gfx::i()->rt.get_render_target_pixels<uint32_t>(rt);
         gfx::i()->rt.set_current_render_target();

         {
            uint32_t w = rt->get_width();
            uint32_t h = rt->get_height();
            int32_t font_v_start = -1;
            int32_t font_v_end = -1;

            for (uint32_t k = 0; k < h; k++)
            {
               for (uint32_t i = 0; i < w; i++)
               {
                  uint32_t pixel = pixels->at(k * w + i);

                  if (pixel != 0)
                  {
                     if (font_v_start < 0)
                     {
                        font_v_start = k;
                     }
                     else
                     {
                        font_v_end = k;
                     }
                  }
               }
            }

            fpi.font_size = (uint32_t)i_font->get_size();
            fpi.height = font_v_end - font_v_start;
            fpi.vert_end = font_v_end;
            fpi.vert_start = font_v_start;
         }

         return fpi;
      }

      std::string text = " !#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
      mws_gfx_ppb font_tex;
      glm::vec2 tex_dim;
      mws_sp<gfx_2d_sprite> font_quad;
      mws_sp<mws_font> ref_font;
      std::vector<font_step> font_steps = { {2, 19, 1}, {20, 90, 10}, {100, 1000, 100} };
      uint32_t smallest_font_size = 0;
      uint32_t largest_font_size = 0;
      uint32_t current_font_size = 0;
      uint32_t crt_font_step_idx = 0;
      std::string font_name;// = "consolas.ttf";
      std::vector<font_pixel_info> fpi_vect;
      struct mix_f { float operator()(const float& i_c0, const float& i_c1, float i_f) { return (1.f - i_f) * i_c0 + i_f * i_c1; } };
      mws_val_mixer<float, mix_f> size_height_mixer;
      mws_val_mixer<float, mix_f> height_size_mixer;
      bool finished = false;
   };
}


void mod_font_metrix::init_mws()
{
   mws_root->new_page<mod_font_metrix_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}
