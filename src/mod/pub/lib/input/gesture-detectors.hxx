#pragma once

#include "pfm.hxx"
#include "input-ctrl.hxx"
#include "gfx.hxx"


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
const uint32_t DOUBLE_TAP_MAX_DURATION = 500;
// max distance between first press position an subsequent pointer positions
const mws_cm DOUBLE_TAP_MAX_POINTER_DISTANCE(0.1f);
// max distance between first press position an subsequent pointer positions for rotating with touch gestures
const uint32_t ROTATION_MAX_POINTER_DEVIATION = 15;


class dragging_detector
{
public:
   dragging_detector();
   bool detect_helper(mws_sp<mws_ptr_evt> evt);
   // returns true if the dragging gesture is finished, false if it is in progress
   bool is_finished() const;
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   glm::vec2 drag_diff = glm::vec2(0.f);
   glm::vec2 press_pos = glm::vec2(0.f);
   glm::vec2 pointer_pos = glm::vec2(0.f);
   glm::vec2 last_move_pos = glm::vec2(0.f);
   uint32_t last_move_pos_time = 0;
   glm::vec2 last_move_pos_bak = glm::vec2(0.f);

private:
   enum class detector_state
   {
      ST_READY,
      ST_PRESSED,
      ST_MOVING,
   };

   void set_state(detector_state i_st);
   std::string to_string(detector_state i_st) const;

   mws_sp<mws_ptr_evt> start_event;
   detector_state det_state;
};


class double_tap_detector
{
public:
   double_tap_detector();
   bool detect_helper(mws_sp<mws_ptr_evt> evt);
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();
   glm::vec2 get_avg_press_pos() const { return (first_press_pos + second_press_pos) / 2.f; }
   glm::vec2 get_first_press_pos() const { return first_press_pos; }
   glm::vec2 get_second_press_pos() const { return second_press_pos; }
   // max duration of a double tap(in ms). if more that this time has passed between taps, it's not considered a double tap anymore
   uint32_t get_double_tap_max_duration() const;
   void set_double_tap_max_duration(uint32_t i_max_duration);
   // max distance between first press position an subsequent pointer positions
   mws_cm get_double_tap_max_pointer_distance() const;
   void set_double_tap_max_pointer_distance(mws_cm i_max_distance);

   // distance evaluator custom function
   std::function<bool(const glm::vec2& i_first_press, const glm::vec2& i_second_press)> dist_eval;

private:
   enum class detector_state
   {
      ST_READY,
      ST_PRESSED_0,
      ST_RELEASED_0,
      ST_PRESSED_1,
   };

   void set_state(detector_state i_st);
   bool leq_max_dist(const glm::vec2& i_first_press, const glm::vec2& i_second_press);

   // get start position of tap
   glm::vec2 first_press_pos = glm::vec2(0.f);
   glm::vec2 second_press_pos = glm::vec2(0.f);
   uint32_t max_duration = DOUBLE_TAP_MAX_DURATION;
   mws_cm max_distance = DOUBLE_TAP_MAX_POINTER_DISTANCE;

   mws_sp<mws_ptr_evt> start_event;
   detector_state det_state;
};


class triple_tap_detector
{
public:
   triple_tap_detector();
   bool detect_helper(mws_sp<mws_ptr_evt> evt);
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

private:
   enum class detector_state
   {
      ST_READY,
      ST_PRESSED_0,
      ST_RELEASED_0,
      ST_PRESSED_1,
      ST_RELEASED_1,
      ST_PRESSED_2,
   };

   void set_state(detector_state i_st);

   // get start position of tap
   glm::vec2 first_press_pos = glm::vec2(0.f);
   glm::vec2 second_press_pos = glm::vec2(0.f);
   glm::vec2 third_press_pos = glm::vec2(0.f);
   uint32_t max_duration = DOUBLE_TAP_MAX_DURATION;
   mws_cm max_distance = DOUBLE_TAP_MAX_POINTER_DISTANCE;

   mws_sp<mws_ptr_evt> start_event;
   detector_state det_state;
};


class pinch_zoom_detector
{
public:
   pinch_zoom_detector();
   bool detect_helper(mws_sp<mws_ptr_evt> evt);
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   // position of first touch
   glm::vec2 position_0 = glm::vec2(0.f);
   // position of second touch
   glm::vec2 position_1 = glm::vec2(0.f);
   // prev position of first touch
   glm::vec2 prev_position_0 = glm::vec2(0.f);
   // prev position of second touch
   glm::vec2 prev_position_1 = glm::vec2(0.f);
   // start position of first touch
   glm::vec2 start_position_0 = glm::vec2(0.f);
   // start position of second touch
   glm::vec2 start_position_1 = glm::vec2(0.f);
   float zoom_factor = 0.f;
   float last_dist = 0.f;

   mws_sp<mws_ptr_evt> start_event;
   gesture_state det_state;
};


class anchor_rotation_one_finger_detector
{
public:
   anchor_rotation_one_finger_detector();

   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   // position of finger
   glm::vec2 position = glm::vec2(0.f);
   // prev position of finger
   glm::vec2 prev_position = glm::vec2(0.f);
   // start position of finger press
   glm::vec2 start_position = glm::vec2(0.f);
   unsigned long start_time = 0;

   mws_sp<mws_ptr_evt> start_event;
};


class axis_roll_detector
{
public:
   axis_roll_detector();

   bool detect_helper(mws_sp<mws_ptr_evt> evt, float& rotation_angle);
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   // position of first touch
   glm::vec2 position_0 = glm::vec2(0.f);
   // position of second touch
   glm::vec2 position_1 = glm::vec2(0.f);
   // start position of first touch
   glm::vec2 start_position_0 = glm::vec2(0.f);
   // start position of second touch
   glm::vec2 start_position_1 = glm::vec2(0.f);

   glm::vec2 anchor_touch_position = glm::vec2(0.f);
   glm::vec2 roll_touch_prev_position = glm::vec2(0.f);
   glm::vec2 roll_touch_position = glm::vec2(0.f);

   mws_sp<mws_ptr_evt> start_event;
   gesture_state det_state;
};


class panning_tilting_detector
{
public:
   panning_tilting_detector();
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   // position of first touch
   glm::vec2 position_0 = glm::vec2(0.f);
   // position of second touch
   glm::vec2 position_1 = glm::vec2(0.f);
   // position of third touch
   glm::vec2 position_2 = glm::vec2(0.f);
   // prev position of third touch
   glm::vec2 prev_position_2 = glm::vec2(0.f);
   // start position of first touch
   glm::vec2 start_position_0 = glm::vec2(0.f);
   // start position of second touch
   glm::vec2 start_position_1 = glm::vec2(0.f);
   // start position of third touch
   glm::vec2 start_position_2 = glm::vec2(0.f);

   mws_sp<mws_ptr_evt> start_event;
};


enum class swipe_types
{
   left = (1 << 0),
   right = (1 << 1),
   up = (1 << 2),
   down = (1 << 3),

   up_left = (1 << 4),
   down_left = (1 << 5),
   up_right = (1 << 6),
   down_right = (1 << 7),

   horizontal = (left | right),
   vertical = (up | down),
   cardinal = (horizontal | vertical),

   diagonal_up = (up_left | up_right),
   diagonal_down = (down_left | down_right),
   diagonal_left = (up_left | down_left),
   diagonal_right = (up_right | down_right),
   diagonal = (diagonal_up | diagonal_down),

   right_side = (right | diagonal_right),
   left_side = (left | diagonal_left),
   top_side = (up | diagonal_up),
   bottom_side = (down | diagonal_down),

   all = (horizontal | vertical | diagonal)
};


class swipe_detector
{
public:
   uint32_t max_swipe_duration = 500;
   float swipe_speed_cms = 0.f;
   swipe_types swipe_direction;
   int min_fingers_pressed = 1;
   int max_fingers_pressed = 2;
   bool trigger_before_touch_ended = false;
   float min_swipe_dist_cm = 2.f;
   float max_swipe_dist_cm = 10.f;
   std::vector<mws_sp<mws_ptr_evt>> touch_vect;
   uint32_t start_time;

   swipe_detector();
   bool detect_helper(mws_sp<mws_ptr_evt> evt);
   bool was_started_from_edge() const;
   gesture_state detect(const mws_sp<mws_ptr_evt> new_event);
   gesture_state reset();

   glm::vec2 press_pos = glm::vec2(0.f);
   glm::vec2 pointer_pos = glm::vec2(0.f);

private:
   enum class detector_state
   {
      ST_READY,
      ST_PRESSED,
      ST_MOVING,
   };

   void set_state(detector_state i_st);
   std::string to_string(detector_state i_st) const;

   mws_sp<mws_ptr_evt> start_event;
   detector_state det_state;

   bool is_valid_swipe();
};
