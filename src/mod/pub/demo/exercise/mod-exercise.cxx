#include "stdafx.hxx"

#include "mod-exercise.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "input/transitions.hxx"
#include "min.hxx"
#include "demo/cmd/start-process.hxx"
#include "demo/cmd/long-operation.hxx"
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


namespace mod_exercise_pref
{
	class mod_preferences_detail : public mws_mod_preferences
	{
	public:
		virtual bool draw_touch_symbols_trail(){ return true; }
	};
}


mod_exercise::mod_exercise() : mws_mod(mws_stringify(MOD_EXERCISE))
{
	prefs = mws_sp<mws_mod_preferences>(new mod_exercise_pref::mod_preferences_detail());
}

mws_sp<mod_exercise> mod_exercise::nwi()
{
	return mws_sp<mod_exercise>(new mod_exercise());
}

bool mod_exercise::update()
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

	return mws_mod::update();
}

void mod_exercise::init()
{
	if (pfm::get_platform_id() == platform_android)
	{
		lop = mws_sp<long_operation>(new holdlightlongop());
		start_process::exe_shell(exeUnlocked);
	}
}

void mod_exercise::load()
{
	lt = microsec_clock::local_time();
	backlightTime = lt;
}

void mod_exercise::unload()
{
	time_duration el2 = microsec_clock::local_time() - lt;

	if (lop)
	{
		lop->join();
	}

	trx("elapse2 {}", el2.total_milliseconds());
}
