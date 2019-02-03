#pragma once

#include "pfm-def.h"
#include <memory>
#include <string>
#include <vector>


class snd_src;
class snd_dst;
class snd_tf;
class musical_note;


class snd
{
public:
	static bool is_init();
	static void init();
	static void update();

private:
	snd();

	static mws_sp<snd> inst;
};


// microphones, audio files, http streaming, generators, etc.
class snd_src
{
public:
	virtual void link(mws_sp<snd_dst> idst) = 0;
	virtual void link(mws_sp<snd_tf> idst) = 0;
};


// speakers, audio files, etc.
class snd_dst
{
public:
};


// any transformations done to a stream of audio: boost volume, add reverb, etc.
class snd_tf
{
public:
	virtual void link(mws_sp<snd_dst> idst) = 0;
	virtual void link(mws_sp<snd_tf> idst) = 0;
};


// play from a sound source.
class snd_player
{
public:
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;
};


class guitar_note_player_impl;
class guitar_note_player : public snd_player
{
public:
	guitar_note_player();
	virtual ~guitar_note_player();

	void play() override;
	void pause() override;
	void resume() override;
	void stop() override;
	void set_note(mws_sp<musical_note> inote);

private:
	mws_sp<guitar_note_player_impl> p;
};


// a pure musical tone.
class musical_note
{
public:
	static const int NUMBER_OF_NOTES = 12;
	static const int NUMBER_OF_OCTAVES = 9;

	enum note_ids
	{
		c_note, c_sharp, d_note, d_sharp, e_note, f_note, f_sharp, g_note, g_sharp, a_note, a_sharp, b_note,
	};

	static mws_sp<musical_note> get_note(note_ids inote, int ioctave);
	static float get_pitch(note_ids inote, int ioctave);
	std::string get_name();
	std::string get_full_name();
	float get_pitch();
	note_ids get_note_id();
	int get_octave();
	bool equal_to(mws_sp<musical_note> inote);
	bool lower_than(mws_sp<musical_note> inote);
	bool higher_than(mws_sp<musical_note> inote);
	mws_sp<musical_note> next_note();
	mws_sp<musical_note> prev_note();
	// get a lower note offset by itone_count tones from this note.
	mws_sp<musical_note> get_lower_note(int itone_count);
	// get a higher note offset by itone_count tones from this note.
	mws_sp<musical_note> get_higher_note(int itone_count);
	// ex 2 tones between A4 and B4, 1 tone between B4 and C5, etc.
	int tones_between(mws_sp<musical_note> inote);

private:
	musical_note(note_ids inote, int ioctave);

	note_ids note_id;
	uint8 octave;

	static std::vector< std::vector<mws_sp<musical_note> > > note_table;
	static std::vector<note_ids> note_list;
	static std::vector<std::string> note_names;
	static std::vector<float> the_8th_octave_pitches;
};
