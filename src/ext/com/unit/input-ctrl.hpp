#pragma once

#include "pfm.hpp"
#include "min.hpp"
#include "com/util/util.hpp"
#include <atomic>
#include <string>
#include <vector>

class keyctrl;
class touchctrl;


enum touch_event
{
	POINTER_PRESSED,
	POINTER_DRAGGED,
	POINTER_RELEASED,
};


struct pointer_sample
{
	touch_event te;
	point2d pos;
	point2d vel;
	point2d acc;
	uint32 time;
	uint32 delta_pressed_time;
	float dt;
};


class pointer_action
{
public:
	pointer_action(pointer_actions iaction, uint32 itime, int ix, int iy, int ipress_id) :
		action(iaction), time(itime), x(ix), y(iy), press_id(ipress_id)
	{}

	pointer_actions action;
	uint32 time;
	int x;
	int y;
	int press_id;
};

class touch_sym_evt : public enable_shared_from_this<touch_sym_evt>, public iadp
{
public:
	enum touch_sym_evt_types
	{
		TS_PRESSED,
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
	static shared_ptr<touchctrl> new_instance();
	shared_ptr<touchctrl> get_instance();

	bool is_pointer_released();
	const std::vector<pointer_sample>& get_pointer_samples();
	void update();
	void pointer_pressed(int ix, int iy);
	void pointer_dragged(int ix, int iy);
	void pointer_released(int ix, int iy);
	void on_mouse_wheel(int ix, int iy, int iwheel_delta);

	std::atomic<std::vector<pointer_action>*> queue_ptr;

private:
	touchctrl();

	virtual shared_ptr<ia_sender> sender_inst();

	void on_pointer_action_pressed(pointer_action& pa);
	void on_pointer_action_dragged(pointer_action& pa);
	void on_pointer_action_released(pointer_action& pa);
	void on_pointer_action_mouse_wheel(pointer_action& pa);

	void on_pointer_pressed_event(pointer_sample& ps);
	void on_pointer_dragged_event(pointer_sample& ps);
	void on_pointer_released_event(pointer_sample& ps);
	void new_touch_symbol_event(shared_ptr<touch_sym_evt> ts);

	int queue_idx;
	std::vector<std::vector<pointer_action> > queue_tab;

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
	static shared_ptr<key_evt> new_instance(std::weak_ptr<keyctrl> isrc, key_evt_types itype, int ikey);
	shared_ptr<key_evt> get_instance();

	static const std::string& get_type_name(key_evt_types tstype);
	std::shared_ptr<keyctrl> get_src();
	key_evt_types get_type();
	int get_key();
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
	static shared_ptr<keyctrl> new_instance();
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
