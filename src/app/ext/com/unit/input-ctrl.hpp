#pragma once

#include "pfm.hpp"
#include "min.hpp"
#include "com/util/util.hpp"
#include <atomic>
#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>

class keyctrl;
class touchctrl;


class pointer_evt
{
public:
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

   pointer_evt()
   {
      type = touch_invalid;
      time = 0;
      touch_count = 0;
      mouse_wheel_delta = 0;
   }

   bool is_multitouch()
   {
      return touch_count > 1;
   }

   // pointer_down_count
   // params:
   // return:
   //    return number of fingers currently pressed when this event was generated, and plus one if mouse button is down
   int pointer_down_count()
   {
      int count = touch_count;

      //if (mMousePressed)
      //{
      //   count++;
      //}

      return count;
   }

   // get_pointer_press_by_index
   // params:
   //    PointerIndex... pointer index
   // return:
   //    returns a touch point at PointerIndex if it exists, else if PointerIndex is 0 return mouse press if it exists, else return null
   const touch_point* get_pointer_press_by_index(uint32 PointerIndex)
   {
      if (touch_count > PointerIndex)
      {
         return &points[PointerIndex];
      }
      //else if (PointerIndex == 0 && mMousePressed)
      //{
      //   return mMousePressed.get();
      //}

      return nullptr;
   }

   const touch_point* find_point(uintptr_t touch_id) const
   {
      for (uint32 i = 0; i < touch_count; i++)
      {
         if (this->points[i].identifier == touch_id)
         {
            return &(this->points[i]);
         }
      }

      return nullptr;
   }

   bool same_touches(const pointer_evt& other) const
   {
      if (other.touch_count == this->touch_count)
      {
         for (uint32 i = 0; i < this->touch_count; i++)
         {
            if (nullptr == this->find_point(other.points[i].identifier))
            {
               return false;
            }
         }

         return true;
      }

      return false;
   }

   glm::vec2 touch_pos(uintptr_t touch_id) const
   {
      const touch_point* p = this->find_point(touch_id);
      mws_assert(p);

      return glm::vec2(p->x, p->y);
   }

   e_touch_type type = touch_invalid;
   uint32 time;
   uint32 touch_count = 0;
   touch_point points[MAX_TOUCH_POINTS];
   real32 mouse_wheel_delta;
};


struct pointer_sample
{
	std::shared_ptr<pointer_evt> te;
	point2d vel;
	point2d acc;
	uint32 delta_pressed_time;
	float dt;
};


class touch_sym_evt : public enable_shared_from_this<touch_sym_evt>, public iadp
{
public:
	enum touch_sym_evt_types
	{
		TS_PRESSED,
      TS_DRAGGED,
      TS_RELEASED,
		TS_FIRST_TAP,
		TS_TAP,
		TS_DOUBLE_TAP,
		TS_TRIPLE_TAP,
		TS_PRESS_AND_DRAG,
		TS_PRESS_AND_HOLD,
		// swipes
		TS_BACKWARD_SWIPE,
		TS_FORWARD_SWIPE,
		TS_UPWARD_SWIPE,
		TS_DOWNWARD_SWIPE,
		TS_MOUSE_WHEEL,
	};

	static const std::string TOUCHSYM_EVT_TYPE;
	static const std::string TOUCHSYM_PRESSED;
	static const std::string TOUCHSYM_RELEASED;
	static const std::string TOUCHSYM_FIRST_TAP;
	static const std::string TOUCHSYM_TAP;
	static const std::string TOUCHSYM_DOUBLE_TAP;
	static const std::string TOUCHSYM_TRIPLE_TAP;
	static const std::string TOUCHSYM_PRESS_AND_DRAG;
	static const std::string TOUCHSYM_PRESS_AND_HOLD;
	static const std::string TOUCHSYM_BACKWARD_WIPE;
	static const std::string TOUCHSYM_FORWARD_SWIPE;
	static const std::string TOUCHSYM_UPWARD_SWIPE;
	static const std::string TOUCHSYM_DOWNWARD_SWIPE;
	static const std::string TOUCHSYM_MOUSE_WHEEL;

	touch_sym_evt(touch_sym_evt_types itype);
	virtual ~touch_sym_evt(){}
	static shared_ptr<touch_sym_evt> as_touch_sym_evt(shared_ptr<iadp> idp);
	shared_ptr<touch_sym_evt> get_instance();

	static const std::string& get_type_name(touch_sym_evt_types tstype);
	touch_sym_evt_types get_type();
	virtual void process();

	int tap_count;
	pointer_sample pressed;
	pointer_sample released;
	pointer_sample crt_state;
	pointer_sample prev_state;
	bool is_finished;

private:
	friend class touchctrl;
	void set_type(touch_sym_evt_types itype);

	touch_sym_evt_types type;
};


class mouse_wheel_evt : public touch_sym_evt
{
public:
    mouse_wheel_evt() : touch_sym_evt(touch_sym_evt::TS_MOUSE_WHEEL){}
	virtual ~mouse_wheel_evt(){}

	int x;
	int y;
	int wheel_delta;
};


class touchctrl : public enable_shared_from_this<touchctrl>, public ia_broadcaster
{
public:
	static shared_ptr<touchctrl> nwi();
	shared_ptr<touchctrl> get_instance();

	bool is_pointer_released();
	const std::vector<pointer_sample>& get_pointer_samples();
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

	void on_pointer_pressed_event(pointer_sample& ps);
	void on_pointer_dragged_event(pointer_sample& ps);
	void on_pointer_released_event(pointer_sample& ps);
	void new_touch_symbol_event(shared_ptr<touch_sym_evt> ts);

	int queue_idx;
	std::vector<std::vector<std::shared_ptr<pointer_evt> > > queue_tab;

	int TAP_PRESS_RELEASE_DELAY;
	int TAP_NEXT_PRESS_DELAY;
	int HOLD_DELAY;
	// max dist in pixels between press and release
	int DRAG_MAX_RADIUS_SQ;


	// common for all instances
	static std::vector<pointer_sample> pointer_samples;
	static std::vector<shared_ptr<touch_sym_evt> > tap_sym_events;
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
	static shared_ptr<key_evt> nwi(std::weak_ptr<keyctrl> isrc, key_evt_types itype, int ikey);
	shared_ptr<key_evt> get_instance();

	static const std::string& get_type_name(key_evt_types tstype);
	std::shared_ptr<keyctrl> get_src();
   bool is_pressed() const;
   bool is_repeated() const;
   bool is_released() const;
   key_evt_types get_type()  const;
	int get_key() const;
	virtual void process();

private:

	key_evt(std::weak_ptr<keyctrl> isrc, key_evt_types itype, int ikey);

	key_evt_types type;
	int key;
	std::weak_ptr<keyctrl> src;
};


class keyctrl : public enable_shared_from_this<keyctrl>, public ia_broadcaster
{
public:
	static shared_ptr<keyctrl> nwi();
	shared_ptr<keyctrl> get_instance();

	void update();
	bool key_is_held(key_types ikey);
	void key_pressed(int ikey);
	void key_released(int ikey);

private:
	keyctrl();

	virtual shared_ptr<ia_sender> sender_inst();
	void new_key_event(shared_ptr<key_evt> ts);

	bool events_pending;
	// common for all instances
	static char key_status[KEY_COUNT];
	static unsigned long key_status_time[KEY_COUNT];
};
