#pragma once

#include "appplex-conf.hpp"

#if defined MOD_GFX

#include "gesture-detectors.hpp"
#include "input-ctrl.hpp"


dragging_detector::dragging_detector()
{
   reset();
}

bool dragging_detector::detect_helper(std::shared_ptr<pointer_evt> evt)
{
   bool gesture_detected = false;
   auto dragging_state = detect(evt);

   drag_diff = glm::vec2(0.f);

   switch (dragging_state)
   {
   case GS_MOVE:
   {
      last_move_pos_bak = pointer_pos;
      pointer_pos = evt->get_pointer_press_by_index(0)->get_position();
      drag_diff = pointer_pos - last_move_pos;
      last_move_pos = pointer_pos;
      last_move_pos_time = evt->time;
      gesture_detected = true;
      break;
   }

   case GS_END:
   {
      pointer_pos = evt->get_pointer_press_by_index(0)->get_position();
      drag_diff = pointer_pos - last_move_pos;
      gesture_detected = true;
      break;
   }
   }

   return gesture_detected;
}

bool dragging_detector::is_finished() const
{
   return det_state == tap_detector_state::ST_READY;
}

gesture_state dragging_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // only one finger press is allowed for dragging
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   //mws_print("dragging_detector new_event->type [%s] [%d]\n", new_event->get_evt_type().c_str(), pfm::time::get_time_millis());
   switch (new_event->type)
   {
   case pointer_evt::touch_began:
      start_event = new_event;
      last_move_pos_bak = last_move_pos = press_pos = new_event->get_pointer_press_by_index(0)->get_position();
      set_state(tap_detector_state::ST_PRESSED);

      return GS_START;

   case pointer_evt::touch_ended:
      if (det_state == tap_detector_state::ST_MOVING)
      {
         reset();

         return GS_END;
      }

      return reset();

   case pointer_evt::touch_moved:
      switch (det_state)
      {
      case tap_detector_state::ST_PRESSED:
         set_state(tap_detector_state::ST_MOVING);
      case tap_detector_state::ST_MOVING:
         return GS_MOVE;
      }

      return reset();
   }

   return reset();
}

gesture_state dragging_detector::reset()
{
   start_event = dummy_event;
   set_state(tap_detector_state::ST_READY);

   return GS_NONE;
}

void dragging_detector::set_state(tap_detector_state i_st)
{
   //mws_print("dragging_detector old state [%s] new state [%s] [%d]\n", to_string(det_state).c_str(), to_string(i_st).c_str(), pfm::time::get_time_millis());
   det_state = i_st;
}

std::string dragging_detector::to_string(tap_detector_state i_st) const
{
   switch (i_st)
   {
   case tap_detector_state::ST_READY:
      return "ST_READY";
   case tap_detector_state::ST_PRESSED:
      return "ST_PRESSED";
   case tap_detector_state::ST_MOVING:
      return "ST_MOVING";
   }

   return "n/a";
}


double_tap_detector::double_tap_detector()
{
   reset();
}

bool double_tap_detector::detect_helper(std::shared_ptr<pointer_evt> evt)
{
   gesture_state gs = detect(evt);
   return gs == GS_ACTION;
}

gesture_state double_tap_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // only one finger press is allowed for taps
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   if (det_state > tap_detector_state::ST_READY)
   {
      auto crt_time = pfm::time::get_time_millis();
      auto delta = crt_time - start_event->time;

      // check for max double tap duration
      if (delta > DOUBLE_TAP_MAX_DURATION)
      {
         reset();

         // if the new event is a press, restart the detector and continue
         // otherwise, abort detection
         if (new_event->type != pointer_evt::touch_began)
         {
            return GS_NONE;
         }
      }
   }

   switch (new_event->type)
   {
   case pointer_evt::touch_began:
      if (det_state == tap_detector_state::ST_READY)
      {
         start_event = new_event;
         first_press_pos = new_event->get_pointer_press_by_index(0)->get_position();
         set_state(tap_detector_state::ST_PRESSED_0);

         return GS_START;
      }
      else if (det_state == tap_detector_state::ST_RELEASED_0)
      {
         second_press_pos = new_event->get_pointer_press_by_index(0)->get_position();
         set_state(tap_detector_state::ST_PRESSED_1);

         if (glm::distance(second_press_pos, first_press_pos) > DOUBLE_TAP_MAX_POINTER_DISTANCE)
         {
            return reset();
         }

         return GS_START;
      }

      return reset();

   case pointer_evt::touch_ended:
      if (det_state == tap_detector_state::ST_PRESSED_0)
      {
         set_state(tap_detector_state::ST_RELEASED_0);

         return GS_START;
      }
      else if (det_state == tap_detector_state::ST_PRESSED_1)
      {
         auto release = new_event->get_pointer_press_by_index(0);

         if (glm::distance(release->get_position(), first_press_pos) > DOUBLE_TAP_MAX_POINTER_DISTANCE)
         {
            return reset();
         }

         reset();

         return GS_ACTION;
      }

      return reset();

   case pointer_evt::touch_moved:
      if (det_state > tap_detector_state::ST_READY)
      {
         auto press = new_event->get_pointer_press_by_index(0);

         if (glm::distance(press->get_position(), first_press_pos) > DOUBLE_TAP_MAX_POINTER_DISTANCE)
         {
            return reset();
         }

         return GS_START;
      }
   }

   return reset();
}

gesture_state double_tap_detector::reset()
{
   start_event = dummy_event;
   set_state(tap_detector_state::ST_READY);

   return GS_NONE;
}

void double_tap_detector::set_state(tap_detector_state i_st)
{
   det_state = i_st;
}


pinch_zoom_detector::pinch_zoom_detector()
{
   start_event = dummy_event;
}

bool pinch_zoom_detector::detect_helper(std::shared_ptr<pointer_evt> evt)
{
   bool gesture_detected = false;
   auto pinch_state = detect(evt);

   zoom_factor = 0.f;

   switch (pinch_state)
   {
   case GS_START:
   {
      last_dist = glm::distance(position_0, position_1);
      break;
   }

   case GS_MOVE:
   case GS_END:
   {
      float dist = glm::distance(position_0, position_1);

      if (pinch_state == GS_MOVE)
      {
         zoom_factor = dist - last_dist;
      }

      last_dist = dist;
      gesture_detected = true;
      break;
   }
   }

   //mws_print("zf %f tc: %d\n", zoom_factor, evt->touch_count);
   return gesture_detected;
}

gesture_state pinch_zoom_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == pointer_evt::touch_cancelled)
   {
      return reset();
   }

   if (new_event->touch_count == 2)
   {
      switch (det_state)
      {
      case GS_NONE:
         start_event = new_event;
         start_position_0 = new_event->touch_pos(new_event->points[0].identifier);
         start_position_1 = new_event->touch_pos(new_event->points[1].identifier);
         prev_position_0 = position_0 = start_position_0;
         prev_position_1 = position_1 = start_position_1;
         det_state = GS_START;

         return GS_START;

      case GS_START:
      case GS_MOVE:
         if (new_event->type == pointer_evt::touch_moved)
         {
            if (prev_position_0 != position_0)
            {
               prev_position_0 = position_0;
            }

            if (prev_position_1 != position_1)
            {
               prev_position_1 = position_1;
            }

            auto p0 = new_event->find_point(start_event->points[0].identifier);
            auto p1 = new_event->find_point(start_event->points[1].identifier);

            if (p0 && p1)
            {
               position_0 = p0->get_position();
               position_1 = p1->get_position();

               float dist_0 = glm::distance(start_position_0, position_0);
               float dist_0_1 = glm::distance(start_position_0, position_1);
               float dist_1 = glm::distance(start_position_1, position_0);
               float dist_1_0 = glm::distance(start_position_1, position_1);

               //mws_print("dist: %f %f\n", dist_0, dist_1);
               if (dist_0 < ROTATION_MAX_POINTER_DEVIATION || dist_0_1 < ROTATION_MAX_POINTER_DEVIATION ||
                  dist_1 < ROTATION_MAX_POINTER_DEVIATION || dist_1_0 < ROTATION_MAX_POINTER_DEVIATION)
               {
                  det_state = GS_START;
               }
               else
               {
                  det_state = GS_MOVE;
               }

               return det_state;
            }

            return reset();
         }
      }
   }

   if (new_event->type == pointer_evt::touch_ended)
   {
      if (new_event->touch_count == 2 && det_state == GS_MOVE)
      {
         position_0 = new_event->touch_pos(start_event->points[0].identifier);
         position_1 = new_event->touch_pos(start_event->points[1].identifier);
         reset();

         return GS_END;
      }
   }

   return reset();
}

gesture_state pinch_zoom_detector::reset()
{
   start_event = dummy_event;
   zoom_factor = last_dist = 0.f;
   det_state = GS_NONE;

   return GS_NONE;
}


anchor_rotation_one_finger_detector::anchor_rotation_one_finger_detector()
{
   start_event = dummy_event;
}

gesture_state anchor_rotation_one_finger_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // this is a one finger gesture
   if (new_event->touch_count != 1)
   {
      return reset();
   }

   // check for cancelled event
   if (new_event->type == pointer_evt::touch_cancelled)
   {
      return reset();
   }

   // check for gesture start, move and end
   if (new_event->type == pointer_evt::touch_began)
   {
      start_event = new_event;
      start_position = new_event->points[0].get_position();
      prev_position = position = start_position;
      start_time = pfm::time::get_time_millis();

      return GS_START;
   }
   else if (new_event->type == pointer_evt::touch_moved)
   {
      // cancel if start event is not valid
      if (start_event->type == pointer_evt::touch_invalid)
      {
         return GS_NONE;
      }

      prev_position = position;
      position = new_event->points[0].get_position();

      return GS_MOVE;
   }
   else if (new_event->type == pointer_evt::touch_ended)
   {
      if (start_event->type == pointer_evt::touch_invalid)
      {
         return GS_NONE;
      }

      prev_position = position;
      position = start_event->points[0].get_position();
      reset();

      return GS_END;
   }

   return GS_NONE;
}

// gesture_state
// reset the detector
// return:
//    returns GS_NONE
gesture_state anchor_rotation_one_finger_detector::reset()
{
   start_event = dummy_event;

   return GS_NONE;
}


axis_roll_detector::axis_roll_detector()
{
   reset();
}

bool axis_roll_detector::detect_helper(std::shared_ptr<pointer_evt> evt, float& rotation_angle)
{
   bool gesture_detected = false;
   auto pinch_state = detect(evt);

   rotation_angle = 0.f;

   switch (pinch_state)
   {
   case GS_START:
   {
      break;
   }

   case GS_MOVE:
   case GS_END:
   {
      float dx = roll_touch_position.x - roll_touch_prev_position.x;
      float dy = roll_touch_position.y - roll_touch_prev_position.y;

      if ((dx != 0 || dy != 0) && anchor_touch_position != roll_touch_prev_position)
      {
         float scale = 0.1f;
         //float dx_rad = glm::radians(dx * scale);
         //float dy_rad = glm::radians(-dy * scale);
         glm::vec3 screen_center = glm::vec3(anchor_touch_position, 0.f);
         glm::vec3 oriented_radius = glm::vec3(roll_touch_prev_position.x, roll_touch_prev_position.y, 0.f) - screen_center;
         glm::vec3 neg_z_axis(0.f, 0.f, -1.f);
         glm::vec3 circle_tangent = glm::cross(neg_z_axis, oriented_radius);
         circle_tangent = glm::normalize(circle_tangent);
         glm::vec3 touch_dir(dx, dy, 0.f);
         touch_dir = glm::normalize(touch_dir);
         float dot_prod = glm::dot(touch_dir, circle_tangent);
         float cos_alpha = dot_prod / (glm::length(touch_dir) * glm::length(circle_tangent)) * scale;
         rotation_angle = -cos_alpha;
         //mws_print("ca: %f\n", cos_alpha);
         gesture_detected = true;
      }
      break;
   }
   }

   //mws_print("ps: %d\n", pinch_state);
   //mws_print("zf %f tc: %d\n", zoom_factor, evt->touch_count);
   return gesture_detected;
}

gesture_state axis_roll_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == pointer_evt::touch_cancelled)
   {
      return reset();
   }

   if (new_event->touch_count == 2)
   {
      switch (det_state)
      {
      case GS_NONE:
         start_event = new_event;
         start_position_0 = new_event->touch_pos(new_event->points[0].identifier);
         start_position_1 = new_event->touch_pos(new_event->points[1].identifier);
         position_0 = start_position_0;
         position_1 = start_position_1;
         anchor_touch_position = position_0;
         roll_touch_position = roll_touch_prev_position = start_position_1;
         det_state = GS_START;

         return GS_START;

      case GS_START:
      case GS_MOVE:
         if (new_event->type == pointer_evt::touch_moved)
         {
            auto p0 = new_event->find_point(start_event->points[0].identifier);
            auto p1 = new_event->find_point(start_event->points[1].identifier);

            if (p0 && p1)
            {
               position_0 = p0->get_position();
               position_1 = p1->get_position();
               det_state = GS_MOVE;

               float dist_0 = glm::distance(start_position_0, position_0);
               float dist_0_1 = glm::distance(start_position_0, position_1);
               float dist_1 = glm::distance(start_position_1, position_0);
               float dist_1_0 = glm::distance(start_position_1, position_1);

               float min_dist = dist_0;
               roll_touch_prev_position = roll_touch_position;
               anchor_touch_position = start_position_0;
               roll_touch_position = position_1;

               if (dist_0_1 < min_dist)
               {
                  min_dist = dist_0_1;
                  anchor_touch_position = start_position_0;
                  roll_touch_position = position_0;
               }

               if (dist_1 < min_dist)
               {
                  min_dist = dist_1;
                  anchor_touch_position = start_position_1;
                  roll_touch_position = position_1;
               }

               if (dist_1_0 < min_dist)
               {
                  min_dist = dist_1_0;
                  anchor_touch_position = start_position_1;
                  roll_touch_position = position_0;
               }

               if (min_dist > ROTATION_MAX_POINTER_DEVIATION)
               {
                  return reset();
               }

               return GS_MOVE;
            }

            return reset();
         }
      }
   }

   if (new_event->type == pointer_evt::touch_ended)
   {
      if (new_event->touch_count == 2 && det_state == GS_MOVE)
      {
         auto p0 = new_event->find_point(start_event->points[0].identifier);
         auto p1 = new_event->find_point(start_event->points[1].identifier);

         if (p0 && p1)
         {
            position_0 = p0->get_position();
            position_1 = p1->get_position();
            reset();

            return GS_END;
         }
      }
   }

   return reset();
}

gesture_state axis_roll_detector::reset()
{
   start_event = dummy_event;
   det_state = GS_NONE;

   return GS_NONE;
}


panning_tilting_detector::panning_tilting_detector()
{
   start_event = dummy_event;
}

gesture_state panning_tilting_detector::detect(const std::shared_ptr<pointer_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == pointer_evt::touch_cancelled)
   {
      return reset();
   }

   // need 3 touches
   if ((new_event->type != pointer_evt::touch_ended) && (new_event->touch_count != 3))
   {
      return reset();
   }

   // check if touch identifiers are unchanged (number of touches and same touch ids)
   if ((start_event->type != pointer_evt::touch_invalid) && !start_event->same_touches(*new_event))
   {
      return reset();
   }

   // check for gesture start, move and end
   if (new_event->type == pointer_evt::touch_began)
   {
      if (new_event->touch_count < 3)
      {
         return GS_NONE;
      }

      start_event = new_event;
      start_position_0 = new_event->touch_pos(new_event->points[0].identifier);
      start_position_1 = new_event->touch_pos(new_event->points[1].identifier);
      start_position_2 = new_event->touch_pos(new_event->points[2].identifier);
      position_0 = start_position_0;
      position_1 = start_position_1;
      prev_position_2 = position_2 = start_position_2;

      return GS_START;
   }
   else if (new_event->type == pointer_evt::touch_moved)
   {
      // cancel if start event is not valid
      if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 3)
      {
         return GS_NONE;
      }

      position_0 = new_event->touch_pos(start_event->points[0].identifier);
      position_1 = new_event->touch_pos(start_event->points[1].identifier);
      prev_position_2 = position_2;
      position_2 = new_event->touch_pos(start_event->points[2].identifier);

      float dist0 = glm::distance(start_position_0, position_0);
      float dist1 = glm::distance(start_position_1, position_1);

      if (dist0 > ROTATION_MAX_POINTER_DEVIATION || dist1 > ROTATION_MAX_POINTER_DEVIATION)
      {
         return GS_NONE;
      }

      return GS_MOVE;
   }
   else if (new_event->type == pointer_evt::touch_ended)
   {
      if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 3)
      {
         return GS_NONE;
      }

      position_0 = new_event->touch_pos(start_event->points[0].identifier);
      position_1 = new_event->touch_pos(start_event->points[1].identifier);
      position_2 = new_event->touch_pos(start_event->points[2].identifier);
      reset();

      return GS_END;
   }

   return GS_NONE;
}

gesture_state panning_tilting_detector::reset()
{
   start_event = dummy_event;

   return GS_NONE;
}

#endif
