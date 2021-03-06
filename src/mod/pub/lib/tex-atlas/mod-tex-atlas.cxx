#include "stdafx.hxx"

#include "mod-tex-atlas.hxx"
#include "mws-tex-atlas.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "rng/rng.hxx"


mod_tex_atlas::mod_tex_atlas() : mws_mod(mws_stringify(MOD_TEX_ATLAS)) {}

mws_sp<mod_tex_atlas> mod_tex_atlas::nwi()
{
   return mws_sp<mod_tex_atlas>(new mod_tex_atlas());
}


namespace mod_tex_atlas_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         tex_atlas = mws_tex_atlas::nwi(512, 512, 4);
         gen_atlas_regions();

         // sprite list
         {
            mws_sp<gfx_tex> tex = tex_atlas->get_tex();
            sprite_list = mws_atlas_sprite_list::nwi(tex_atlas);
            sprite_list->camera_id_list = { "mws_cam" };
            (*sprite_list)[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            (*sprite_list)["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            attach(sprite_list);

            fill_sprite_list();
         }
         // tex quad
         {
            mws_sp<gfx_tex> tex = tex_atlas->get_tex();
            tex_quad = gfx_quad_2d::nwi();
            auto& rvxo = *tex_quad;

            rvxo.camera_id_list = { "mws_cam" };
            //rvxo[MP_BLENDING] = MV_ALPHA;
            rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
            rvxo["u_s2d_tex"][MP_TEXTURE_INST] = tex;
            rvxo.set_dimensions(1, 1);
            rvxo.set_translation(50, 50);
            rvxo.set_scale((float)tex->get_width(), (float)tex->get_width());

            attach(tex_quad);
         }
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
      {
         if (i_dp->is_type(mws_key_evt::key_evt_type))
         {
            mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(i_dp);

            if (ke->get_type() == mws_key_evt::ke_pressed)
            {
               bool do_action = true;

               switch (ke->get_key())
               {
               case mws_key_r:
               {
                  gen_atlas_regions();
                  fill_sprite_list();
                  break;
               }

               default:
                  do_action = false;
               }

               if (do_action)
               {
                  process(ke);
               }
            }
         }

         if (!i_dp->is_processed())
         {
            mws_page::receive(i_dp);
         }
      }

      virtual void update_state() override
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
      }

      void gen_atlas_regions()
      {
         RNG rng;
         std::vector<gfx_color> color_vect =
         {
            gfx_color::colors::green, gfx_color::colors::cyan, gfx_color::colors::gold, gfx_color::colors::gray,
            gfx_color::colors::indigo, gfx_color::colors::magenta, gfx_color::colors::orange, gfx_color::colors::orchid,
            gfx_color::colors::red, gfx_color::colors::saddle_brown, gfx_color::colors::white, gfx_color::colors::yellow,
            gfx_color::colors::blue, gfx_color::colors::blue_violet, gfx_color::colors::pink, gfx_color::colors::thistle,
         };
         int elem_count = rng.range(10, 40);

         tex_atlas->clear();

         for (int k = 0; k < elem_count; k++)
         {
            gfx_color color = color_vect[rng.nextInt(color_vect.size())];
            int width = rng.range(15, 120);
            int height = rng.range(15, 120);
            mws_tex_atlas::region reg = tex_atlas->get_region(width, height);
            std::vector<gfx_color> tex;

            tex.assign(reg.rect.z * reg.rect.w, color);
            tex_atlas->set_region(reg, (const unsigned char*)tex.data(), reg.rect.z * sizeof(gfx_color));
         }

         tex_atlas->upload();
      }

      void fill_sprite_list()
      {
         mws_sp<gfx_tex> tex = tex_atlas->get_tex();
         float x_off = 100.f + (float)tex->get_width();
         float y_off = 50.f;

         sprite_list->clear();
         sprite_list->add(0, x_off, y_off);
         sprite_list->add(1, x_off + 150, y_off);
         sprite_list->add(2, x_off + 350, y_off);
         sprite_list->add(3, x_off, y_off + 150);
         sprite_list->add(4, x_off + 150, y_off + 150);
         sprite_list->add(5, x_off + 350, y_off + 150);
         sprite_list->push_data();
      }

      mws_sp<mws_tex_atlas> tex_atlas;
      mws_sp<gfx_quad_2d> tex_quad;
      mws_sp<mws_atlas_sprite_list> sprite_list;
   };
}


void mod_tex_atlas::init_mws()
{
   mws_root->new_page<mod_tex_atlas_ns::main_page>();
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::blue;
}
