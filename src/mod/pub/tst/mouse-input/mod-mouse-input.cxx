#include "stdafx.hxx"

#include "mod-mouse-input.hxx"
#include "input/input-ctrl.hxx"
#include "input/gesture-detectors.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "fonts/mws-font.hxx"
#include "gfx.hxx"
#include "gfx-vxo.hxx"
#include "gfx-vxo-ext.hxx"
#include <glm/inc.hpp>


class main_page : public mws_page
{
public:
   virtual void init() override
   {
      auto mws_mod = get_mod();
      remaining_fnt = mws_font::nwi(32.f);
      remaining_fnt->set_color(gfx_color::colors::red);

      persp_cam = gfx_camera::nwi();
      persp_cam->name = "defcam";
      persp_cam->camera_id = "default";
      persp_cam->rendering_priority = 1;
      persp_cam->near_clip_distance = 10.01f;
      persp_cam->far_clip_distance = 50000.f;
      persp_cam->fov_y_deg = 60.f;
      persp_cam->clear_color = true;
      persp_cam->clear_color_value = gfx_color::colors::black;
      persp_cam->clear_depth = true;

      u_v3_light_dir = -glm::vec3(-1.f, 1.f, 1.f);

      {
         billboard = std::make_shared<gfx_billboard>();
         gfx_billboard& r_billboard = *billboard;
         r_billboard[MP_SHADER_NAME] = gfx::c_o_sh_id;
         r_billboard[MP_CULL_BACK] = false;
         r_billboard[MP_CULL_FRONT] = false;
         r_billboard[MP_BLENDING] = MV_NONE;
         r_billboard["u_v4_color"] = gfx_color::colors::white.to_vec4();
         //r_billboard[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
         r_billboard.set_dimensions(500, 500);
         r_billboard.position = glm::vec3(-500.f);
      }

      auto scene = mws_mod->gfx_scene_inst;
      scene->attach(persp_cam);
      scene->attach(billboard);

      persp_cam->position = glm::vec3(0.f, 0.f, sphere_radius + 15.f);

      mws_report_gfx_errs();
   }

   virtual void receive(mws_sp<mws_dp> idp) override
   {
      if (!idp->is_processed())
      {
         if (idp->is_type(mws_ptr_evt::ptr_evt_type))
         {
            mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(idp);
            bool is_dragging = dragging_dt.detect_helper(ts);

            switch (ts->press_type)
            {
            case mws_ptr_evt::e_not_pressed:
               message = "not pressed";
               break;
            case mws_ptr_evt::e_touch_pressed:
               message = "touch pressed";
               break;
            case mws_ptr_evt::e_left_mouse_btn:
               message = "left mouse button pressed";
               break;
            case mws_ptr_evt::e_middle_mouse_btn:
               message = "middle mouse button pressed";
               break;
            case mws_ptr_evt::e_right_mouse_btn:
               message = "right mouse button pressed";
               break;
            }

            if (ts->type == mws_ptr_evt::touch_ended)
            {
               message = "released";
            }

            //mws_print("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
            if (is_dragging)
            {
               float dx_rad = glm::radians(dragging_dt.drag_diff.x / 2);
               float dy_rad = glm::radians(dragging_dt.drag_diff.y / 2);

               glm::vec3 right_dir = glm::cross(look_at_dir, up_dir);
               glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
               look_at_dir = glm::normalize(look_at_dir * rot_around_right_dir);
               up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));

               glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, up_dir);
               look_at_dir = glm::normalize(look_at_dir * rot_around_up_dir);
               cam_ctrl = true;
               process(ts);
            }
            else if (ts->type == mws_ptr_evt::touch_ended)
            {
               cam_ctrl = false;
               process(ts);
            }
            else if (ts->type == mws_ptr_evt::mouse_wheel)
            {
               message = "mouse wheel " + mws_to_str_fmt("[ %2.3f ] received", ts->mouse_wheel_delta);
               persp_cam->position += look_at_dir * 150.f * ts->mouse_wheel_delta;
               process(ts);
            }
         }
         else if (idp->is_type(mws_key_evt::key_evt_type))
         {
            mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

            if (ke->get_type() != mws_key_evt::ke_released)
            {
               bool do_action = true;

               switch (ke->get_key())
               {
               case mws_key_q:
               {
                  persp_cam->position -= look_at_dir * 0.05f;
                  break;
               }

               case mws_key_e:
               {
                  persp_cam->position += look_at_dir * 0.05f;
                  break;
               }

               case mws_key_a:
               {
                  glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), look_at_dir);
                  up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
                  break;
               }

               case mws_key_d:
               {
                  glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), look_at_dir);
                  up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
                  break;
               }

               case mws_key_z:
               {
                  speed -= 0.05f;
                  break;
               }

               case mws_key_c:
               {
                  speed += 0.05f;
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
      }
   }

   virtual void update_state() override
   {
      u_v3_light_dir = -glm::vec3(-1.f, -0.5f, 0.5f);

      persp_cam->draw_axes(glm::vec3(0.f), 1000, 0.25f);
      persp_cam->position += look_at_dir * speed;
      persp_cam->look_at(look_at_dir, up_dir);

      mws_report_gfx_errs();
   }

   virtual void update_view(mws_sp<mws_camera> g) override
   {
      g->drawText(message, 20.f, 20.f, remaining_fnt);
   }

   std::string message;
   mws_sp<gfx_billboard> billboard;
   mws_sp<gfx_camera> persp_cam;
   glm::vec3 u_v3_light_dir;
   float t = 0.f;
   float t2 = 0.f;
   float phi = 0.f;
   float theta = 0.f;
   glm::vec3 look_at_dir = glm::vec3(0.f, 0.f, -1.f);
   glm::vec3 up_dir = glm::vec3(0.f, 1.f, 0.f);
   float speed = 0.f;
   float sphere_radius = 1500;
   bool cam_ctrl = false;
   dragging_detector dragging_dt;
   mws_sp<mws_font> remaining_fnt;
};

mod_mouse_input::mod_mouse_input() : mws_mod(mws_stringify(MOD_mouse_input)) {}

mws_sp<mod_mouse_input> mod_mouse_input::nwi()
{
   return mws_sp<mod_mouse_input>(new mod_mouse_input());
}

void mod_mouse_input::init_mws()
{
   mws_root->new_page<main_page>();
}
