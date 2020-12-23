#pragma once

#include "input-def.hxx"
#include "min.hxx"
#include "util/util.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
#include <atomic>
#include <string>
#include <vector>
mws_pop_disable_all_warnings

class mws_key_ctrl;
class mws_touch_ctrl;


class mws_ptr_evt : public mws_ptr_evt_base
{
public:
   static const std::string ptr_evt_type;

   static mws_sp<mws_ptr_evt> nwi();
   virtual ~mws_ptr_evt() {}

   // used for debugging!
   void process(mws_sp<mws_receiver> i_dst) override;
   static mws_sp<mws_ptr_evt> as_pointer_evt(mws_sp<mws_dp> i_dp);
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
   const touch_point* get_pointer_press_by_index(uint32_t i_pointer_index);

   const touch_point* find_point(uintptr_t i_touch_id) const;
   bool same_touches(const mws_ptr_evt& i_other) const;
   glm::vec2 touch_pos(uintptr_t i_touch_id) const;
   std::string get_evt_type();
   static glm::vec2 get_pos(const touch_point& i_tp);

protected:
   mws_ptr_evt() {}
};


class mws_touch_ctrl : public std::enable_shared_from_this<mws_touch_ctrl>, public mws_broadcaster
{
public:
   static mws_sp<mws_touch_ctrl> nwi();
   mws_sp<mws_touch_ctrl> get_instance();

   bool is_pointer_released();
   void update();
   void enqueue_pointer_event(mws_sp<mws_ptr_evt_base> i_pe);

   std::atomic<std::vector<mws_sp<mws_ptr_evt>>*> queue_ptr{ nullptr };

private:
   mws_touch_ctrl();

   virtual mws_sp<mws_sender> sender_inst();

   int queue_idx = 0;
   std::vector<std::vector<mws_sp<mws_ptr_evt>>> queue_tab;
   mws_sp<mws_ptr_evt> prev_ptr_evt;

   // prevents mouse-move events from messing up the touch/pointer events
   inline static bool is_pointer_down = false;
};


class mws_key_evt : public std::enable_shared_from_this<mws_key_evt>, public mws_dp
{
public:
   enum key_evt_types
   {
      ke_pressed,
      ke_repeated,
      ke_released,
   };

   static const std::string key_evt_type;
   static const std::string key_evt_pressed;
   static const std::string key_evt_repeated;
   static const std::string key_evt_released;

   static mws_sp<mws_key_evt> as_key_evt(mws_sp<mws_dp> i_dp);
   static mws_sp<mws_key_evt> nwi(mws_wp<mws_key_ctrl> i_src, key_evt_types i_type, mws_key_types i_key);
   mws_sp<mws_key_evt> get_instance();

   static bool is_ascii(int i_key_id);
   static const std::string& get_type_name(key_evt_types i_key_evt);
   mws_sp<mws_key_ctrl> get_src();
   bool is_letter() const;
   bool is_pressed() const;
   bool is_repeated() const;
   bool is_released() const;
   key_evt_types get_type()  const;
   mws_key_types get_key() const;
   void process(mws_sp<mws_receiver> i_dst) override;

private:
   mws_key_evt(mws_wp<mws_key_ctrl> i_src, key_evt_types i_type, mws_key_types i_key);

   key_evt_types type;
   mws_key_types key;
   mws_wp<mws_key_ctrl> src;
};


class mws_key_ctrl : public std::enable_shared_from_this<mws_key_ctrl>, public mws_broadcaster
{
public:
   static mws_sp<mws_key_ctrl> nwi();
   mws_sp<mws_key_ctrl> get_instance();

   void update();
   bool key_is_held(mws_key_types i_key);
   void key_pressed(mws_key_types i_key);
   void key_released(mws_key_types i_key);
   // resets all keys to the default [ 0 ms, mws_key_IDLE ]
   void clear_keys();

private:
   mws_key_ctrl();

   virtual mws_sp<mws_sender> sender_inst();
   void new_key_event(mws_sp<mws_key_evt> i_ke);

   bool events_pending = false;
   // common for all instances
   static uint32_t time_until_first_key_repeat_ms;
   static uint32_t key_repeat_threshold_ms;
   static uint32_t max_key_repeat_count;
};
