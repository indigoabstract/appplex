#include "stdafx.hxx"

#include "gesture-detectors.hxx"
#include "input-ctrl.hxx"


// since it can't be null, use a dummy event used as default pointer event for gesture detectors
static mws_sp<mws_ptr_evt> dummy_event = mws_ptr_evt::nwi();


// dragging_detector
dragging_detector::dragging_detector()
{
   reset();
}

bool dragging_detector::detect_helper(mws_sp<mws_ptr_evt> evt)
{
   bool gesture_detected = false;
   auto dragging_state = detect(evt);

   drag_diff = glm::vec2(0.f);

   switch (dragging_state)
   {
   case GS_MOVE:
   {
      last_move_pos_bak = pointer_pos;
      pointer_pos = mws_ptr_evt::get_pos(*evt->get_pointer_press_by_index(0));
      drag_diff = pointer_pos - last_move_pos;
      last_move_pos = pointer_pos;
      last_move_pos_time = evt->time;
      gesture_detected = true;
      break;
   }

   case GS_END:
   {
      pointer_pos = mws_ptr_evt::get_pos(*evt->get_pointer_press_by_index(0));
      drag_diff = pointer_pos - last_move_pos;
      gesture_detected = true;
      break;
   }
   }

   return gesture_detected;
}

bool dragging_detector::is_finished() const
{
   return det_state == detector_state::ST_READY;
}

gesture_state dragging_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // only one finger press is allowed for dragging
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   //mws_print("dragging_detector new_event->type [%s] [%d]\n", new_event->get_evt_type().c_str(), mws::time::get_time_millis());
   switch (new_event->type)
   {
   case mws_ptr_evt::touch_began:
      start_event = new_event;
      last_move_pos_bak = last_move_pos = press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
      set_state(detector_state::ST_PRESSED);

      return GS_START;

   case mws_ptr_evt::touch_ended:
      if (det_state == detector_state::ST_MOVING)
      {
         reset();

         return GS_END;
      }

      return reset();

   case mws_ptr_evt::touch_moved:
      switch (det_state)
      {
      case detector_state::ST_PRESSED:
         set_state(detector_state::ST_MOVING);
      case detector_state::ST_MOVING:
         return GS_MOVE;
      }

      return reset();
   }

   return reset();
}

gesture_state dragging_detector::reset()
{
   start_event = dummy_event;
   set_state(detector_state::ST_READY);

   return GS_NONE;
}

void dragging_detector::set_state(detector_state i_st)
{
   //mws_print("dragging_detector old state [%s] new state [%s] [%d]\n", to_string(det_state).c_str(), to_string(i_st).c_str(), mws::time::get_time_millis());
   det_state = i_st;
}

std::string dragging_detector::to_string(detector_state i_st) const
{
   switch (i_st)
   {
   case detector_state::ST_READY:
      return "ST_READY";
   case detector_state::ST_PRESSED:
      return "ST_PRESSED";
   case detector_state::ST_MOVING:
      return "ST_MOVING";
   }

   return "n/a";
}


// double_tap_detector
double_tap_detector::double_tap_detector()
{
   reset();
}

bool double_tap_detector::detect_helper(mws_sp<mws_ptr_evt> evt)
{
   gesture_state gs = detect(evt);
   return gs == GS_ACTION;
}

gesture_state double_tap_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // only one finger press is allowed for taps
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   if (det_state > detector_state::ST_READY)
   {
      auto crt_time = mws::time::get_time_millis();
      auto delta = crt_time - start_event->time;

      // check for max double tap duration
      if (delta > max_duration)
      {
         reset();

         // if the new event is a press, restart the detector and continue
         // otherwise, abort detection
         if (new_event->type != mws_ptr_evt::touch_began)
         {
            return GS_NONE;
         }
      }
   }

   switch (new_event->type)
   {
   case mws_ptr_evt::touch_began:
      if (det_state == detector_state::ST_READY)
      {
         start_event = new_event;
         first_press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
         set_state(detector_state::ST_PRESSED_0);

         return GS_START;
      }
      else if (det_state == detector_state::ST_RELEASED_0)
      {
         second_press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
         set_state(detector_state::ST_PRESSED_1);

         if (!leq_max_dist(first_press_pos, second_press_pos))
         {
            return reset();
         }

         return GS_START;
      }

      return reset();

   case mws_ptr_evt::touch_ended:
      if (det_state == detector_state::ST_PRESSED_0)
      {
         set_state(detector_state::ST_RELEASED_0);

         return GS_START;
      }
      else if (det_state == detector_state::ST_PRESSED_1)
      {
         auto release = new_event->get_pointer_press_by_index(0);

         if (!leq_max_dist(mws_ptr_evt::get_pos(*release), first_press_pos))
         {
            return reset();
         }

         reset();

         return GS_ACTION;
      }

      return reset();

   case mws_ptr_evt::touch_moved:
      if (det_state > detector_state::ST_READY)
      {
         auto press = new_event->get_pointer_press_by_index(0);

         if (!leq_max_dist(mws_ptr_evt::get_pos(*press), first_press_pos))
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
   set_state(detector_state::ST_READY);

   return GS_NONE;
}

uint32_t double_tap_detector::get_double_tap_max_duration() const { return max_duration; }

void double_tap_detector::set_double_tap_max_duration(uint32_t i_max_duration) { max_duration = i_max_duration; }

mws_cm double_tap_detector::get_double_tap_max_pointer_distance() const { return max_distance; }

void double_tap_detector::set_double_tap_max_pointer_distance(mws_cm i_max_distance) { max_distance = i_max_distance; }

void double_tap_detector::set_state(detector_state i_st)
{
   det_state = i_st;
}

bool double_tap_detector::leq_max_dist(const glm::vec2& i_first_press, const glm::vec2& i_second_press)
{
   if (dist_eval)
   {
      if (!dist_eval(i_first_press, i_second_press))
      {
         return false;
      }
   }
   else
   {
      if (glm::distance(i_first_press, i_second_press) > max_distance.to_px().val())
      {
         return false;
      }
   }

   return true;
}


// triple_tap_detector
triple_tap_detector::triple_tap_detector()
{
   reset();
}

bool triple_tap_detector::detect_helper(mws_sp<mws_ptr_evt> evt)
{
   gesture_state gs = detect(evt);
   return gs == GS_ACTION;
}

gesture_state triple_tap_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // only one finger press is allowed for taps
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   if (det_state > detector_state::ST_READY)
   {
      auto crt_time = mws::time::get_time_millis();
      auto delta = crt_time - start_event->time;

      // check for max double tap duration
      if (delta > max_duration)
      {
         reset();

         // if the new event is a press, restart the detector and continue
         // otherwise, abort detection
         if (new_event->type != mws_ptr_evt::touch_began)
         {
            return GS_NONE;
         }
      }
   }

   switch (new_event->type)
   {
   case mws_ptr_evt::touch_began:
      if (det_state == detector_state::ST_READY)
      {
         start_event = new_event;
         first_press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
         set_state(detector_state::ST_PRESSED_0);

         return GS_START;
      }
      else if (det_state == detector_state::ST_RELEASED_0)
      {
         second_press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
         set_state(detector_state::ST_PRESSED_1);

         if (glm::distance(second_press_pos, first_press_pos) > max_distance.to_px().val())
         {
            return reset();
         }

         return GS_MOVE;
      }
      else if (det_state == detector_state::ST_RELEASED_1)
      {
         third_press_pos = mws_ptr_evt::get_pos(*new_event->get_pointer_press_by_index(0));
         set_state(detector_state::ST_PRESSED_2);

         if (glm::distance(third_press_pos, first_press_pos) > max_distance.to_px().val())
         {
            return reset();
         }

         return GS_MOVE;
      }

      return reset();

   case mws_ptr_evt::touch_ended:
   {
      if (det_state == detector_state::ST_PRESSED_0)
      {
         set_state(detector_state::ST_RELEASED_0);

         return GS_START;
      }
      else if (det_state == detector_state::ST_PRESSED_1)
      {
         auto release = new_event->get_pointer_press_by_index(0);

         if (glm::distance(mws_ptr_evt::get_pos(*release), first_press_pos) > max_distance.to_px().val())
         {
            return reset();
         }

         set_state(detector_state::ST_RELEASED_1);

         return GS_MOVE;
      }
      else if (det_state == detector_state::ST_PRESSED_2)
      {
         auto release = new_event->get_pointer_press_by_index(0);

         if (glm::distance(mws_ptr_evt::get_pos(*release), first_press_pos) > max_distance.to_px().val())
         {
            return reset();
         }

         reset();

         return GS_ACTION;
      }

      return reset();
   }

   case mws_ptr_evt::touch_moved:
      if (det_state > detector_state::ST_READY)
      {
         auto press = new_event->get_pointer_press_by_index(0);

         if (glm::distance(mws_ptr_evt::get_pos(*press), first_press_pos) > max_distance.to_px().val())
         {
            return reset();
         }

         return GS_MOVE;
      }
   }

   return reset();
}

gesture_state triple_tap_detector::reset()
{
   start_event = dummy_event;
   set_state(detector_state::ST_READY);

   return GS_NONE;
}

void triple_tap_detector::set_state(detector_state i_st)
{
   det_state = i_st;
}


// pinch_zoom_detector
pinch_zoom_detector::pinch_zoom_detector()
{
   start_event = dummy_event;
}

bool pinch_zoom_detector::detect_helper(mws_sp<mws_ptr_evt> evt)
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

gesture_state pinch_zoom_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == mws_ptr_evt::touch_cancelled)
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
         if (new_event->type == mws_ptr_evt::touch_moved)
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
               position_0 = mws_ptr_evt::get_pos(*p0);
               position_1 = mws_ptr_evt::get_pos(*p1);

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

   if (new_event->type == mws_ptr_evt::touch_ended)
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


// anchor_rotation_one_finger_detector
anchor_rotation_one_finger_detector::anchor_rotation_one_finger_detector()
{
   start_event = dummy_event;
}

gesture_state anchor_rotation_one_finger_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // this is a one finger gesture
   if (new_event->touch_count != 1)
   {
      return reset();
   }

   // check for cancelled event
   if (new_event->type == mws_ptr_evt::touch_cancelled)
   {
      return reset();
   }

   // check for gesture start, move and end
   if (new_event->type == mws_ptr_evt::touch_began)
   {
      start_event = new_event;
      start_position = mws_ptr_evt::get_pos(new_event->points[0]);
      prev_position = position = start_position;
      start_time = mws::time::get_time_millis();

      return GS_START;
   }
   else if (new_event->type == mws_ptr_evt::touch_moved)
   {
      // cancel if start event is not valid
      if (start_event->type == mws_ptr_evt::touch_invalid)
      {
         return GS_NONE;
      }

      prev_position = position;
      position = mws_ptr_evt::get_pos(new_event->points[0]);

      return GS_MOVE;
   }
   else if (new_event->type == mws_ptr_evt::touch_ended)
   {
      if (start_event->type == mws_ptr_evt::touch_invalid)
      {
         return GS_NONE;
      }

      prev_position = position;
      position = mws_ptr_evt::get_pos(start_event->points[0]);
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


// axis_roll_detector
axis_roll_detector::axis_roll_detector()
{
   reset();
}

bool axis_roll_detector::detect_helper(mws_sp<mws_ptr_evt> evt, float& rotation_angle)
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

gesture_state axis_roll_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == mws_ptr_evt::touch_cancelled)
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
         if (new_event->type == mws_ptr_evt::touch_moved)
         {
            auto p0 = new_event->find_point(start_event->points[0].identifier);
            auto p1 = new_event->find_point(start_event->points[1].identifier);

            if (p0 && p1)
            {
               position_0 = mws_ptr_evt::get_pos(*p0);
               position_1 = mws_ptr_evt::get_pos(*p1);
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

   if (new_event->type == mws_ptr_evt::touch_ended)
   {
      if (new_event->touch_count == 2 && det_state == GS_MOVE)
      {
         auto p0 = new_event->find_point(start_event->points[0].identifier);
         auto p1 = new_event->find_point(start_event->points[1].identifier);

         if (p0 && p1)
         {
            position_0 = mws_ptr_evt::get_pos(*p0);
            position_1 = mws_ptr_evt::get_pos(*p1);
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


// panning_tilting_detector
panning_tilting_detector::panning_tilting_detector()
{
   start_event = dummy_event;
}

gesture_state panning_tilting_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // check for cancelled event
   if (new_event->type == mws_ptr_evt::touch_cancelled)
   {
      return reset();
   }

   // need 3 touches
   if ((new_event->type != mws_ptr_evt::touch_ended) && (new_event->touch_count != 3))
   {
      return reset();
   }

   // check if touch identifiers are unchanged (number of touches and same touch ids)
   if ((start_event->type != mws_ptr_evt::touch_invalid) && !start_event->same_touches(*new_event))
   {
      return reset();
   }

   // check for gesture start, move and end
   if (new_event->type == mws_ptr_evt::touch_began)
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
   else if (new_event->type == mws_ptr_evt::touch_moved)
   {
      // cancel if start event is not valid
      if (start_event->type == mws_ptr_evt::touch_invalid || new_event->touch_count < 3)
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
   else if (new_event->type == mws_ptr_evt::touch_ended)
   {
      if (start_event->type == mws_ptr_evt::touch_invalid || new_event->touch_count < 3)
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


// swipe_detector
swipe_detector::swipe_detector()
{
   reset();
}

bool swipe_detector::detect_helper(mws_sp<mws_ptr_evt> evt)
{
   auto state = detect(evt);

   if (state == GS_END)
   {
      pointer_pos = mws_ptr_evt::get_pos(*evt->get_pointer_press_by_index(0));

      return true;
   }

   return false;
}

bool swipe_detector::was_started_from_edge() const
{
   bool started_from_edge = false;
   float dist = 0.f;
   float horiz_threshold = mws::screen::get_width() * 0.02f;
   float vert_threshold = mws::screen::get_height() * 0.02f;

   switch (swipe_direction)
   {
   case swipe_types::left:
      dist = mws::screen::get_width() - press_pos.x;
      started_from_edge = (dist < horiz_threshold);
      break;

   case swipe_types::right:
      dist = press_pos.x;
      started_from_edge = (dist < horiz_threshold);
      break;

   case swipe_types::up:
      dist = press_pos.y;
      started_from_edge = (dist < vert_threshold);
      break;

   case swipe_types::down:
      dist = mws::screen::get_height() - press_pos.y;
      started_from_edge = (dist < vert_threshold);
      break;
   }

   return started_from_edge;
}

gesture_state swipe_detector::detect(const mws_sp<mws_ptr_evt> new_event)
{
   // only one finger press is allowed for swiping
   if (new_event->pointer_down_count() != 1)
   {
      return reset();
   }

   //mws_print("dragging_detector new_event->type [%s] [%d]\n", new_event->get_evt_type().c_str(), mws::time::get_time_millis());
   switch (new_event->type)
   {
   case mws_ptr_evt::touch_began:
      press_pos = mws_ptr_evt::get_pos(new_event->points[0]);
      start_event = new_event;
      touch_vect.clear();
      touch_vect.push_back(new_event);
      start_time = new_event->time;
      set_state(detector_state::ST_PRESSED);

      return GS_START;

   case mws_ptr_evt::touch_ended:
      if (det_state == detector_state::ST_MOVING)
      {
         touch_vect.push_back(new_event);

         if (is_valid_swipe())
         {
            reset();
            return GS_END;
         }
      }

      return reset();

   case mws_ptr_evt::touch_moved:
      switch (det_state)
      {
      case detector_state::ST_PRESSED:
         touch_vect.push_back(new_event);
         set_state(detector_state::ST_MOVING);
         return GS_MOVE;

      case detector_state::ST_MOVING:
         touch_vect.push_back(new_event);

         if (trigger_before_touch_ended && is_valid_swipe())
         {
            reset();
            return GS_END;
         }

         return GS_MOVE;
      }

      return reset();
   }

   return reset();
}

gesture_state swipe_detector::reset()
{
   touch_vect.clear();
   start_event = dummy_event;
   set_state(detector_state::ST_READY);

   return GS_NONE;
}

void swipe_detector::set_state(detector_state i_st)
{
   //mws_print("dragging_detector old state [%s] new state [%s] [%d]\n", to_string(det_state).c_str(), to_string(i_st).c_str(), mws::time::get_time_millis());
   det_state = i_st;
}

std::string swipe_detector::to_string(detector_state i_st) const
{
   switch (i_st)
   {
   case detector_state::ST_READY:
      return "ST_READY";
   case detector_state::ST_PRESSED:
      return "ST_PRESSED";
   case detector_state::ST_MOVING:
      return "ST_MOVING";
   }

   return "n/a";
}

// https://github.com/prime31/TouchKit/blob/master/Assets/TouchKit/Recognizers/TKSwipeRecognizer.cs
bool swipe_detector::is_valid_swipe()
{
   float screen_pixels_per_cm = mws::screen::get_avg_dpcm();
   // if we have a time stipulation and we exceeded it stop listening for swipes, fail
   if (max_swipe_duration > 0 && (mws::time::get_time_millis() - start_time) > max_swipe_duration)
   {
      return false;
   }

   // if we don't have at least two points to test yet, then fail
   if (touch_vect.size() < 2)
   {
      return false;
   }

   glm::vec2 start_point = mws_ptr_evt::get_pos(touch_vect.front()->points[0]);
   glm::vec2 end_point = mws_ptr_evt::get_pos(touch_vect.back()->points[0]);

   // the ideal distance in pixels from the start to the finish
   float ideal_dist_px = glm::distance(start_point, end_point);
   float real_dist_px = 0.f;

   // the ideal distance in centimeters, based on the screen pixel density
   float ideal_dist_cm = ideal_dist_px / screen_pixels_per_cm;

   // if the distance moved in cm was less than the minimum,
   if (ideal_dist_cm < min_swipe_dist_cm || ideal_dist_cm > max_swipe_dist_cm)
   {
      return false;
   }

   // add up distances between all points sampled during the gesture to get the real distance
   for (uint32_t i = 1; i < touch_vect.size(); i++)
   {
      glm::vec2 p0 = mws_ptr_evt::get_pos(touch_vect[i - 1]->points[0]);
      glm::vec2 p1 = mws_ptr_evt::get_pos(touch_vect[i]->points[0]);
      real_dist_px += glm::distance(p0, p1);
   }

   // if the real distance is 10% greater than the ideal distance, then fail
   // this weeds out really irregular "lines" and curves from being considered swipes
   if (real_dist_px > ideal_dist_px * 1.1f)
   {
      return false;
   }

   // the speed in cm/s of the swipe
   swipe_speed_cms = ideal_dist_cm / ((mws::time::get_time_millis() - start_time) / 1000.f);

   // turn the slope of the ideal swipe line into an angle in degrees
   glm::vec2 v2 = glm::normalize(end_point - start_point);
   float swipe_angle_deg = glm::atan(v2.y, v2.x);
   swipe_angle_deg = glm::degrees(swipe_angle_deg);

   if (swipe_angle_deg < 0)
   {
      swipe_angle_deg = 360 + swipe_angle_deg;
   }

   swipe_angle_deg = 360 - swipe_angle_deg;

   // depending on the angle of the line, give a logical swipe direction
   if (swipe_angle_deg >= 292.5f && swipe_angle_deg <= 337.5f)
   {
      swipe_direction = swipe_types::up_right;
   }
   else if (swipe_angle_deg >= 247.5f && swipe_angle_deg <= 292.5f)
   {
      swipe_direction = swipe_types::up;
   }
   else if (swipe_angle_deg >= 202.5f && swipe_angle_deg <= 247.5f)
   {
      swipe_direction = swipe_types::up_left;
   }
   else if (swipe_angle_deg >= 157.5f && swipe_angle_deg <= 202.5f)
   {
      swipe_direction = swipe_types::left;
   }
   else if (swipe_angle_deg >= 112.5f && swipe_angle_deg <= 157.5f)
   {
      swipe_direction = swipe_types::down_left;
   }
   else if (swipe_angle_deg >= 67.5f && swipe_angle_deg <= 112.5f)
   {
      swipe_direction = swipe_types::down;
   }
   else if (swipe_angle_deg >= 22.5f && swipe_angle_deg <= 67.5f)
   {
      swipe_direction = swipe_types::down_right;
   }
   // swipe_angle_deg >= 337.5f || swipe_angle_deg <= 22.5f
   else
   {
      swipe_direction = swipe_types::right;
   }

   return true;
}
