#include "stdafx.h"

#include "transitions.hpp"

using std::string;
using std::vector;


const string transition_evt::TRANSITION_EVT_TYPE					= "tr-";
const string transition_evt::TRANSITION_STARTED						= "tr-started";
const string transition_evt::TRANSITION_STOPPED						= "tr-stopped";
const string transition_evt::TRANSITION_PAUSED						= "tr-paused";
const string transition_evt::TRANSITION_RESUMED						= "tr-resumed";
const string transition_evt::TRANSITION_FINISHED					= "tr-finished";
const string transition_evt::TRANSITION_UPDATE						= "tr-update";
const string transition_evt::TRANSITION_INTERVAL_CHANGED			= "tr-interval-changed";


transition_evt::transition_evt(transition_evt_types itype) : mws_dp(get_type_name(itype))
{
	set_type(itype);
}

shared_ptr<transition_evt> transition_evt::nwi(transition_evt_types itype)
{
	return shared_ptr<transition_evt>(new transition_evt(itype));
}

const std::string& transition_evt::get_type_name(transition_evt::transition_evt_types itype)
{
	static const std::string types[] =
	{
		TRANSITION_STARTED,
		TRANSITION_STOPPED,
		TRANSITION_PAUSED,
		TRANSITION_RESUMED,
		TRANSITION_FINISHED,
		TRANSITION_UPDATE,
		TRANSITION_INTERVAL_CHANGED,
	};

	return types[itype];
}

transition_evt::transition_evt_types transition_evt::get_type()
{
	return type;
}

void transition_evt::set_type(transition_evt_types itype)
{
	type = itype;
}

	
linear_transition::linear_transition(int durationms)
{
	if(durationms <= 0)
	{
      mws_throw mws_exception("duration must be greater than 0");
	}

	reset();
	duration = durationms;
	finished = true;
	paused = false;
}

linear_transition::transition_type linear_transition::get_transition_type()const
{
	return simple_transition;
}

bool linear_transition::is_finished()const
{
	return finished;
}

bool linear_transition::is_paused()const
{
	return paused;
}

int linear_transition::get_elapsed_time()const
{
	uint32 delta = pfm::time::get_time_millis() - start_time - pause_duration;

	return delta;
}

int linear_transition::get_duration()const
{
	return duration;
}

float linear_transition::get_position()const
{
	return position;
}

void linear_transition::start()
{
	start(pfm::time::get_time_millis());
}

void linear_transition::stop()
{
	finished = true;
	paused = false;
}

void linear_transition::pause()
{
	if(paused || finished)
	{
      mws_throw mws_exception();//trs("transition cannot be paused. invalid state [paused %1%, finished %2%]") % paused % finished);
	}

	pause_time = pfm::time::get_time_millis();
	paused = true;
}

void linear_transition::resume()
{
	if(!paused || finished)
	{
      mws_throw mws_exception();//trs("transition cannot be resumed. invalid state [paused %1%, finished %2%]") % paused % finished);
	}

	pause_duration += pfm::time::get_time_millis() - pause_time;
	paused = false;
}

void linear_transition::update()
{
	if(!finished && !paused)
	{
		int tms = get_elapsed_time();

		if(tms <= duration)
		{
			position = tms / float(duration);
		}
		else
		{
			position = 1;
			finished = true;
		}
	}
}

void linear_transition::reset()
{
	start_time = pfm::time::get_time_millis();
	pause_time = pfm::time::get_time_millis();
	pause_duration = pfm::time::get_time_millis();
	position = 0;
	finished = true;
	paused = false;
}

void linear_transition::start(uint32 offset)
{
	reset();
	start_time = pfm::time::get_time_millis() - offset;
	finished = false;
	paused = false;
}


ms_linear_transition::ms_linear_transition() : linear_transition(1){}

shared_ptr<ms_linear_transition> ms_linear_transition::nwi(shared_ptr<ms_transition_data> td)
{
	shared_ptr<ms_linear_transition> mslt(new ms_linear_transition());

	mslt->duration = 0;

	for(auto lt : td->transitions)
	{
		mslt->transitions.push_back(lt);
		mslt->duration += lt->get_duration();
	}

	return mslt;
}

shared_ptr<ms_linear_transition> ms_linear_transition::get_instance()
{
	return shared_from_this();
}

ms_linear_transition::transition_type ms_linear_transition::get_transition_type()const
{
	return multi_step_transition;
}

int ms_linear_transition::get_interval_index()const
{
	return interval_idx;
}

int ms_linear_transition::length()const
{
	return transitions.size();
}

const shared_ptr<linear_transition> ms_linear_transition::get_transition_at(int index)const
{
	return transitions[index];
}

void ms_linear_transition::start()
{
	linear_transition::start();
	transitions[interval_idx]->start();
	broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_STARTED));
}

void ms_linear_transition::stop()
{
	linear_transition::stop();
	transitions[interval_idx]->stop();
	broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_STOPPED));
}

void ms_linear_transition::pause()
{
	linear_transition::pause();
	transitions[interval_idx]->pause();
	broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_PAUSED));
}

void ms_linear_transition::resume()
{
	linear_transition::resume();
	transitions[interval_idx]->resume();
	broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_RESUMED));
}

void ms_linear_transition::update()
{
	if(finished || paused)
	{
		return;
	}

	int tms = get_elapsed_time();
	shared_ptr<linear_transition> tr = transitions[interval_idx];

	tr->update();

	if(tr->is_finished())
		// current transition finished. move to the next
	{
		if(interval_idx < transitions.size() - 1)
			// next transition exists. start it and call update
		{
			int td = 0;
			interval_idx++;
			tr = transitions[interval_idx];

			for (int k = 0; k < interval_idx; k++)
			{
				td += transitions[k]->get_duration();
			}

			tr->start(tms - td);
			tr->update();
		}
		else
			// last transition finished
		{
			finished = true;
		}
	}

	if(finished)
	{
		position = 1;
		broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_FINISHED));
	}
	else
	{
		position = tms / float(duration);
		broadcast(get_instance(), transition_evt::nwi(transition_evt::TR_UPDATE));
	}
}

shared_ptr<mws_sender> ms_linear_transition::sender_inst()
{
	return get_instance();
}

void ms_linear_transition::reset()
{
	linear_transition::reset();
	interval_idx = 0;
}


ms_transition_data::ms_transition_data(){}

shared_ptr<ms_transition_data> ms_transition_data::new_transition_data(const vector<shared_ptr<linear_transition> >& itransitions)
{
	shared_ptr<ms_transition_data> td(new ms_transition_data());

	for(auto t : itransitions)
	{
		td->transitions.push_back(t);
	}

	return td;
}

shared_ptr<ms_transition_data> ms_transition_data::new_position_data(const vector<int>& tposition)
{
	shared_ptr<ms_transition_data> td(new ms_transition_data());
	int deltat = 0;

	for(int position : tposition)
	{
		shared_ptr<linear_transition> lt(new linear_transition(position - deltat));

		td->transitions.push_back(lt);
		deltat = position;
	}

	return td;
}

shared_ptr<ms_transition_data> ms_transition_data::new_position_data(const int tposition[], int tlength)
{
	shared_ptr<ms_transition_data> td(new ms_transition_data());
	int deltat = 0;

	for(int k = 0; k < tlength; k++)
	{
		int position = tposition[k];
		shared_ptr<linear_transition> lt(new linear_transition(position - deltat));

		td->transitions.push_back(lt);
		deltat = position;
	}

	return td;
}

shared_ptr<ms_transition_data> ms_transition_data::new_duration_data(const vector<int>& tduration)
{
	shared_ptr<ms_transition_data> td(new ms_transition_data());

	for(int duration : tduration)
	{
		shared_ptr<linear_transition> lt(new linear_transition(duration));

		td->transitions.push_back(lt);
	}

	return td;
}

shared_ptr<ms_transition_data> ms_transition_data::new_duration_data(const int tduration[], int tlength)
{
	shared_ptr<ms_transition_data> td(new ms_transition_data());

	for(int k = 0; k < tlength; k++)
	{
		shared_ptr<linear_transition> lt(new linear_transition(tduration[k]));

		td->transitions.push_back(lt);
	}

	return td;
}
