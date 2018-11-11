#include "stdafx.h"

#include "free-camera.hpp"
#include "pfm.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/util/util.hpp"
#include "gfx-camera.hpp"
#include "unit.hpp"


free_camera::free_camera(std::shared_ptr<unit> i_u)
{
   u = i_u;
   look_at_dir = glm::vec3(0.f, 0.f, -1.f);
   up_dir = glm::vec3(0.f, 1.f, 0.f);
   speed = 0.f;
   mw_speed_factor = 25.f;
   inf_phi_deg = 1.f;
   sup_phi_deg = 179.f;
   phi_deg = 90.f;
   theta_deg = 210.f;
   ks = std::make_shared<kinetic_scrolling>();
   mov_type = e_roll_view_axis;
}

void free_camera::update_input(shared_ptr<iadp> idp)
{
   if (idp->is_processed())
   {
      return;
   }

   bool ctrl_held = u.lock()->key_ctrl->key_is_held(KEY_CONTROL);

   if (ctrl_held)
   {
      mov_type = e_translation;
   }
   else
   {
      mov_type = e_roll_view_axis;
   }

   if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);

      bool dragging_detected = dragging_det.detect(ts);

      if (dragging_detected)
      {
         // rotation about the camera's view/target axis.
         if (ctrl_held)
         {
            float dx = ts->points[0].x - dragging_det.last_move_pos.x;// ts->prev_state.te->points[0].x;
            float dy = ts->points[0].y - dragging_det.last_move_pos.y;// ts->prev_state.te->points[0].y;
            float dx_rad = glm::radians(dx / 2);
            float dy_rad = glm::radians(dy / 2);

            glm::vec3 right_dir = glm::cross(look_at_dir, up_dir);
            glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
            look_at_dir = glm::normalize(look_at_dir * rot_around_right_dir);
            up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));

            glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, up_dir);
            look_at_dir = glm::normalize(look_at_dir * rot_around_up_dir);
            ts->process();
         }
         // translation movement.
         else
         {
            float dx = ts->points[0].x - dragging_det.last_move_pos.x;// ts->prev_state.te->points[0].x;
            float dy = ts->points[0].y - dragging_det.last_move_pos.y;// ts->prev_state.te->points[0].y;

            //if (ts->is_finished)
            //{
            //   uint32 delta_t = ts->time - ts->prev_state.te->time;

            //   if (delta_t < 150)
            //   {
            //      ks->start_slowdown();
            //   }
            //   else
            //   {
            //      ks->reset();
            //   }
            //}
            //else
            //{
            //   ks->begin(ts->points[0].x, ts->points[0].y);
            //}

            float camera_radius = glm::distance(persp_cam->position(), target_ref_point);

            theta_deg -= dx * camera_radius * 0.00045f;
            phi_deg -= dy * camera_radius * 0.00045f;
            clamp_angles();
            //mws_print("tdx %f pdx %f\n", theta_deg, phi_deg);
            mov_type = e_roll_view_axis;
            ts->process();
         }
      }

      if (!ts->is_processed())
      {
         //mws_print("tn %s\n", ts->get_type_name(ts->type).c_str());
         switch (ts->type)
         {
         case pointer_evt::touch_began:
         {
            ks->grab(ts->points[0].x, ts->points[0].y);
            ts->process();
            break;
         }

         case pointer_evt::touch_ended:
         {
            ts->process();
            break;
         }

         case pointer_evt::mouse_wheel:
         {
            persp_cam->position += look_at_dir * mw_speed_factor * ts->mouse_wheel_delta;
            ts->process();
            break;
         }
         }
      }
   }
   else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
   {
      shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

      if (!ke->is_released())
      {
         bool isAction = true;

         switch (ke->get_key())
         {
         case KEY_Q:
         {
            persp_cam->position -= look_at_dir * mw_speed_factor * 1.5f;
            break;
         }

         case KEY_E:
         {
            persp_cam->position += look_at_dir * mw_speed_factor * 1.5f;
            break;
         }

         case KEY_A:
         {
            glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), look_at_dir);
            up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
            break;
         }

         case KEY_S:
         {
            speed = 0.f;
            break;
         }

         case KEY_D:
         {
            glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), look_at_dir);
            up_dir = glm::normalize(up_dir * rot_around_look_at_dir);
            break;
         }

         case KEY_Z:
         {
            speed -= 0.05f;
            break;
         }

         case KEY_C:
         {
            speed += 0.05f;
            break;
         }

         default:
            isAction = false;
         }

         //if (!isAction && !ke->is_repeated())
         //{
         //   isAction = true;

         //   switch (ke->get_key())
         //   {
         //   case KEY_SPACE:
         //   case KEY_F1:
         //      break;

         //   default:
         //      isAction = false;
         //   }
         //}

         if (isAction)
         {
            ke->process();
         }
      }
   }

}

void free_camera::update()
{

   point2d scroll_pos = ks->update();

   switch (mov_type)
   {
   case e_translation:
   {
      persp_cam->position += look_at_dir * speed;
      persp_cam->look_at(look_at_dir, up_dir);
      break;
   }

   case e_roll_own_axis:
   {
      break;
   }

   case e_roll_view_axis:
   {
      float camera_radius = glm::distance(persp_cam->position(), target_ref_point);

      //mws_print("td0 %f pd0 %f", theta_deg, phi_deg);
      if (ks->is_active())
      {
         theta_deg -= scroll_pos.x * 0.1f;
         phi_deg -= scroll_pos.y * 0.1f;
         clamp_angles();
      }
      //mws_print("td1 %f pd1 %f\n", theta_deg, phi_deg);

      float sin_phi = glm::sin(glm::radians(phi_deg));
      float cos_phi = glm::cos(glm::radians(phi_deg));
      float sin_theta = glm::sin(glm::radians(theta_deg));
      float cos_theta = glm::cos(glm::radians(theta_deg));
      glm::vec3 sphere_position(camera_radius * sin_theta * sin_phi, camera_radius * cos_phi, camera_radius * cos_theta * sin_phi);

      glm::vec3 view_pos = target_ref_point + sphere_position;
      persp_cam->position = view_pos;
      persp_cam->look_at_pos(target_ref_point, up_dir);
      look_at_dir = glm::normalize(target_ref_point - persp_cam->position());

      up_dir = glm::vec3(0, 1, 0);
      glm::vec3 right_dir = glm::normalize(glm::cross(look_at_dir, up_dir));
      up_dir = glm::normalize(glm::cross(right_dir, look_at_dir));
      persp_cam->look_at_pos(target_ref_point, up_dir);
      //mws_print("td %f\n", theta_deg);
      break;
   }
   }
}

void free_camera::clamp_angles()
{
   if (phi_deg < inf_phi_deg)
   {
      phi_deg = inf_phi_deg;
   }
   else if (phi_deg > sup_phi_deg)
   {
      phi_deg = sup_phi_deg;
   }

   float mod_theta = 360.f;
   theta_deg = glm::mod(theta_deg, mod_theta);
}
