#include "stdafx.h"

#include "unit-exercise.hpp"

#ifdef UNIT_EXERCISE

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "com/unit/transitions.hpp"
#include "min.hpp"
#include "public/cmd/start-process.hpp"
#include "public/cmd/long-operation.hpp"
#include <boost/date_time.hpp>

using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::time_duration;
using boost::posix_time::milliseconds;

ptime lt(microsec_clock::local_time());
ptime backlightTime;

unicodestring exeUnlocked = untr("dbus-send --print-reply --system --dest=\"com.nokia.mce\" \"/com/nokia/mce/request\" \"com.nokia.mce.request.req_tklock_mode_change\" \"string:unlocked\"");
unicodestring exeBacklight = untr("dbus-send --system --type=method_call --dest=\"com.nokia.mce\" \"/com/nokia/mce/request\" \"com.nokia.mce.request.req_display_blanking_pause\"");




#define ACCELEROMETER_FILE_N900 "/sys/class/i2c-adapter/i2c-3/3-001d/coord"

class accelerometer
{
public:
	static int x;
	static int y;
	static int z;

	static bool update()
	{
		int tmp[3] = { 0, 0, 0 };
		FILE *fd;

		if (!(fd = fopen(ACCELEROMETER_FILE_N900, "r")))
		{
			return false;
		}

		if (fscanf(fd, "%i %i %i", tmp, tmp + 1, tmp + 2) != 3)
		{
			return false;
		}

		if (fclose(fd) == EOF)
		{
			return false;
		}

		x = tmp[0];
		y = tmp[1];
		z = tmp[2];

		return true;
	}

private:
	accelerometer(){}
};

int accelerometer::x = 0;
int accelerometer::y = 0;
int accelerometer::z = 0;


class holdlightlongop : public long_operation
{
	void run()
	{
		start_process::exe_shell(exeUnlocked);
	}
};


namespace unit_exercise_pref
{
	class unit_preferences_detail : public unit_preferences
	{
	public:
		virtual bool draw_touch_symbols_trail(){ return true; }
	};
}


unit_exercise::unit_exercise()
{
	set_name("exercise");
	prefs = shared_ptr<unit_preferences>(new unit_exercise_pref::unit_preferences_detail());
}

shared_ptr<unit_exercise> unit_exercise::nwi()
{
	return shared_ptr<unit_exercise>(new unit_exercise());
}

bool unit_exercise::update()
{
	if (pfm::get_platform_id() == platform_android)
	{
		//if(accelerometer::update())
		//{
		//	string str = trs("x[%+05d]y[%+05d]z[%+05d]") % accelerometer::x % accelerometer::y % accelerometer::z;
		//	float ts = 5.5;

		//	g->saveTransformState();
		//	g->scale(ts, ts);
		//	g->drawText(str, 1, 0);
		//	g->restoreTransformState();
		//}

		time_duration el = microsec_clock::local_time() - backlightTime;

		if (el.total_seconds() > 50)
			// keep backlight on
		{
			lop->join();
			lop->run_on_separate_thread(lop);
			backlightTime = microsec_clock::local_time();
		}
	}

	return unit::update();
}

void unit_exercise::init()
{
	if (pfm::get_platform_id() == platform_android)
	{
		lop = shared_ptr<long_operation>(new holdlightlongop());
		start_process::exe_shell(exeUnlocked);
	}
}

void unit_exercise::load()
{
	lt = microsec_clock::local_time();
	backlightTime = lt;
}

void unit_exercise::unload()
{
	time_duration el2 = microsec_clock::local_time() - lt;

	if (lop)
	{
		lop->join();
	}

	trx("elapse2 {}", el2.total_milliseconds());
}

#endif
