#include "stdafx.hxx"

#include "snd.hxx"
#include "pfm.hxx"


std::vector< std::vector<mws_sp<musical_note> > > musical_note::note_table;
std::vector<musical_note::note_ids> musical_note::note_list = { c_note, c_sharp, d_note, d_sharp, e_note, f_note, f_sharp, g_note, g_sharp, a_note, a_sharp, b_note };
std::vector<std::string> musical_note::note_names = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
std::vector<float> musical_note::the_8th_octave_pitches = { 4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902 };


musical_note::musical_note(note_ids inote, int ioctave)
{
	note_id = inote;
	octave = ioctave;
}

mws_sp<musical_note> musical_note::get_note(note_ids inote, int ioctave)
{
	if (ioctave >= 0 && ioctave < NUMBER_OF_OCTAVES);
	{
		// init table
		if (note_table.empty())
		{
			note_table.resize(NUMBER_OF_OCTAVES);

			for (int k = 0; k < NUMBER_OF_OCTAVES; k++)
			{
				std::vector<mws_sp<musical_note> >& octave_notes = note_table[k];
				octave_notes.resize(NUMBER_OF_NOTES);

				for (int i = 0; i < NUMBER_OF_NOTES; i++)
				{
					octave_notes[i] = mws_sp<musical_note>(new musical_note(note_list[i], k));
				}
			}
		}

		return note_table[ioctave][inote];
	}

	return nullptr;
}

float musical_note::get_pitch(note_ids inote, int ioctave)
{
	float pitch = the_8th_octave_pitches[inote];
	int div = 1 << (NUMBER_OF_OCTAVES - ioctave - 1);

	pitch = pitch / div;

	return pitch;
}

std::string musical_note::get_name()
{
	return note_names[note_id];
}

std::string musical_note::get_full_name()
{
	std::string name = trs("{0}[{1}]", note_names[note_id], (int)octave);
	return name;
}

float musical_note::get_pitch()
{
	return get_pitch(note_id, octave);
}

musical_note::note_ids musical_note::get_note_id()
{
	return note_id;
}

int musical_note::get_octave()
{
	return octave;
}

bool musical_note::equal_to(mws_sp<musical_note> inote)
{
	return note_id == inote->note_id && octave == inote->octave;
}

bool musical_note::lower_than(mws_sp<musical_note> inote)
{
	if (octave < inote->octave)
	{
		return true;
	}
	else if (octave == inote->octave)
	{
		return note_id < inote->note_id;
	}

	return false;
}

bool musical_note::higher_than(mws_sp<musical_note> inote)
{
	return !equal_to(inote) && !lower_than(inote);
}

mws_sp<musical_note> musical_note::next_note()
{
	int t_octave = octave;
	note_ids t_note = note_id;

	if (t_note == b_note)
	{
		if (t_octave < NUMBER_OF_OCTAVES - 1)
		{
			t_note = c_note;
			t_octave++;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		t_note = note_list[t_note + 1];
	}

	return musical_note::get_note(t_note, t_octave);
}

mws_sp<musical_note> musical_note::prev_note()
{
	int t_octave = octave;
	note_ids t_note = note_id;

	if (note_id == c_note)
	{
		if (t_octave > 0)
		{
			t_note = b_note;
			t_octave--;
		}
		else
		{
			return nullptr;
		}
	}
	else
	{
		t_note = note_list[note_id - 1];
	}

	return musical_note::get_note(t_note, t_octave);
}

mws_sp<musical_note> musical_note::get_lower_note(int itone_count)
{
	mws_sp<musical_note> nr = get_note(note_id, octave);

	for (int k = 0; k < itone_count && nr; k++)
	{
		nr = nr->prev_note();
	}

	return nr;
}

mws_sp<musical_note> musical_note::get_higher_note(int itone_count)
{
	mws_sp<musical_note> nr = get_note(note_id, octave);

	for (int k = 0; k < itone_count && nr; k++)
	{
		nr = nr->next_note();
	}

	return nr;
}

int musical_note::tones_between(mws_sp<musical_note> inote)
{
	int delta = 0;

	if (lower_than(inote))
	{
		auto nr = inote;

		do
		{
			nr = nr->prev_note();
			delta++;
		} while (!equal_to(nr));
	}
	else if (!equal_to(inote))
	{
		auto nr = inote;

		do
		{
			nr = nr->next_note();
			delta++;
		} while (!equal_to(nr));
	}

	return delta;
}
