#pragma once

#include "pfm.hpp"
#include "min.hpp"
#include <string>
#include <vector>


class transition_evt : public iadp
{
public:
	enum transition_evt_types
	{
		TR_STARTED,
		TR_STOPPED,
		TR_PAUSED,
		TR_RESUMED,
		TR_FINISHED,
		TR_UPDATE,
		TR_INTERVAL_CHANGED,
	};

	static const std::string TRANSITION_EVT_TYPE;
	static const std::string TRANSITION_STARTED;
	static const std::string TRANSITION_STOPPED;
	static const std::string TRANSITION_PAUSED;
	static const std::string TRANSITION_RESUMED;
	static const std::string TRANSITION_FINISHED;
	static const std::string TRANSITION_UPDATE;
	static const std::string TRANSITION_INTERVAL_CHANGED;

	static shared_ptr<transition_evt> new_instance(transition_evt_types itype);
	static const std::string& get_type_name(transition_evt_types itype);
	transition_evt_types get_type();

protected:
	transition_evt(transition_evt_types itype);

private:
	void set_type(transition_evt_types itype);

	transition_evt_types type;
};


class transition
	// basic transition, in milliseconds
{
public:
	enum transition_type
	{
		basic_transition,
		multi_step_transition,
	};

	transition();

	virtual transition_type get_transition_type();
	bool is_finished();
	bool is_paused();
	virtual void start();
	virtual void stop();
	virtual void pause();
	virtual void resume();
	virtual void update();

protected:
   uint32 start_time;
   uint32 pause_time;
   uint32 pause_duration;
	bool finished;
	bool paused;
};


class ms_linear_transition;

class linear_transition
	// single step fixed-time transition, in milliseconds
{
public:
	enum transition_type
	{
		simple_transition,
		multi_step_transition,
	};

	linear_transition(int durationms);

	virtual transition_type get_transition_type()const;
	bool is_finished()const;
	bool is_paused()const;
	virtual int get_elapsed_time()const;
	virtual int get_duration()const;
	virtual float get_position()const;
	virtual void start();
	virtual void stop();
	virtual void pause();
	virtual void resume();
	virtual void update();

protected:
	friend class ms_linear_transition;
	virtual void reset();
	virtual void start(uint32 offset);

	int duration;
	uint32 start_time;
   uint32 pause_time;
   uint32 pause_duration;
	float position;
	bool finished;
	bool paused;
};


class ms_transition_data;

class ms_linear_transition : public linear_transition, public ia_broadcaster, public enable_shared_from_this<ms_linear_transition>
	// multi-step fixed-time transition, in milliseconds
{
public:
	static shared_ptr<ms_linear_transition> new_instance(shared_ptr<ms_transition_data> td);
	shared_ptr<ms_linear_transition> get_instance();

	transition_type get_transition_type()const;
	int get_interval_index()const;
	int length()const;
	const shared_ptr<linear_transition> get_transition_at(int index)const;

	void start();
	void stop();
	void pause();
	void resume();
	void update();

protected:
	ms_linear_transition();

	virtual shared_ptr<ia_sender> sender_inst();
	void reset();

	int interval_idx;
	std::vector<shared_ptr<linear_transition> > transitions;
};


class ms_transition_data
	// transition data, in milliseconds
{
public:
	// transition data
	static shared_ptr<ms_transition_data> new_transition_data(const std::vector<shared_ptr<linear_transition> >& itransitions);
	// position data [500, 2500, 10500, 14500, 15000]
	static shared_ptr<ms_transition_data> new_position_data(const std::vector<int>& tposition);
	static shared_ptr<ms_transition_data> new_position_data(const int tposition[], int tlength);
	// duration data [500, 2000, 8000, 4000, 500]
	static shared_ptr<ms_transition_data> new_duration_data(const std::vector<int>& tduration);
	static shared_ptr<ms_transition_data> new_duration_data(const int tduration[], int tlength);

private:
	friend class ms_linear_transition;
	ms_transition_data();

	std::vector<shared_ptr<linear_transition> > transitions;
};
