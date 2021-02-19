#include "stdafx.hxx"

#include "mod-multi-touch-input.hxx"
#include "gfx-quad-2d.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_multi_touch_input::mod_multi_touch_input() : mws_mod(mws_stringify(MOD_MULTI_TOUCH_INPUT)) {}

mws_sp<mod_multi_touch_input> mod_multi_touch_input::nwi()
{
   return mws_sp<mod_multi_touch_input>(new mod_multi_touch_input());
}


namespace mod_multi_touch_input_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_page::init();

         std::vector<gfx_color> color_vect =
         {
            gfx_color::colors::blue, gfx_color::colors::cyan, gfx_color::colors::green, gfx_color::colors::orange,
            gfx_color::colors::red, gfx_color::colors::white, gfx_color::colors::yellow, gfx_color::colors::magenta,
         };

         ptr_vxo_assigned.assign(mws_ptr_evt::max_touch_points, false);
         ptr_vxo_vect.resize(mws_ptr_evt::max_touch_points);

         for(uint32_t k = 0; k < mws_ptr_evt::max_touch_points; k++)
         {
            ptr_vxo_vect[k] = gfx_2d_sprite::nwi();
            gfx_2d_sprite& vxo = *ptr_vxo_vect[k];
            vxo.name = mws_to_str_fmt("ptr-%d", k);
            vxo.visible = false;
            vxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
            vxo["u_v4_color"] = color_vect[k].to_vec4();
            vxo[MP_DEPTH_TEST] = true;
            vxo[MP_DEPTH_WRITE] = true;
            vxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
            vxo[MP_BLENDING] = MV_NONE;
            vxo[MP_CULL_BACK] = false;
            vxo[MP_CULL_FRONT] = false;
            vxo.camera_id_list.clear();
            vxo.camera_id_list.push_back("mws_cam");
            vxo.set_dimensions(1, 1);
            vxo.set_scale(glm::vec2(mws_cm(2.f).to_px().val()));
            attach(ptr_vxo_vect[k]);
         }
         // ready icon
         {
            ready_icon_size_px = mws_cm(3.f).to_px().val();
            ready_vxo = gfx_2d_sprite::nwi();
            gfx_2d_sprite& vxo = *ready_vxo;
            vxo.name = "ready-vxo";
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
            vxo.set_scale(glm::vec2(ready_icon_size_px));
            vxo.set_translation(ready_icon_size_px, ready_icon_size_px);
            attach(ready_vxo);
         }
      }

      virtual void receive(mws_sp<mws_dp> i_dp) override
      {
         if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
         {
            mws_sp<mws_ptr_evt> pe = mws_ptr_evt::as_pointer_evt(i_dp);

            if (pe->touch_count > 0)
            {
               touch_0 = mws_ptr_evt::get_pos(pe->points[0]);
            }

            switch (pe->type)
            {
            case mws_ptr_evt::touch_invalid:
            {
               break;
            }

            case mws_ptr_evt::touch_began:
            {
               for (uint32_t k = 0; k < pe->touch_count; k++)
               {
                  mws_ptr_evt::touch_point& pt = pe->points[k];

                  if (pt.is_changed)
                  {
                     int idx = -1;

                     for (int i = 0; i < mws_ptr_evt::max_touch_points; i++)
                     {
                        if (ptr_vxo_assigned[i] == false)
                        {
                           idx = i;
                           ptr_vxo_assigned[idx] = true;
                           ptr_id_to_vxo[pt.identifier] = idx;
                           break;
                        }
                     }

                     if (idx >= 0)
                     {
                        ptr_vxo_vect[idx]->set_translation(pt.x, pt.y);
                        ptr_vxo_vect[idx]->visible = true;
                     }
                     else
                     {
                        mws_println("ERROR[ run out of ids ]");
                     }
                  }
               }
               break;
            }

            case mws_ptr_evt::touch_moved:
            {
               for (uint32_t k = 0; k < pe->touch_count; k++)
               {
                  mws_ptr_evt::touch_point& pt = pe->points[k];

                  if (pt.is_changed)
                  {
                     auto it = ptr_id_to_vxo.find(pt.identifier);

                     if (it != ptr_id_to_vxo.end())
                     {
                        ptr_vxo_vect[it->second]->set_translation(pt.x, pt.y);

                     }
                     else
                     {
                        mws_println("ERROR[ touch_moved: invalid id [%d] at idx [%d] ]", pt.identifier, k);
                     }
                  }
               }
               break;
            }

            case mws_ptr_evt::touch_ended:
            {
               for (uint32_t k = 0; k < pe->touch_count; k++)
               {
                  mws_ptr_evt::touch_point& pt = pe->points[k];

                  if (pt.is_changed)
                  {
                     auto it = ptr_id_to_vxo.find(pt.identifier);

                     if (it != ptr_id_to_vxo.end())
                     {
                        ptr_vxo_vect[it->second]->set_translation(pt.x, pt.y);
                        ptr_vxo_vect[it->second]->visible = false;
                        ptr_vxo_assigned[it->second] = false;
                        ptr_id_to_vxo.erase(it);
                     }
                     else
                     {
                        mws_println("ERROR[ touch_ended: invalid id [%d] at idx [%d] ]", pt.identifier, k);
                     }
                  }
               }
               break;
            }

            case mws_ptr_evt::touch_cancelled:
            {
               break;
            }
            }

            process(pe);
         }
      }

      virtual void update_view(mws_sp<mws_camera> i_g) override
      {
         std::string txt = mws_to_str_fmt("t0 [ %4.2f, %4.2f ] cw %d ch %d", touch_0.x, touch_0.y, mws::screen::get_width(), mws::screen::get_height());

         i_g->draw_text(txt, 0.5f * ready_icon_size_px, 1.5f * ready_icon_size_px);
      }

      mws_sp<gfx_2d_sprite> ready_vxo;
      std::vector<mws_sp<gfx_2d_sprite>> ptr_vxo_vect;
      std::vector<bool> ptr_vxo_assigned;
      std::unordered_map<uintptr_t, int> ptr_id_to_vxo;
      glm::vec2 touch_0 = glm::vec2(0.f);
      float ready_icon_size_px = 0.f;
   };
}


void mod_multi_touch_input::init_mws()
{
   mws_root->new_page<mod_multi_touch_input_ns::main_page>();
   mws_cam->clear_color = true;
}
