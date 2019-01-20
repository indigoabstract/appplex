#pragma once

#include "pfm.hpp"
#include "min.hpp"
#include "com/util/util.hpp"
#include <atomic>
#include <string>
#include <vector>
#include <glm/inc.hpp>

class key_ctrl;
class touchctrl;


class pointer_evt : public iadp
{
public:
   static const std::string TOUCHSYM_EVT_TYPE;
   static const int MAX_TOUCH_POINTS = 8;

   enum e_touch_type
   {
      touch_invalid,
      touch_began,
      touch_moved,
      touch_ended,
      touch_cancelled,
      mouse_wheel,
   };

   struct touch_point
   {
      glm::vec2 get_position() const
      {
         return glm::vec2(x, y);
      }

      uintptr_t identifier = 0;
      float x = 0.f;
      float y = 0.f;
      bool is_changed = false;
   };

   pointer_evt();
   // used for debugging!
   virtual void process() override;
   static mws_sp<pointer_evt> as_pointer_evt(shared_ptr<iadp> idp);
   bool is_multitouch();

   // pointer_down_count
   // params:
   // return:
   //    return number of fingers currently pressed when this event was generated, and plus one if mouse button is down
   int pointer_down_count();

   // get_pointer_press_by_index
   // params:
   //    PointerIndex... pointer index
   // return:
   //    returns a touch point at PointerIndex if it exists, else if PointerIndex is 0 return mouse press if it exists, else return null
   const touch_point* get_pointer_press_by_index(uint32 pointer_index);

   const touch_point* find_point(uintptr_t touch_id) const;
   bool same_touches(const pointer_evt& other) const;
   glm::vec2 touch_pos(uintptr_t touch_id) const;
   std::string get_evt_type();

   e_touch_type type = touch_invalid;
   uint32 time;
   uint32 touch_count = 0;
   touch_point points[MAX_TOUCH_POINTS];
   real32 mouse_wheel_delta;
};


class touchctrl : public enable_shared_from_this<touchctrl>, public ia_broadcaster
{
public:
   static shared_ptr<touchctrl> nwi();
   shared_ptr<touchctrl> get_instance();

   bool is_pointer_released();
   void update();
   void enqueue_pointer_event(std::shared_ptr<pointer_evt> ite);

   std::atomic<std::vector<std::shared_ptr<pointer_evt> >*> queue_ptr;

private:
   touchctrl();

   virtual shared_ptr<ia_sender> sender_inst();

   void on_pointer_action_pressed(std::shared_ptr<pointer_evt> pa);
   void on_pointer_action_dragged(std::shared_ptr<pointer_evt> pa);
   void on_pointer_action_released(std::shared_ptr<pointer_evt> pa);
   void on_pointer_action_mouse_wheel(std::shared_ptr<pointer_evt> pa);

   int queue_idx;
   std::vector<std::vector<std::shared_ptr<pointer_evt> > > queue_tab;

   static bool is_pointer_down;
   static point2d first_press;
   static point2d last_pointer_pos;
   static uint32 pointer_press_time;
   static uint32 pointer_last_event_time;
   static uint32 pointer_release_time;
};


class key_evt : public enable_shared_from_this<key_evt>, public iadp
{
public:
   enum key_evt_types
   {
      KE_PRESSED,
      KE_REPEATED,
      KE_RELEASED,
   };

   static const std::string KEYEVT_EVT_TYPE;
   static const std::string KEYEVT_PRESSED;
   static const std::string KEYEVT_REPEATED;
   static const std::string KEYEVT_RELEASED;

   static shared_ptr<key_evt> as_key_evt(shared_ptr<iadp> idp);
   static shared_ptr<key_evt> nwi(std::weak_ptr<key_ctrl> isrc, key_evt_types itype, key_types i_key);
   shared_ptr<key_evt> get_instance();

   static const std::string& get_type_name(key_evt_types tstype);
   std::shared_ptr<key_ctrl> get_src();
   bool is_pressed() const;
   bool is_repeated() const;
   bool is_released() const;
   key_evt_types get_type()  const;
   key_types get_key() const;
   virtual void process();

private:

   key_evt(std::weak_ptr<key_ctrl> isrc, key_evt_types itype, key_types i_key);

   key_evt_types type;
   key_types key;
   std::weak_ptr<key_ctrl> src;
};


class key_ctrl : public enable_shared_from_this<key_ctrl>, public ia_broadcaster
{
public:
   static shared_ptr<key_ctrl> nwi();
   shared_ptr<key_ctrl> get_instance();

   void update();
   bool key_is_held(key_types i_key);
   void key_pressed(int i_key);
   void key_released(int i_key);

private:
   key_ctrl();

   virtual shared_ptr<ia_sender> sender_inst();
   void new_key_event(shared_ptr<key_evt> ts);

   bool events_pending;
   // common for all instances
   static uint8 keys_status[KEY_COUNT];
   static uint32 keys_status_time[KEY_COUNT];
};
