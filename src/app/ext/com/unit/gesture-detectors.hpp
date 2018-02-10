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

      // need 2 touches
      if ((new_event->type != pointer_evt::touch_ended) && (new_event->touch_count != 2))
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
         if (new_event->touch_count < 2)
         {
            return GS_NONE;
         }

         start_event = new_event;
         start_position_0 = new_event->touch_pos(new_event->points[0].identifier);
         start_position_1 = new_event->touch_pos(new_event->points[1].identifier);
         prev_position_0 = position_0 = start_position_0;
         prev_position_1 = position_1 = start_position_1;

         return GS_START;
      }
      else if (new_event->type == pointer_evt::touch_moved)
      {
         // cancel if start event is not valid
         if (start_event->type == pointer_evt::touch_invalid || start_event->touch_count < 2)
         {
            return GS_NONE;
         }

         if (prev_position_0 != position_0)
         {
            prev_position_0 = position_0;
         }

         if (prev_position_1 != position_1)
         {
            prev_position_1 = position_1;
         }

         position_0 = new_event->touch_pos(start_event->points[0].identifier);
         position_1 = new_event->touch_pos(start_event->points[1].identifier);

         return GS_MOVE;
      }
      else if (new_event->type == pointer_evt::touch_ended)
      {
         if (start_event->type == pointer_evt::touch_invalid || start_event->touch_count < 2)
         {
            return GS_NONE;
         }

         position_0 = new_event->touch_pos(start_event->points[0].identifier);
         position_1 = new_event->touch_pos(start_event->points[1].identifier);
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
   // prev position of first touch
   glm::vec2 prev_position_0;
   // prev position of second touch
   glm::vec2 prev_position_1;
   // start position of first touch
   glm::vec2 start_position_0;
   // start position of second touch
   glm::vec2 start_position_1;

   std::shared_ptr<pointer_evt> start_event;
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
      // check for cancelled event
      if (new_event->type == pointer_evt::touch_cancelled)
      {
         return reset();
      }

      // need 2 touches
      if ((new_event->type != pointer_evt::touch_ended) && (new_event->touch_count != 2))
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
         if (new_event->touch_count < 2)
         {
            return GS_NONE;
         }

         start_event = new_event;
         start_position_0 = new_event->touch_pos(new_event->points[0].identifier);
         start_position_1 = new_event->touch_pos(new_event->points[1].identifier);
         position_0 = start_position_0;
         prev_position_1 = position_1 = start_position_1;

         return GS_START;
      }
      else if (new_event->type == pointer_evt::touch_moved)
      {
         // cancel if start event is not valid
         if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
         {
            return GS_NONE;
         }

         position_0 = new_event->touch_pos(start_event->points[0].identifier);
         prev_position_1 = position_1;
         position_1 = new_event->touch_pos(start_event->points[1].identifier);

         float dist = glm::distance(start_position_0, position_0);

         if (dist > ROTATION_MAX_POINTER_DEVIATION)
         {
            return GS_NONE;
         }

         return GS_MOVE;
      }
      else if (new_event->type == pointer_evt::touch_ended)
      {
         if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
         {
            return GS_NONE;
         }

         position_0 = new_event->touch_pos(start_event->points[0].identifier);
         position_1 = new_event->touch_pos(start_event->points[1].identifier);
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
   // prev position of second touch
   glm::vec2 prev_position_1;
   // start position of first touch
   glm::vec2 start_position_0;
   // start position of second touch
   glm::vec2 start_position_1;

   std::shared_ptr<pointer_evt> start_event;
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
