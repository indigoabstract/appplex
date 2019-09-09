#include "stdafx.hxx"

#include "mod-glowing-text.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"
#include "mws/font-db.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "gfx-tex.hxx"
#include <array>


mod_glowing_text::mod_glowing_text() : mws_mod(mws_stringify(MOD_GLOWING_TEXT)) {}

mws_sp<mod_glowing_text> mod_glowing_text::nwi()
{
   return mws_sp<mod_glowing_text>(new mod_glowing_text());
}


namespace mod_glowing_text_ns
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
         // tex
         {
            gfx_tex_params prm;

            prm.set_rt_params();
            input_tex = gfx::i()->tex.nwi("tex", 512, 512, &prm);
         }
         // rt
         {
            input_rt = gfx::i()->rt.new_rt();
            input_rt->set_color_attachment(input_tex);
         }
         // tex quad
         {
            input_quad = gfx_quad_2d::nwi();
            auto& rvxo = *input_quad;

            rvxo.camera_id_list.clear();
            rvxo.camera_id_list.push_back("mws_cam");
            //rvxo[MP_BLENDING] = MV_ALPHA;
            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = input_tex;
            rvxo.set_dimensions(1, 1);
            rvxo.set_translation(50, 50);
            rvxo.set_scale((float)input_tex->get_width(), (float)input_tex->get_width());
            rvxo.set_v_flip(true);

            attach(input_quad);
         }
         // ping pong buffers
         {
            gfx_tex_params prm;

            prm.set_rt_params();
            kawase_blur_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(kawase_blur_sh_id);

            for (uint32 k = 0; k < ping_pong_vect.size(); k++)
            {
               fx_buffer& rt = ping_pong_vect[k];

               rt.tex = gfx::i()->tex.nwi(mws_to_str_fmt("tex-%d", k), input_tex->get_width(), input_tex->get_width(), &prm);
               rt.rt = gfx::i()->rt.new_rt();
               rt.rt->set_color_attachment(rt.tex);
               rt.quad = gfx_quad_2d::nwi();
               auto& rvxo = *rt.quad;

               rvxo.camera_id_list.clear();
               rvxo.camera_id_list.push_back("mws_cam");
               //rvxo[MP_BLENDING] = MV_ALPHA;
               rvxo[MP_SHADER_NAME] = kawase_blur_sh_id;
               //rvxo["u_s2d_tex"][MP_TEXTURE_INST] = rt.tex;
               rvxo.set_dimensions(1, 1);
               rvxo.set_scale((float)rt.tex->get_width(), (float)rt.tex->get_width());
               rvxo.set_v_flip(true);
            }

            (*ping_pong_vect[0].quad)["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[1].tex;
            (*ping_pong_vect[1].quad)["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[0].tex;

            // accumulation buffer
            {
               fx_buffer& rt = accumulation_buff;

               accumulation_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(accumulation_sh_id);
               rt.tex = gfx::i()->tex.nwi(mws_to_str_fmt("tex-acc-buff"), input_tex->get_width(), input_tex->get_width(), &prm);
               rt.rt = gfx::i()->rt.new_rt();
               rt.rt->set_color_attachment(rt.tex);
               rt.quad = gfx_quad_2d::nwi();
               auto& rvxo = *rt.quad;

               rvxo.camera_id_list.clear();
               rvxo.camera_id_list.push_back("mws_cam");
               rvxo[MP_BLENDING] = MV_ADD;
               rvxo[MP_SHADER_NAME] = accumulation_sh_id;
               //rvxo["u_s2d_tex"][MP_TEXTURE_INST] = rt.tex;
               rvxo["u_v1_mul_fact"] = u_v1_mul_fact;
               rvxo.set_dimensions(1, 1);
               rvxo.set_scale((float)rt.tex->get_width(), (float)rt.tex->get_width());
               rvxo.set_v_flip(true);
            }
         }
         // output quad
         {
            output_quad = gfx_quad_2d::nwi();
            auto& rvxo = *output_quad;

            rvxo.camera_id_list.clear();
            rvxo.camera_id_list.push_back("mws_cam");
            rvxo[MP_BLENDING] = MV_ADD;
            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = input_tex;
            rvxo.set_dimensions(1, 1);
            rvxo.set_translation(100 + (float)input_tex->get_width(), 50);
            rvxo.set_translation(50, 50);
            rvxo.set_scale((float)input_tex->get_width(), (float)input_tex->get_width());
            rvxo.set_v_flip(true);

            attach(output_quad);
         }
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override { mws_page::receive(i_dp); }
      virtual void update_state() override {}

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         if (!tex_init)
         {
            update_rt(i_g);
            tex_init = true;
         }
      }

      void update_rt(mws_sp<mws_camera> i_g)
      {
         mws_sp<mws_font> fnt = i_g->get_font();
         mws_sp<mws_font> fnt_big = mws_font::nwi(fnt, mws_cm(1.2f));
         fnt_big->set_color(font_glow_color);
         std::string text = "glowing text";
         float text_width = fnt_big->get_text_width(text);
         std::array<mws_sp<gfx_rt>, 2> rt_vect = { input_rt, ping_pong_vect[1].rt };
         mws_sp<gfx_tex> output_tex = ping_pong_vect[1].tex;

         // put the input texture in input_tex ping_pong_vect[1].tex, to be used by ping_pong_vect[0]
         for (mws_sp<gfx_rt> rt : rt_vect)
         {
            gfx::i()->rt.set_current_render_target(rt);
            i_g->drawText(text, (input_tex->get_height() - text_width) / 2.f, (input_tex->get_height() - fnt_big->get_height()) / 2.f, fnt_big);
            i_g->update_camera_state();
            gfx::i()->rt.set_current_render_target();
         }

         for (uint32 k = 0; k < iteration_count; k++)
         {
            float sample_factor = k + 1.5f;
            fx_buffer& rt = ping_pong_vect[k % 2];

            gfx::i()->rt.set_current_render_target(rt.rt);
            (*rt.quad)["u_v2_offset"] = glm::vec2(sample_factor / rt.tex->get_width(), sample_factor / rt.tex->get_width());
            rt.quad->draw_out_of_sync(i_g);
            output_tex = rt.tex;
            gfx::i()->rt.set_current_render_target(accumulation_buff.rt);
            accumulation_buff.quad->draw_out_of_sync(i_g);
            gfx::i()->rt.set_current_render_target();
         }
         // set color to white
         {
            gfx::i()->rt.set_current_render_target(input_rt);
            fnt_big->set_color(gfx_color::colors::white);
            i_g->drawText(text, (input_tex->get_height() - text_width) / 2.f, (input_tex->get_height() - fnt_big->get_height()) / 2.f, fnt_big);
            i_g->update_camera_state();
            gfx::i()->rt.set_current_render_target();
         }

         //(*output_quad)["u_s2d_tex"][MP_TEXTURE_INST] = output_tex;
         (*output_quad)["u_s2d_tex"][MP_TEXTURE_INST] = accumulation_buff.tex;
      }

      bool tex_init = false;
      static const uint32 iteration_count = 19;
      static const inline float u_v1_mul_fact = 0.745f;
      static const inline gfx_color font_glow_color = gfx_color::from_argb(0xff007fff);
      mws_sp<gfx_tex> input_tex;
      mws_sp<gfx_rt> input_rt;
      mws_sp<gfx_quad_2d> input_quad;
      struct fx_buffer { mws_sp<gfx_tex> tex; mws_sp<gfx_rt> rt; mws_sp<gfx_quad_2d> quad; };
      std::array<fx_buffer, 2> ping_pong_vect;
      mws_sp<gfx_shader> kawase_blur_shader;
      static const inline std::string kawase_blur_sh_id = "kawase-blur";
      fx_buffer accumulation_buff;
      mws_sp<gfx_shader> accumulation_shader;
      static const inline std::string accumulation_sh_id = "accumulation";
      mws_sp<gfx_quad_2d> output_quad;
   };
}


void mod_glowing_text::init_mws()
{
   mws_root->new_page<mod_glowing_text_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}
