#include "stdafx.h"
#include "appplex-conf.hpp"

#if defined MOD_SND && defined PLATFORM_WINDOWS_PC

#include "snd.hpp"
#include "pfm.hpp"
#include "min.hpp"
#include <fmod/fmod.hpp>
#include <fmod/fmod_errors.h>
#pragma comment (lib, "fmodex_vc.lib")

#ifdef MOD_STK

#include <stk/Stk.h>

#endif

FMOD::System* fmodSystem = nullptr;
// FMOD Callbacks
static FMOD_RESULT F_CALLBACK PCMRead32(FMOD_SOUND *sound, void *data, unsigned int length);
static FMOD_RESULT F_CALLBACK PCMSetPosition(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype);

// this is an error handling function for FMOD errors
void fmod_error_check(FMOD_RESULT result)
{
   if (result != FMOD_OK)
   {
      mws_print("FMOD error! (%d) %s", result, FMOD_ErrorString(result));
   }
}

void init_fmod()
{
   FMOD_RESULT result;
   result = FMOD::System_Create(&fmodSystem);
   fmod_error_check(result);

   result = fmodSystem->init(32, FMOD_INIT_NORMAL, 0);
   fmod_error_check(result);

#ifdef MOD_STK
   stk::Stk::setSampleRate(44100.0);
#endif
}


#ifdef MOD_STK

#include <stk/Guitar.h>
#include <stk/Plucked.h>
#pragma comment (lib, "libstk.lib")


struct snd_context
{
	std::weak_ptr<guitar_note_player_impl> gnp;
};


class guitar_note_player_impl
{
public:
	guitar_note_player_impl()
	{
		set_note(musical_note::get_note(musical_note::a_note, 2));
		open();
	}

	~guitar_note_player_impl()
	{
	}

	bool is_running()
	{
		return false;
	}

	void play()
	{
		stop();
		FMOD_RESULT result = fmodSystem->playSound(FMOD_CHANNEL_FREE, genSound, false, &channel);
		fmod_error_check(result);
	}

	void pause()
	{
		stop();
	}

	void resume()
	{
	}

	void stop()
	{
		if (channel)
		{
			channel->stop();
			channel = nullptr;
		}

		guitar_inst.clear();
		guitar_inst.noteOn(note->get_pitch(), 2.);
		guitar_inst.setLoopGain(0.9999);
	}

	void set_note(std::shared_ptr<musical_note> inote)
	{
		note = inote;
		guitar_inst.noteOn(note->get_pitch(), 2.);
		guitar_inst.setLoopGain(0.9999);
	}

	void open()
	{
		genSound = nullptr;
		channel = nullptr;

		// The sample rate, number of channels and total time in seconds before the gnerated sample repeats
		int sampleRate = 44100;
		int channels = 2;
		int soundLengthSeconds = 10;

		FMOD_CREATESOUNDEXINFO soundInfo;

		memset(&soundInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
		soundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);

		// The number of samples to fill per call to the PCM read callback (here we go for 1 second's worth)
		soundInfo.decodebuffersize = sampleRate;

		// The length of the entire sample in bytes, calculated as:
		// Sample rate * number of channels * bits per sample per channel * number of seconds
		soundInfo.length = sampleRate * channels * sizeof(signed short) * soundLengthSeconds;

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
		soundInfo.userdata = &ctx;

		// Create a user-defined sound with FMOD_OPENUSER
		FMOD_RESULT result = fmodSystem->createStream(NULL, FMOD_OPENUSER, &soundInfo, &genSound);
		fmod_error_check(result);
	}

	snd_context ctx;
	stk::Guitar guitar_inst;
	stk::StkFrames me_frm;
	std::shared_ptr<musical_note> note;
	FMOD::Sound* genSound;
	FMOD::Channel* channel;
};

FMOD_RESULT F_CALLBACK PCMRead32(FMOD_SOUND *sound, void *data, unsigned int length)
{
	//mws_print("PCMRead %d\n", length);
	// Get the object we are using
	// Note that FMOD_Sound * must be cast to FMOD::Sound * to access it
	snd_context *ctx = nullptr;
	((FMOD::Sound *) sound)->getUserData((void **)&ctx);
	std::shared_ptr<guitar_note_player_impl> gnp = ctx ? ctx->gnp.lock() : nullptr;

	if (gnp)
	{
		// Get buffer in 32-bit format
		float* stereo32BitBuffer = (float*)data;
		// A 2-channel 32-bit stereo stream uses 8 bytes per sample
		int sample_count = length / 8;
		stk::StkFrames& frm = gnp->me_frm;

		frm.resize(sample_count, 2, 0);

		stk::StkFrames& frm2 = gnp->guitar_inst.tick(frm, 0);
		stk::StkFloat& src = frm2[0];
		stk::StkFloat* srcp = &src;
		int size = frm2.size() * sizeof(stk::StkFloat);

		mws_assert(size == length);
		memcpy(stereo32BitBuffer, srcp, size);
	}

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK PCMSetPosition(FMOD_SOUND *sound, int subsound, unsigned int position, FMOD_TIMEUNIT postype)
{
	// If you need to process the user changing the playback position (seeking), do it here

	return FMOD_OK;
}


guitar_note_player::guitar_note_player()
{
	p = std::make_shared<guitar_note_player_impl>();
	p->ctx.gnp = p;
}

guitar_note_player::~guitar_note_player() {}

void guitar_note_player::play()
{
	p->play();
}

void guitar_note_player::pause()
{
	p->pause();
}

void guitar_note_player::resume()
{
	p->resume();
}

void guitar_note_player::stop()
{
	p->stop();
}

void guitar_note_player::set_note(std::shared_ptr<musical_note> inote)
{
	p->set_note(inote);
}

#endif // MOD_STK


// snd code
std::shared_ptr<snd> snd::inst;

snd::snd()
{
}

bool snd::is_init()
{
	return inst != nullptr;
}

void snd::init()
{
	if (!is_init())
	{
		inst = std::shared_ptr<snd>(new snd());
		init_fmod();
	}
	else
	{
		mws_print("gl_ctrl::init: this method must only be called once\n");

      mws_throw mws_exception("this method must only be called once");
	}
}

void snd::update()
{
	fmodSystem->update();
}

#endif // MOD_SND
