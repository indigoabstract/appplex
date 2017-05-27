#include "stdafx.h"

#include "unit-test-stk.hpp"

#ifdef UNIT_TEST_STK

#include "com/ux/ux-camera.hpp"
#include "com/ux/ux.hpp"
#include "snd.hpp"
#include <rng/rng.hpp>
#include <glm/gtc/constants.hpp>
#include <stk/Guitar.h>
#include <stk/Plucked.h>
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>


#pragma comment (lib, "libstk.lib")
#pragma comment (lib, "fmodex_vc.lib")


namespace unit_test_stk_ns
{
	// this is an error handling function for FMOD errors
	void fmod_error_check(FMOD_RESULT result)
	{
		if (result != FMOD_OK)
		{
			vprint("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
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
			guitar_inst.clear();
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
		//vprint("PCMRead %d\n", length);
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

		ia_assert(size == length);
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
}
using namespace unit_test_stk_ns;


unit_test_stk::unit_test_stk()
{
	set_name("test-stk");
}

shared_ptr<unit_test_stk> unit_test_stk::new_instance()
{
	return shared_ptr<unit_test_stk>(new unit_test_stk());
}

void unit_test_stk::init()
{
}

void unit_test_stk::init_ux()
{
	class mainpage : public ux_page
	{
	public:
		mainpage(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			FMOD_RESULT result;
			result = FMOD::System_Create(&fmodSystem);
			fmod_error_check(result);

			result = fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
			fmod_error_check(result);

			std::string path = pfm::filesystem::get_path("test-fmod/01-scent-of-nova.mp3");
			result = fmodSystem->createStream(path.c_str(), FMOD_SOFTWARE | FMOD_LOOP_NORMAL, 0, &music);
			fmod_error_check(result);
			std::string path2 = pfm::filesystem::get_path("test-fmod/rm_snd_ijike.wav");
			result = fmodSystem->createSound(path2.c_str(), FMOD_SOFTWARE, 0, &actionSound);
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
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}

			if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
			{
				shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

				int x = ts->crt_state.te->points[0].x;
				int y = ts->crt_state.te->points[0].y;

				switch (ts->get_type())
				{
				case touch_sym_evt::TS_PRESSED:
					break;

				case touch_sym_evt::TS_RELEASED:
					break;

				}
			}
			else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

				if (ke->get_type() == key_evt::KE_PRESSED)
				{
					switch (ke->get_key())
					{
					case KEY_I:
					{
						if (last_played_note)
						{
							show_note = trs("last played: {0} {1}", last_played_note->get_full_name(), last_played_note->get_pitch());
							trx(show_note);
						}
						else
						{
							trx("empty.");
						}
						break;
					}

					case KEY_G:
					{
						int numGenerators = 4;
						generator->stop();
						generatorId = (generatorId + 1) % numGenerators;
						generator->SetGenerator(generators[generatorId]);
						generator->Start();
						break;
					}

					case KEY_P:
					{
						// Start playback and get the channel and a reference to the sound
						FMOD::Channel* channel = generator->Start();
						break;
					}

					case KEY_R:
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

					case KEY_S:
						// stop the sound
						generator->stop();
						break;
					}
				}
			}

			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			fmodSystem->update();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);

			g->drawText("test stk", 10, 10);
			g->drawText(show_note, 10, 50);
		}

		FMOD::System* fmodSystem;
		FMOD::Sound* music;
		FMOD::Sound* actionSound;
		FMOD::Channel* musicChannel;
		FMOD::DSP* dspecho;
		shared_ptr<Generator> generator;
		shared_ptr<musical_note> last_played_note;
		std::string show_note;
	};

	ux_page::new_shared_instance(new mainpage(uxroot));
	ux_cam->clear_color = true;
}

#endif
