#pragma once

#include "pfm.hxx"


class updatectrl : public std::enable_shared_from_this<updatectrl>
{
public:
	enum updatestatetype
	{
		GAME_RUNNING,
		GAME_STOPPED,
		GAME_PAUSED,
	};

	static mws_sp<updatectrl> nwi();
	mws_sp<updatectrl> getInst();

	int update();

	// get notifications about the current game state
	void started();
	void stopped();
	void paused();
	void resumed();

	// number of updates since started() was called
	int getTickCount();
	// the number of milliseconds between each game update
	void setTimeStepDuration(int iupdateInterval);
	int getTimeStepDuration();
	// time since game started (in ms)
	int getTime();

	// reset game time / update count
	void resetTime();

private:

	updatectrl();

	// the number of milliseconds between each game update
	int updateInterval;

	// internal timekeeping
	uint32_t startTime;
	uint32_t pauseTime;
	uint32_t pauseStartTime;
	// current game state
	updatestatetype state;

	// number of updates since the game started
	int tickCount;
};
