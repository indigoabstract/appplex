#include "stdafx.hxx"

#include "mod-test-snd-generators.hxx"
#include "snd/snd.hxx"
#include "mws/mws-camera.hxx"
#include "fonts/mws-font.hxx"
#include "mws/mws-com.hxx"
#include "gfx-color.hxx"
#include "rng/rng.hxx"
#include <stk/Guitar.h>
#include <stk/Plucked.h>
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#include <glm/inc.hpp>

#pragma comment (lib, "libstk.lib")
#pragma comment (lib, "fmodex_vc.lib")


namespace mod_test_snd_generators_ns
{
	// this is an error handling function for FMOD errors
	void fmod_error_check(FMOD_RESULT result)
	{
		if (result != FMOD_OK)
		{
			mws_print("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
		}
	}


	// Some example sound generators
	// These receive a value from 0.0-1.0 indicating the position in the repeating samples to generate
	// and return a value from -1.0 to +1.0 indicating the volume at the current sample position
	using GeneratorType = std::function < double(double) > ;

	class Generators
	{
	public:
		// Sine wave
		static double Sine(double samplePos)
		{
			return sin(samplePos * glm::pi<float>() * 2);
		}

		// Sawtooth
		static double Sawtooth(double samplePos)
		{
			if (samplePos == 0)
				return 0.f;

			return (2.f * samplePos) - 1.f;
		}

		// Square wave
		static double Square(double samplePos)
		{
			return (samplePos < 0.5f ? 1.f : -1.f);
		}

		// White noise ("static")
		static double WhiteNoise(double samplePos)
		{
			return static_cast<double>(rand()) / RAND_MAX;
		}
	};


	// Class which generates audio according to the specified function, frequency, sample rate and volume
	class Generator
	{
	private:
		// The sound
		//Song sound;

		// Sample rate
		int sampleRate;

		// Number of channels
		int channels;

		// Length of sample in seconds before it repeats
		int lengthInSeconds;

		// Frequency to generate
		float frequency;

		// Volume (0.0-1.0)
		float volume;

		// The function to use to generate samples
		GeneratorType generator;

		// How many samples we have generated so far
		int samplesElapsed;
		stk::Guitar guitar_inst;
		//stk::Plucked guitar_inst;
		stk::StkFrames frm;

	public:
		// Constructor
		Generator(FMOD::System* ifmod, GeneratorType generator, double frequency, int sampleRate, int channels, int lengthInSeconds, float volume)
			: fmod(ifmod), generator(generator), frequency(frequency), sampleRate(sampleRate), channels(channels),
			lengthInSeconds(lengthInSeconds), volume(volume), samplesElapsed(0)
		{
			FMOD_CREATESOUNDEXINFO soundInfo;

			memset(&soundInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
			soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

			// The number of samples to fill per call to the PCM read callback (here we go for 1 second's worth)
			soundInfo.decodebuffersize = sampleRate;

			// The length of the entire sample in bytes, calculated as:
			// Sample rate * number of channels * bits per sample per channel * number of seconds
			soundInfo.length = sampleRate * channels * sizeof(signed short) * lengthInSeconds;

			// Number of channels and sample rate
			soundInfo.numchannels = channels;
			soundInfo.defaultfrequency = sampleRate;

			// The sound format (here we use 16-bit signed PCM)
			//soundInfo.format = FMOD_SOUND_FORMAT_PCM16;
			soundInfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;

			// Callback for generating new samples
			soundInfo.pcmreadcallback = PCMRead32;

			// Callback for when the user seeks in the playback
			soundInfo.pcmsetposcallback = PCMSetPosition;

			// Pointer to the object we are using to generate the samples
			// (our callbacks query this data to access a concrete instance of the class from our static member callback functions)
			soundInfo.userdata = this;

			// Create a user-defined sound with FMOD_OPENUSER
			FMOD_RESULT result = fmod->createStream(NULL, FMOD_OPENUSER, &soundInfo, &genSound);
			fmod_error_check(result);
			stk::Stk::setSampleRate(44100.0);
		}

		// Start playing
		FMOD::Channel* Start()
		{
			//guitar_inst.clear();
			guitar_inst.noteOn(frequency, 2.);
			samplesElapsed = 0;
			FMOD_RESULT result = fmod->playSound(FMOD_CHANNEL_FREE, genSound, false, &channel);
			fmod_error_check(result);
			return channel;
		}

		void stop()
		{
			channel->stop();
		}

		// Change the sound being generated
		void SetGenerator(GeneratorType g)
		{
			generator = g;
		}

		void set_frequency(float ifrequency)
		{
			frequency = ifrequency;
			guitar_inst.setLoopGain(0.99);
		}

		FMOD::System* fmod;
		FMOD::Sound* genSound;
		FMOD::Channel* channel;
		// FMOD Callbacks
		static FMOD_RESULT F_CALLBACK PCMRead32(FMOD_SOUND *sound, void *data, unsigned int length);
		static FMOD_RESULT F_CALLBACK PCMSetPosition(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype);
	};

	// Generate new samples
	// We must fill "length" bytes in the buffer provided by "data"
	FMOD_RESULT Generator::PCMRead32(FMOD_SOUND *sound, void *data, unsigned int length)
	{
		//mws_print("PCMRead %d\n", length);
		// Get the object we are using
		// Note that FMOD_Sound * must be cast to FMOD::Sound * to access it
		Generator *me = nullptr;
		((FMOD::Sound *) sound)->getUserData((void **)&me);

		// Get buffer in 32-bit format
		float* stereo32BitBuffer = (float*)data;
		// A 2-channel 32-bit stereo stream uses 8 bytes per sample
		int sample_count = length / 8;
		stk::StkFrames& frm = me->frm;

		frm.resize(sample_count, 2, 0);

		stk::StkFrames& frm2 = me->guitar_inst.tick(frm, 0);
		stk::StkFloat& src = frm2[0];
		stk::StkFloat* srcp = &src;
		int size = frm2.size() * sizeof(stk::StkFloat);

		mws_assert(size == length);
		memcpy(stereo32BitBuffer, srcp, size);

		return FMOD_OK;
	}

	FMOD_RESULT Generator::PCMSetPosition(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
	{
		// If you need to process the user changing the playback position (seeking), do it here

		return FMOD_OK;
	}

	// Types of sound generator
	GeneratorType generators[] =
	{
		Generators::Sine,
		Generators::Sawtooth,
		Generators::Square,
		Generators::WhiteNoise
	};
	int generatorId = 0;

	gfx_color red(gfx_color::from_argb(0xffff0000));
	gfx_color orange(gfx_color::from_argb(0xffff8000));
	gfx_color yellow(gfx_color::from_argb(0xffffff00));
	gfx_color green(gfx_color::from_argb(0xff008000));
	gfx_color blue(gfx_color::from_argb(0xff0000ff));
	gfx_color indigo(gfx_color::from_argb(0xff4b0082));
	gfx_color violet(gfx_color::from_argb(0xff9400d3));
	gfx_color light_steel_blue(gfx_color::from_argb(0xffB0C4DE));
	gfx_color salmon(gfx_color::from_argb(0xfffa8072));
	gfx_color tan(gfx_color::from_argb(0xffd2b48c));
	gfx_color brown(gfx_color::from_argb(0xffa0522d));
	gfx_color light_green(gfx_color::from_argb(0xff7fff00));

	class colored_note
	{
	public:
		colored_note()
		{
			set_note(musical_note::a_note);
		}

		colored_note(musical_note::note_ids inote)
		{
			set_note(inote);
		}

		musical_note::note_ids get_note(){ return note; }

		void set_note(musical_note::note_ids inote)
		{
			note = inote;
			color = get_note_color(note);
		}

		gfx_color get_color(){ return color; }

		colored_note get_prev_note()
		{
			colored_note cn;
			musical_note::note_ids nt = note;

			if (note == musical_note::c_note)
			{
				nt = musical_note::b_note;
				cn.set_note(nt);
			}
			else
			{
				nt = musical_note::note_ids(note - 1);
				cn.set_note(nt);
			}

			return cn;
		}

		colored_note get_next_note()
		{
			colored_note cn;
			musical_note::note_ids nt = note;

			if (note == musical_note::b_note)
			{
				nt = musical_note::c_note;
				cn.set_note(nt);
			}
			else
			{
				nt = musical_note::note_ids(note + 1);
				cn.set_note(nt);
			}

			return cn;
		}

		static gfx_color get_note_color(musical_note::note_ids inote)
		{
			switch (inote)
			{
			case musical_note::c_note:
				return yellow;

			case musical_note::c_sharp:
				return salmon;

			case musical_note::d_note:
				return green;

			case musical_note::d_sharp:
				return tan;

			case musical_note::e_note:
				return blue;

			case musical_note::f_note:
				return indigo;

			case musical_note::f_sharp:
				return light_steel_blue;

			case musical_note::g_note:
				return violet;

			case musical_note::g_sharp:
				return light_green;

			case musical_note::a_note:
				return red;

			case musical_note::a_sharp:
				return brown;

			case musical_note::b_note:
				return orange;
			}

         mws_throw mws_exception("illegal value for note");
		}

	private:
		musical_note::note_ids note;
		gfx_color color;
	};


	class main_page : public mws_page
	{
	public:
		std::vector<std::vector<colored_note> > note_tab;

		virtual void init()
		{
			mws_page::init();

			tuning =
			{
				musical_note::get_note(musical_note::e_note, 4), musical_note::get_note(musical_note::b_note, 3),
				musical_note::get_note(musical_note::g_note, 3), musical_note::get_note(musical_note::d_note, 3),
				musical_note::get_note(musical_note::a_note, 2), musical_note::get_note(musical_note::e_note, 2),
			};
			note_tab.resize(6);

			for (int string_idx = 0; string_idx < 6; string_idx++)
			{
				note_tab[string_idx].resize(21);
				note_tab[string_idx][0].set_note(tuning[string_idx]->get_note_id());

				for (int fret_idx = 0; fret_idx < 21 - 1; fret_idx++)
				{
					colored_note cn = note_tab[string_idx][fret_idx].get_next_note();

					note_tab[string_idx][fret_idx + 1] = cn;
				}
			}


			mws_path res_dir = mws::filesys::res_dir();
			mws_path test_fmod_res_dir = res_dir.parent_path().parent_path() / "test-fmod/res/";
			FMOD_RESULT result;
			result = FMOD::System_Create(&fmodSystem);
			fmod_error_check(result);

			result = fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
			fmod_error_check(result);

			mws_path path = test_fmod_res_dir / "01-scent-of-nova.mp3";
			result = fmodSystem->createStream(path.string().c_str(), FMOD_SOFTWARE | FMOD_LOOP_NORMAL, 0, &music);
			fmod_error_check(result);
			mws_path path2 = test_fmod_res_dir / "rm_snd_ijike.wav";
			result = fmodSystem->createSound(path2.string().c_str(), FMOD_SOFTWARE, 0, &actionSound);
			fmod_error_check(result);
			//result = fmodSystem->playSound(FMOD_CHANNEL_FREE, music, false, &musicChannel);
			fmod_error_check(result);
			musicChannel->setVolume(0.75f);
			result = fmodSystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &dspecho);
			fmod_error_check(result);
			//result = musicChannel->addDSP(dspecho, 0);
			fmod_error_check(result);
			result = dspecho->setParameter(FMOD_DSP_ECHO_DELAY, 300.0f);
			fmod_error_check(result);


			// The sample rate, number of channels and total time in seconds before the gnerated sample repeats
			int sampleRate = 44100;
			int channels = 2;
			int soundLengthSeconds = 10;

			// Which generator to use
			int generatorId = 0;
			int numGenerators = 4;

			// Frequency to generate (Hz)
			double frequency = 82.4;

			// Volume level (0.0 - 1.0)
			float volume = 1.0f;

			// Set up sound generator
			generator = std::make_shared<Generator>(fmodSystem, generators[generatorId], frequency, sampleRate, channels, soundLengthSeconds, volume);
			note_font = mws_font::nwi(20.f);
			note_font->set_color(gfx_color::colors::black);
		}

		virtual void receive(mws_sp<mws_dp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(mws_ptr_evt::ptr_evt_type))
			{
			}
			else if (idp->is_type(mws_key_evt::key_evt_type))
			{
				mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(idp);

				if (ke->get_type() == mws_key_evt::ke_pressed)
				{
					switch (ke->get_key())
					{
					case mws_key_i:
					{
						if (last_played_note)
						{
							show_note = trs("last played: {0} {1}", last_played_note->get_full_name(), last_played_note->get_pitch());
							trx(show_note.c_str());
						}
						else
						{
							trx("empty.");
						}
						break;
					}

					case mws_key_g:
					{
						int numGenerators = 4;
						generator->stop();
						generatorId = (generatorId + 1) % numGenerators;
						generator->SetGenerator(generators[generatorId]);
						generator->Start();
						break;
					}

					case mws_key_p:
					{
						// Start playback and get the channel and a reference to the sound
						FMOD::Channel* channel = generator->Start();
						break;
					}

					case mws_key_r:
					{
						auto start_note = musical_note::get_note(musical_note::e_note, 2);
						auto end_note = musical_note::get_note(musical_note::c_note, 6);
						int span = start_note->tones_between(end_note);
						RNG rng;
						int tones = rng.nextInt(span + 1);
						auto nr = start_note->get_higher_note(tones);

						generator->set_frequency(nr->get_pitch());
						//trx("playing %1%") % nr->get_name();
						FMOD::Channel* channel = generator->Start();
						last_played_note = nr;
						break;
					}

					case mws_key_s:
						// stop the sound
						generator->stop();
						break;
					}
				}
			}

			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
			fmodSystem->update();
		}

		virtual void update_view(mws_sp<mws_camera> g)
		{
			mws_page::update_view(g);

			const std::string& text = get_mod()->get_name();

			g->drawText(text, 10, 20);

			std::string notes = "A, A#, B, C, C#, D, D#, E, F, F#, G, G#";

			g->drawText(notes, 10, 50);

			g->set_color(gfx_color::from_argb(0x808080));
			g->drawLine(60, 0, 60, get_mod()->get_height());
			g->drawLine(780, 0, 780, get_mod()->get_height());

			int string_count = tuning.size();

			for (int k = 0; k < string_count; k++)
			{
				auto crt_note = tuning[k];

				for (int j = 0; j < 21; j++)
				{

					gfx_color c = note_tab[k][j].get_color();

					if (last_played_note && last_played_note->equal_to(crt_note))
					{
						gfx_color c2(0xff, 0xff, 0xff);
						g->set_color(c2);
						g->fillRect(10 + j * 60 - 10, 200 + k * 60 - 10, 40 + 20, 20 + 20);
					}

					g->set_color(c);
					g->fillRect(10 + j * 60, 200 + k * 60, 40, 20);
					std::string note_name = (j == 0) ? crt_note->get_full_name() : crt_note->get_name();
					g->drawText(note_name, 10 + j * 60, 200 + k * 60, note_font);
					crt_note = crt_note->next_note();
				}
			}

			g->drawText(show_note, 10, 90);
		}

		FMOD::System* fmodSystem;
		FMOD::Sound* music;
		FMOD::Sound* actionSound;
		FMOD::Channel* musicChannel;
		FMOD::DSP* dspecho;
		mws_sp<Generator> generator;
		mws_sp<musical_note> last_played_note;
		std::string show_note;
		std::vector<mws_sp<musical_note> > tuning;
		mws_sp<mws_font> note_font;
	};
}


mod_test_snd_generators::mod_test_snd_generators() : mws_mod(mws_stringify(MOD_TEST_SND_GENERATORS)) {}

mws_sp<mod_test_snd_generators> mod_test_snd_generators::nwi()
{
	return mws_sp<mod_test_snd_generators>(new mod_test_snd_generators());
}

void mod_test_snd_generators::init()
{
}

void mod_test_snd_generators::init_mws()
{
   mws_root->new_page<mod_test_snd_generators_ns::main_page>();
	mws_cam->clear_color = true;
}

void mod_test_snd_generators::load()
{
}
