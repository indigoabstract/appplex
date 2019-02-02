#pragma once

#include "min.hxx"
#include <glm/inc.hpp>
#include "com/unit/gesture-detectors.hxx"

class gfx_camera;
class unit;
class kinetic_scrolling;


class free_camera
{
public:
   enum e_movement_types
   {
      e_translation,
      e_roll_own_axis,
      e_roll_view_axis,
   };

   free_camera(mws_sp<unit> i_u);

	void update_input(mws_sp<mws_dp> idp);
	void update();

	mws_sp<gfx_camera> persp_cam;
	glm::vec3 look_at_dir = glm::vec3(0.f);
	glm::vec3 up_dir = glm::vec3(0.f);
   glm::vec3 target_ref_point = glm::vec3(0.f);
	float speed = 0.f;
	float mw_speed_factor = 0.f;

private:
   void clamp_angles();

   float inf_phi_deg = 0.f;
   float sup_phi_deg = 0.f;
   float phi_deg = 0.f; // [10, 80]
   float theta_deg = 0.f; // [0, 360)
   float sign_phi = 0.f;
   e_movement_types mov_type = e_movement_types::e_translation;
   dragging_detector dragging_det;
   mws_sp<kinetic_scrolling> ks;
   mws_wp<unit> u;
};
