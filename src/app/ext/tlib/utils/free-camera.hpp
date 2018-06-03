#pragma once

#include "min.hpp"
#include <glm/glm.hpp>
#include "com/unit/gesture-detectors.hpp"

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

   free_camera(std::shared_ptr<unit> i_u);

	void update_input(shared_ptr<iadp> idp);
	void update();

	std::shared_ptr<gfx_camera> persp_cam;
	glm::vec3 look_at_dir;
	glm::vec3 up_dir;
   glm::vec3 target_ref_point;
	float speed;
	float mw_speed_factor;

private:
   void clamp_angles();

   float inf_phi_deg;
   float sup_phi_deg;
   float phi_deg; // [10, 80]
   float theta_deg; // [0, 360)
   float sign_phi;
   e_movement_types mov_type;
   dragging_detector dragging_det;
   std::shared_ptr<kinetic_scrolling> ks;
   std::weak_ptr<unit> u;
};
