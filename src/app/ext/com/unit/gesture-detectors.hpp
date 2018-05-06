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
   dragging_detector();
   bool detect_helper(std::shared_ptr<pointer_evt> evt);
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

   glm::vec2 drag_diff;
   glm::vec2 press_pos;
   glm::vec2 pointer_pos;
   glm::vec2 last_move_pos;
   glm::vec2 last_move_pos_bak;

private:
   enum class tap_detector_state
   {
      ST_READY,
      ST_PRESSED,
      ST_MOVING,
   };

   void set_state(tap_detector_state i_st);
   std::string to_string(tap_detector_state i_st) const;

   std::shared_ptr<pointer_evt> start_event;
   tap_detector_state det_state;
};


class double_tap_detector
{
public:
   double_tap_detector();
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

private:
   enum class tap_detector_state
   {
      ST_READY,
      ST_PRESSED_0,
      ST_RELEASED_0,
      ST_PRESSED_1,
   };

   void set_state(tap_detector_state i_st);

   // get start position of tap
   glm::vec2 first_press_pos;
   glm::vec2 second_press_pos;

   std::shared_ptr<pointer_evt> start_event;
   tap_detector_state det_state;
};


class pinch_zoom_detector
{
public:
   pinch_zoom_detector();
   bool detect_helper(std::shared_ptr<pointer_evt> evt, float& zoom_factor);
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

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
   anchor_rotation_one_finger_detector();

   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

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
   axis_roll_detector();

   bool detect_helper(std::shared_ptr<pointer_evt> evt, float& rotation_angle);
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

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
   panning_tilting_detector();
   gesture_state detect(const std::shared_ptr<pointer_evt> new_event);
   gesture_state reset();

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
