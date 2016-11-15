#include "stdafx.h"

#include "input-ctrl.hpp"
#include "pfm-def.h"
#include "unit.hpp"
#include <glm/gtc/constants.hpp>

using std::string;
using std::vector;


const string touch_sym_evt::TOUCHSYM_EVT_TYPE						= "ts-";
const string touch_sym_evt::TOUCHSYM_PRESSED						= "ts-pressed";
const string touch_sym_evt::TOUCHSYM_RELEASED						= "ts-released";
const string touch_sym_evt::TOUCHSYM_FIRST_TAP						= "ts-first-tap";
const string touch_sym_evt::TOUCHSYM_TAP							= "ts-tap";
const string touch_sym_evt::TOUCHSYM_DOUBLE_TAP						= "ts-double-tap";
const string touch_sym_evt::TOUCHSYM_TRIPLE_TAP						= "ts-triple-tap";
const string touch_sym_evt::TOUCHSYM_PRESS_AND_DRAG					= "ts-press-and-drag";
const string touch_sym_evt::TOUCHSYM_PRESS_AND_HOLD					= "ts-press-and-hold";
const string touch_sym_evt::TOUCHSYM_BACKWARD_WIPE					= "ts-backward-swipe";
const string touch_sym_evt::TOUCHSYM_FORWARD_SWIPE					= "ts-forward-swipe";
const string touch_sym_evt::TOUCHSYM_UPWARD_SWIPE					= "ts-upward-swipe";
const string touch_sym_evt::TOUCHSYM_DOWNWARD_SWIPE					= "ts-downward-swipe";
const string touch_sym_evt::TOUCHSYM_MOUSE_WHEEL					= "ts-mouse-wheel";


touch_sym_evt::touch_sym_evt(touch_sym_evt_types itype) : iadp(get_type_name(itype))
{
	tap_count = 0;
	type = itype;
	is_finished = false;
	//trx("newtouch %x") % this;
}

shared_ptr<touch_sym_evt> touch_sym_evt::as_touch_sym_evt(shared_ptr<iadp> idp)
{
	return static_pointer_cast<touch_sym_evt>(idp);
}

shared_ptr<touch_sym_evt> touch_sym_evt::get_instance()
{
	return shared_from_this();
}


const std::string& touch_sym_evt::get_type_name(touch_sym_evt_types tstype)
{
	static const std::string types[] =
	{
		TOUCHSYM_PRESSED,
		TOUCHSYM_RELEASED,
		TOUCHSYM_FIRST_TAP,
		TOUCHSYM_TAP,
		TOUCHSYM_DOUBLE_TAP,
		TOUCHSYM_TRIPLE_TAP,
		TOUCHSYM_PRESS_AND_DRAG,
		TOUCHSYM_PRESS_AND_HOLD,
		TOUCHSYM_BACKWARD_WIPE,
		TOUCHSYM_FORWARD_SWIPE,
		TOUCHSYM_UPWARD_SWIPE,
		TOUCHSYM_DOWNWARD_SWIPE,
		TOUCHSYM_MOUSE_WHEEL,
	};

	return types[tstype];
}

touch_sym_evt::touch_sym_evt_types touch_sym_evt::get_type()
{
	return type;
}

void touch_sym_evt::set_type(touch_sym_evt_types itype)
{
	type = itype;
	set_name(get_type_name(type));
}

void touch_sym_evt::process()
{
	//trx("touch process %x") % getInst().get();
	iadp::process();
}


vector<pointer_sample>					touchctrl::pointer_samples;
vector<shared_ptr<touch_sym_evt> >		touchctrl::tap_sym_events;
bool									touchctrl::is_pointer_down = false;
point2d									touchctrl::first_press;
point2d									touchctrl::last_pointer_pos;
uint32									touchctrl::pointer_press_time;
uint32									touchctrl::pointer_last_event_time;
uint32									touchctrl::pointer_release_time;


touchctrl::touchctrl()
{
	TAP_PRESS_RELEASE_DELAY = 200;
	TAP_NEXT_PRESS_DELAY = 200;
	HOLD_DELAY = 750;
	DRAG_MAX_RADIUS_SQ = 25 * 25;

	queue_tab.resize(2);
	queue_idx = 0;
	queue_ptr = &queue_tab[queue_idx];
}

shared_ptr<touchctrl> touchctrl::new_instance()
{
	return shared_ptr<touchctrl>(new touchctrl());
}

shared_ptr<touchctrl> touchctrl::get_instance()
{
	return shared_from_this();
}

bool touchctrl::is_pointer_released()
{
	return !is_pointer_down;
}

const vector<pointer_sample>& touchctrl::get_pointer_samples()
{
	return pointer_samples;
}

void touchctrl::update()
{
	// set the current input queue as the queue for processing the input
	std::vector<pointer_action>* input_queue_ptr = &queue_tab[queue_idx];

	// switch queues, so the currently empty queue is used for taking input events
	queue_idx = (queue_idx + 1) % 2;
	queue_tab[queue_idx].clear();
	queue_ptr = &queue_tab[queue_idx];

	if (!input_queue_ptr->empty())
	{
		for (auto pa : *input_queue_ptr)
		{
			switch (pa.action)
			{
			case POINTER_PRESS:
				on_pointer_action_pressed(pa);
				break;

			case POINTER_RELEASE:
				on_pointer_action_released(pa);
				break;

			case POINTER_DRAGG:
				on_pointer_action_dragged(pa);
				break;

			case POINTER_MOUSE_WHEEL:
				on_pointer_action_mouse_wheel(pa);
				break;
			}
		}
	}

	if(!tap_sym_events.empty())
	{
		shared_ptr<touch_sym_evt> ts = tap_sym_events.back();
		uint32 crtTime =  pfm::time::get_time_millis();
		uint32 delta = crtTime - ts->crt_state.time;

		switch(ts->get_type())
		{
		case touch_sym_evt::TS_PRESSED:
			{
				if(delta >= HOLD_DELAY)
				{
					shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_PRESS_AND_HOLD));

					ts->set_type(touch_sym_evt::TS_PRESS_AND_HOLD);
					*nts = *ts;
					new_touch_symbol_event(nts);
				}

				break;
			}

		case touch_sym_evt::TS_TAP:
			if(delta > TAP_NEXT_PRESS_DELAY)
			{
				shared_ptr<touch_sym_evt> nts(new touch_sym_evt(ts->get_type()));

				*nts = *ts;

				switch(ts->tap_count)
				{
				case 2:
					nts->set_type(touch_sym_evt::TS_DOUBLE_TAP);
					break;

				case 3:
					nts->set_type(touch_sym_evt::TS_TRIPLE_TAP);
					break;
				}

				new_touch_symbol_event(nts);
				tap_sym_events.clear();
			}

			break;
		}
	}
}

void touchctrl::pointer_pressed(int ix, int iy)
{
	pointer_action pa(POINTER_PRESS, pfm::time::get_time_millis(), ix, iy, 0);

	(*queue_ptr).push_back(pa);
}

void touchctrl::pointer_dragged(int ix, int iy)
{
	pointer_action pa(POINTER_DRAGG, pfm::time::get_time_millis(), ix, iy, 0);

	(*queue_ptr).push_back(pa);
}

void touchctrl::pointer_released(int ix, int iy)
{
	pointer_action pa(POINTER_RELEASE, pfm::time::get_time_millis(), ix, iy, 0);

	(*queue_ptr).push_back(pa);
}

void touchctrl::on_mouse_wheel(int ix, int iy, int iwheel_delta)
{
	pointer_action pa(POINTER_MOUSE_WHEEL, pfm::time::get_time_millis(), ix, iy, iwheel_delta);

	(*queue_ptr).push_back(pa);
}

shared_ptr<ia_sender> touchctrl::sender_inst()
{
	return get_instance();
}

void touchctrl::on_pointer_action_pressed(pointer_action& pa)
{
	pointer_press_time = pa.time;

	is_pointer_down = true;
	first_press = last_pointer_pos = point2d(pa.x, pa.y);

	pointer_sample ps;

	ps.te = POINTER_PRESSED;
	ps.pos = first_press;
	ps.vel.set(0, 0);
	ps.acc.set(0, 0);
	ps.time = pa.time;
	ps.delta_pressed_time = 0;
	ps.dt = 0;

	pointer_samples.clear();
	pointer_samples.push_back(ps);
	on_pointer_pressed_event(ps);
}

void touchctrl::on_pointer_action_dragged(pointer_action& pa)
{
	if (is_pointer_down && pointer_samples.size() > 0)
	{
		pointer_sample ps;
		pointer_sample pps = pointer_samples.back();

		pointer_last_event_time = pa.time;
		last_pointer_pos.set(pa.x, pa.y);

		ps.te = POINTER_DRAGGED;
		ps.pos.set(pa.x, pa.y);
		ps.time = pa.time;
		ps.delta_pressed_time = ps.time - pointer_press_time;

		ps.dt = ps.delta_pressed_time - pps.delta_pressed_time;

		if (ps.dt > 0)
		{
			ps.vel.set((ps.pos.x - pps.pos.x) / ps.dt, (ps.pos.y - pps.pos.y) / ps.dt);
			ps.acc.set((ps.vel.x - pps.vel.x) / ps.dt, (ps.vel.y - pps.vel.y) / ps.dt);
		}
		else
		{
			ps.vel.set(0, 0);
			ps.acc.set(0, 0);
		}

		pointer_samples.push_back(ps);
		on_pointer_dragged_event(ps);
	}
}

void touchctrl::on_pointer_action_released(pointer_action& pa)
{
	pointer_release_time = pa.time;
	last_pointer_pos.set(pa.x, pa.y);
	is_pointer_down = false;

	if (pointer_samples.size() > 0)
	{
		pointer_sample ps;
		pointer_sample pps = pointer_samples.back();

		ps.te = POINTER_RELEASED;
		ps.pos.set(pa.x, pa.y);
		ps.time = pa.time;
		ps.delta_pressed_time = ps.time - pointer_press_time;

		ps.dt = ps.delta_pressed_time - pps.delta_pressed_time;

		if (ps.dt > 0)
		{
			ps.vel.set((ps.pos.x - pps.pos.x) / ps.dt, (ps.pos.y - pps.pos.y) / ps.dt);
			ps.acc.set((ps.vel.x - pps.vel.x) / ps.dt, (ps.vel.y - pps.vel.y) / ps.dt);
		}
		else
		{
			ps.vel.set(0, 0);
			ps.acc.set(0, 0);
		}

		pointer_samples.push_back(ps);
		on_pointer_released_event(ps);
	}
}

void touchctrl::on_pointer_action_mouse_wheel(pointer_action& pa)
{
	shared_ptr<mouse_wheel_evt> ts(new mouse_wheel_evt());

	ts->x = pa.x;
	ts->y = pa.y;
	ts->wheel_delta = pa.press_id;

	new_touch_symbol_event(ts);
}

void touchctrl::on_pointer_pressed_event(pointer_sample& ps)
{
	shared_ptr<touch_sym_evt> ts(new touch_sym_evt(touch_sym_evt::TS_PRESSED));

	ts->tap_count = tap_sym_events.size() + 1;
	ts->set_type(touch_sym_evt::TS_PRESSED);
	ts->prev_state = ts->crt_state = ps;
	ts->pressed = ps;
	tap_sym_events.push_back(ts);

	// dispatch event
	{
		shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_PRESSED));

		*nts = *ts;
		new_touch_symbol_event(nts);
	}
}

void touchctrl::on_pointer_dragged_event(pointer_sample& ps)
{
	ia_assert(!tap_sym_events.empty());

	if(tap_sym_events.empty())
	{
		return;
	}

	shared_ptr<touch_sym_evt> ts = tap_sym_events.back();
	pointer_sample& pressed = ts->pressed;

	ts->prev_state = ts->crt_state;
	ts->crt_state = ps;

	switch(ts->get_type())
	{
	case touch_sym_evt::TS_PRESSED:
		{
			float dx = ps.pos.x - pressed.pos.x;
			float dy = ps.pos.y - pressed.pos.y;
			float dist = dx * dx + dy * dy;

			if(dist > DRAG_MAX_RADIUS_SQ)
			{
				shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_PRESS_AND_DRAG));

				ts->set_type(touch_sym_evt::TS_PRESS_AND_DRAG);
				*nts = *ts;
				new_touch_symbol_event(nts);
			}

			break;
		}

	case touch_sym_evt::TS_PRESS_AND_DRAG:
	case touch_sym_evt::TS_PRESS_AND_HOLD:
		{
			shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_PRESS_AND_HOLD));

			*nts = *ts;
			new_touch_symbol_event(nts);
			break;
		}
	}
}

void touchctrl::on_pointer_released_event(pointer_sample& ps)
{
	ia_assert(!tap_sym_events.empty());

	if(tap_sym_events.empty())
	{
		return;
	}

	shared_ptr<touch_sym_evt> ts = tap_sym_events.back();
	pointer_sample& pressed = ts->pressed;
	shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_RELEASED));

	ts->prev_state = ts->crt_state;
	ts->crt_state = ps;
	ts->released = ps;
	ts->is_finished = true;

	*nts = *ts;
	nts->set_type(touch_sym_evt::TS_RELEASED);

	float dx = ps.pos.x - pressed.pos.x;
	float dy = ps.pos.y - pressed.pos.y;
	float dist = dx * dx + dy * dy;

	if(dist < DRAG_MAX_RADIUS_SQ)
	{
		shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_FIRST_TAP));

		*nts = *ts;
		nts->set_type(touch_sym_evt::TS_FIRST_TAP);
		new_touch_symbol_event(nts);
	}

	switch(ts->get_type())
	{
	case touch_sym_evt::TS_PRESSED:
		{
			uint32 delta = ps.time - pressed.time;

			if(delta <= TAP_PRESS_RELEASE_DELAY)
			{
				ts->set_type(touch_sym_evt::TS_TAP);
			}
			else
			{
				tap_sym_events.clear();
			}

			break;
		}

	case touch_sym_evt::TS_PRESS_AND_DRAG:
	case touch_sym_evt::TS_PRESS_AND_HOLD:
		{
			// dispatch event
			{
				shared_ptr<touch_sym_evt> nts(new touch_sym_evt(touch_sym_evt::TS_PRESS_AND_HOLD));

				*nts = *ts;
				new_touch_symbol_event(nts);
			}

			if(ts->get_type() == touch_sym_evt::TS_PRESS_AND_DRAG && ts->tap_count == 1)
				// swipe symbol
			{
				pointer_sample& p1 = pointer_samples.front();
				pointer_sample& p2 = pointer_samples.back();
				//trx("swipe %1%, %2%		%3%, %4%") % p1.pos.x % p1.pos.y % p2.pos.x % p2.pos.y;
				point2d p(p2.pos.x - p1.pos.x, p2.pos.y - p1.pos.y);
				p.y = -p.y;
				float length = sqrtf(p.x * p.x + p.y * p.y);
				float cosa = p.x / length;
				float sina = p.y / length;
				float ac = acosf(cosa) * 180 / glm::pi<float>();
				//float as = asinf(sina) * 180 / M_PI;
				//float one = sina * sina + cosa * cosa;

				if(sina < 0)
				{
					ac = 360 - ac;
				}

				//trx("x %1%, y%2%		c %3%, s %4%	ac %5%") % p.x % p.y % cosa % sina % ac;

				int verticalAngle = 20;
				int horizontalAngle = 10;
				bool isSwipe = false;
				touch_sym_evt::touch_sym_evt_types tsType;

				if(ac >= 90 - verticalAngle && ac < 90 + verticalAngle)
				{
					tsType = touch_sym_evt::TS_UPWARD_SWIPE;
					isSwipe = true;
				}
				else if(ac >= 180 - horizontalAngle && ac < 180 + horizontalAngle)
				{
					tsType = touch_sym_evt::TS_FORWARD_SWIPE;
					isSwipe = true;
				}
				else if(ac >= 270 - verticalAngle && ac < 270 + verticalAngle)
				{
					tsType = touch_sym_evt::TS_DOWNWARD_SWIPE;
					isSwipe = true;
				}
				else if(ac >= 360 - horizontalAngle || ac < 0 + horizontalAngle)
				{
					tsType = touch_sym_evt::TS_BACKWARD_SWIPE;
					isSwipe = true;
				}

				if(isSwipe)
				{
					shared_ptr<touch_sym_evt> swipe(new touch_sym_evt(tsType));

					*swipe = *ts;
					swipe->set_type(tsType);
					new_touch_symbol_event(swipe);
				}
			}

			tap_sym_events.clear();
			break;
		}
	}

	// dispatch release event
	new_touch_symbol_event(nts);
}

void touchctrl::new_touch_symbol_event(shared_ptr<touch_sym_evt> ts)
{
	//trx("tc %1% type %2% fin %3% dt %4%") % ts->tapCount % ts->getName() % ts->isFinished % (ts->crtState.time - ts->pressed.time);
	broadcast(get_instance(), ts);
}



const string key_evt::KEYEVT_EVT_TYPE						= "ke-";
const string key_evt::KEYEVT_PRESSED						= "ke-pressed";
const string key_evt::KEYEVT_REPEATED						= "ke-repeated";
const string key_evt::KEYEVT_RELEASED						= "ke-released";


key_evt::key_evt(std::weak_ptr<keyctrl> isrc, key_evt::key_evt_types itype, int ikey) : iadp(get_type_name(itype))
{
	src = isrc;
	type = itype;
	key = ikey;
	//trx("newkeyevt %x") % this;
}

shared_ptr<key_evt> key_evt::as_key_evt(shared_ptr<iadp> idp)
{
	return static_pointer_cast<key_evt>(idp);
}

shared_ptr<key_evt> key_evt::new_instance(std::weak_ptr<keyctrl> isrc, key_evt::key_evt_types itype, int ikey)
{
	return shared_ptr<key_evt>(new key_evt(isrc, itype, ikey));
}

shared_ptr<key_evt> key_evt::get_instance()
{
	return shared_from_this();
}

const std::string& key_evt::get_type_name(key_evt_types tstype)
{
	static const std::string types[] =
	{
		KEYEVT_PRESSED,
		KEYEVT_REPEATED,
		KEYEVT_RELEASED,
	};

	return types[tstype];
}

std::shared_ptr<keyctrl> key_evt::get_src()
{
	return src.lock();
}

key_evt::key_evt_types key_evt::get_type()
{
	return type;
}

int key_evt::get_key()
{
	return key;
}

void key_evt::process()
{
	//trx("keyevt process %x") % getInst().get();
	iadp::process();
}


enum keystatus
{
	KEY_IDLE,
	KEY_PRESSED,
	KEY_FIRST_PRESSED,
	KEY_REPEATED,
	KEY_RELEASED,
	KEY_RELEASED_IDLE,
};


char keyctrl::key_status[KEY_COUNT] = {KEY_IDLE};
unsigned long keyctrl::key_status_time[KEY_COUNT] = {0};
//for(int  k = 0; k < KEY_COUNT; k++)
//{
//	keyStatus[k] = KEY_IDLE;
//	keyStatusTime[k] = 0;
//}


keyctrl::keyctrl()
{
	events_pending = false;
}

shared_ptr<keyctrl> keyctrl::new_instance()
{
	return shared_ptr<keyctrl>(new keyctrl());
}

shared_ptr<keyctrl> keyctrl::get_instance()
{
	return shared_from_this();
}

void keyctrl::update()
{
	if (events_pending)
	{
		auto inst = get_instance();
		uint32 crtTime = pfm::time::get_time_millis();
		bool events_still_pending = false;

		for (int k = KEY_INVALID; k < KEY_COUNT; k++)
		{
			switch (key_status[k])
			{
			case KEY_PRESSED:
				new_key_event(key_evt::new_instance(inst, key_evt::KE_PRESSED, k));
				key_status[k] = KEY_FIRST_PRESSED;
				events_still_pending = true;
				break;

			case KEY_FIRST_PRESSED:
				if (crtTime - key_status_time[k] > 400)
				{
					new_key_event(key_evt::new_instance(inst, key_evt::KE_REPEATED, k));
					key_status[k] = KEY_REPEATED;
					key_status_time[k] = crtTime;
				}

				events_still_pending = true;
				break;

			case KEY_REPEATED:
				if (crtTime - key_status_time[k] > 25)
				{
					new_key_event(key_evt::new_instance(inst, key_evt::KE_REPEATED, k));
					key_status_time[k] = crtTime;
				}

				events_still_pending = true;
				break;

			case KEY_RELEASED:
				key_status[k] = KEY_RELEASED_IDLE;
				new_key_event(key_evt::new_instance(inst, key_evt::KE_RELEASED, k));
				events_still_pending = true;
				break;

			case KEY_RELEASED_IDLE:
				key_status[k] = KEY_IDLE;
				break;
			}
		}

		events_pending = events_still_pending;
	}
}

bool keyctrl::key_is_held(key_types ikey)
{
	ia_assert(ikey >= KEY_INVALID && ikey < KEY_COUNT);

	return key_status[ikey] != KEY_IDLE;
}

void keyctrl::key_pressed(int ikey)
{
	ia_assert(ikey >= KEY_INVALID && ikey < KEY_COUNT);
	events_pending = true;

	if(key_status[ikey] != KEY_FIRST_PRESSED && key_status[ikey] != KEY_REPEATED)
	{
		key_status[ikey] = KEY_PRESSED;
		key_status_time[ikey] = pfm::time::get_time_millis();
	}
}

void keyctrl::key_released(int ikey)
{
	ia_assert(ikey >= KEY_INVALID && ikey < KEY_COUNT);
	events_pending = true;
	key_status[ikey] = KEY_RELEASED;
}

shared_ptr<ia_sender> keyctrl::sender_inst()
{
	return get_instance();
}

void keyctrl::new_key_event(shared_ptr<key_evt> ke)
{
	//trx("keyevt type %1%) ke->getName();
	broadcast(ke->get_src(), ke);
}
