#pragma once

#include "pfm.hpp"
#include "input-ctrl.hpp"


// gesture action codes
enum gesture_state
{
   // no gesture was detected
   GS_NONE = 0,
   // the start of a gesture was detected. waiting for next events to confirm
   GS_START = (1 << 0),
   // gesture was detected and is ongoing
   GS_MOVE = (1 << 1),
   // gesture ended on a touch release
   GS_END = (1 << 2),
   // gesture was detected and has finished (ex. a double click or a finger swipe)
   GS_ACTION = GS_START | GS_END,
};


// max duration of a double tap. if more that this time has passed between taps, it's not considered a double tap anymore
const unsigned long DOUBLE_TAP_MAX_DURATION = 500;
// max distance between first press position an subsequent pointer positions
const unsigned long DOUBLE_TAP_MAX_POINTER_DISTANCE = 25;
// max distance between first press position an subsequent pointer positions for rotating with touch gestures
const unsigned long ROTATION_MAX_POINTER_DEVIATION = 15;
// since it can't be null, use a dummy event used as default pointer event for gesture detectors
static auto dummy_event = std::make_shared<pointer_evt>();


class dragging_detector
{
public:
   dragging_detector()
   {
      reset();
   }

   // Operations

   // detect_helper
   // feed new touch event and return detected state
   // params:
   //    evt... feed new event for gesture detection
   //    dragging_delta... calculate the diff from last pointer position
   // return:
   //    returns true if pinch zoom is detected
   bool detect_helper(std::shared_ptr<pointer_evt> evt, glm::vec2& dragging_delta)
   {
      bool gesture_detected = false;
      auto dragging_state = detect(evt);

      dragging_delta = glm::vec2(0.f);

      switch (dragging_state)
      {
      case GS_MOVE:
      {
         last_move_pos_bak = pointer_pos;
         pointer_pos = evt->get_pointer_press_by_index(0)->get_position();
         dragging_delta = pointer_pos - last_move_pos;
         last_move_pos = pointer_pos;
         gesture_detected = true;
         break;
      }

      case GS_END:
      {
         pointer_pos = evt->get_pointer_press_by_index(0)->get_position();
         dragging_delta = pointer_pos - last_move_pos;
         gesture_detected = true;
         break;
      }
      }

      return gesture_detected;
   }

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_MOVE if dragging is detected, GS_START/GS_END/GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
   {
      // only one finger press is allowed for dragging
      if (new_event->pointer_down_count() != 1)
      {
         return reset();
      }

      switch (new_event->type)
      {
      case pointer_evt::touch_began:
         if (det_state == tap_detector_state::ST_READY)
         {
            start_event = new_event;
            last_move_pos_bak = last_move_pos = press_pos = new_event->get_pointer_press_by_index(0)->get_position();
            det_state = tap_detector_state::ST_PRESSED;

            return GS_START;
         }

         return reset();

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
            det_state = tap_detector_state::ST_MOVING;
         case tap_detector_state::ST_MOVING:
            return GS_MOVE;
         }

         return reset();
      }

      return reset();
   }

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;
      det_state = tap_detector_state::ST_READY;

      return GS_NONE;
   }

   // Access

   // Inquiry

//private:
   enum class tap_detector_state
   {
      ST_READY,
      ST_PRESSED,
      ST_MOVING,
   };

   // Member Variables

   glm::vec2 press_pos;
   glm::vec2 pointer_pos;
   glm::vec2 last_move_pos;
   glm::vec2 last_move_pos_bak;
   std::shared_ptr<pointer_evt> start_event;
   tap_detector_state det_state;
};


class double_tap_detector
{
public:
   // Constructor
   // params:
   double_tap_detector()
   {
      reset();
   }

   // Operations

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_ACTION if double tap detected, GS_START or GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
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
            det_state = tap_detector_state::ST_PRESSED_0;

            return GS_START;
         }
         else if (det_state == tap_detector_state::ST_RELEASED_0)
         {
            second_press_pos = new_event->get_pointer_press_by_index(0)->get_position();
            det_state = tap_detector_state::ST_PRESSED_1;

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
            det_state = tap_detector_state::ST_RELEASED_0;

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

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;
      det_state = tap_detector_state::ST_READY;

      return GS_NONE;
   }

   // Access

   // Inquiry 

private:
   enum class tap_detector_state
   {
      ST_READY,
      ST_PRESSED_0,
      ST_RELEASED_0,
      ST_PRESSED_1,
   };

   // Member Variables

   // get start position of tap
   glm::vec2 first_press_pos;
   glm::vec2 second_press_pos;

   std::shared_ptr<pointer_evt> start_event;
   tap_detector_state det_state;
};


class pinch_zoom_detector
{
public:
   // Constructor
   // params:
   pinch_zoom_detector()
   {
      start_event = dummy_event;
   }

   // Operations

   // detect_helper
   // feed new touch event and return detected state
   // params:
   //    evt... feed new event for gesture detection
   //    zoom_factor... calculate the factor for zooming (+ zooms in, - zooms out)
   // return:
   //    returns true if pinch zoom is detected
   bool detect_helper(std::shared_ptr<pointer_evt> evt, float& zoom_factor)
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

      //vprint("zf %f tc: %d\n", zoom_factor, evt->touch_count);
      return gesture_detected;
   }

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_MOVE if pinch is detected, GS_START/GS_END on start end, or GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
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

                  //vprint("dist: %f %f\n", dist_0, dist_1);
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

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;
      zoom_factor = last_dist = 0.f;
      det_state = GS_NONE;

      return GS_NONE;
   }

   // Access

   // Inquiry

   // Member Variables

   // position of first touch
   glm::vec2 position_0;
   // position of second touch
   glm::vec2 position_1;
   // prev position of first touch
   glm::vec2 prev_position_0;
   // prev position of second touch
   glm::vec2 prev_position_1;
   // start position of first touch
   glm::vec2 start_position_0;
   // start position of second touch
   glm::vec2 start_position_1;
   float zoom_factor;
   float last_dist;

   std::shared_ptr<pointer_evt> start_event;
   gesture_state det_state;
};


class anchor_rotation_one_finger_detector
{
public:
   // Constructor
   // params:
   anchor_rotation_one_finger_detector()
   {
      start_event = dummy_event;
   }

   // Operations

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_MOVE if axis rolling is detected, GS_START/GS_END on start end, or GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
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
         prev_position = mPosition = start_position;
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

         prev_position = mPosition;
         mPosition = new_event->points[0].get_position();

         return GS_MOVE;
      }
      else if (new_event->type == pointer_evt::touch_ended)
      {
         if (start_event->type == pointer_evt::touch_invalid)
         {
            return GS_NONE;
         }

         prev_position = mPosition;
         mPosition = start_event->points[0].get_position();
         reset();

         return GS_END;
      }

      return GS_NONE;
   }

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;

      return GS_NONE;
   }

   // Access

   // Inquiry 

   // Member Variables

   // position of finger
   glm::vec2 mPosition;
   // prev position of finger
   glm::vec2 prev_position;
   // start position of finger press
   glm::vec2 start_position;
   unsigned long start_time;

   std::shared_ptr<pointer_evt> start_event;
};


class axis_roll_detector
{
public:
   // Constructor
   // params:
   axis_roll_detector()
   {
      reset();
   }

   // Operations

   // detect_helper
   // feed new touch event and return detected state
   // params:
   //    evt... feed new event for gesture detection
   //    zoom_factor... calculate the factor for zooming (+ zooms in, - zooms out)
   // return:
   //    returns true if pinch zoom is detected
   bool detect_helper(std::shared_ptr<pointer_evt> evt, float& rotation_angle)
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
            //vprint("ca: %f\n", cos_alpha);
            gesture_detected = true;
         }
         break;
      }
      }

      //vprint("ps: %d\n", pinch_state);
      //vprint("zf %f tc: %d\n", zoom_factor, evt->touch_count);
      return gesture_detected;
   }

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_MOVE if axis rolling is detected, GS_START/GS_END on start end, or GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
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

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;
      det_state = GS_NONE;

      return GS_NONE;
   }

   // Access

   // Inquiry

   // Member Variables

   // position of first touch
   glm::vec2 position_0;
   // position of second touch
   glm::vec2 position_1;
   // start position of first touch
   glm::vec2 start_position_0;
   // start position of second touch
   glm::vec2 start_position_1;

   glm::vec2 anchor_touch_position;
   glm::vec2 roll_touch_prev_position;
   glm::vec2 roll_touch_position;

   std::shared_ptr<pointer_evt> start_event;
   gesture_state det_state;
};


class panning_tilting_detector
{
public:
   // Constructor
   // params:
   panning_tilting_detector()
   {
      start_event = dummy_event;
   }

   // Operations

   // detect
   // feed new touch event and return detected state
   // params:
   //    new_event... feed new event for gesture detection
   // return:
   //    returns GS_MOVE if panning/tilting is detected, GS_START/GS_END on start end, or GS_NONE otherwise
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event)
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
         start_position2 = new_event->touch_pos(new_event->points[2].identifier);
         position_0 = start_position_0;
         position_1 = start_position_1;
         prev_position2 = position_2 = start_position2;

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
         prev_position2 = position_2;
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

   // gesture_state
   // reset the detector
   // return:
   //    returns GS_NONE
   gesture_state reset()
   {
      start_event = dummy_event;

      return GS_NONE;
   }

   // Access

   // Inquiry 

   // Member Variables

   // position of first touch
   glm::vec2 position_0;
   // position of second touch
   glm::vec2 position_1;
   // position of third touch
   glm::vec2 position_2;
   // prev position of third touch
   glm::vec2 prev_position2;
   // start position of first touch
   glm::vec2 start_position_0;
   // start position of second touch
   glm::vec2 start_position_1;
   // start position of third touch
   glm::vec2 start_position2;

   std::shared_ptr<pointer_evt> start_event;
};
