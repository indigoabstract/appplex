#include "stdafx.hxx"
#include "appplex-conf.hxx"

#if defined PLATFORM_ANDROID

#include "snd.hxx"
#include "pfm.hxx"
#include "min.hxx"
#include "main.hxx"
#include <snd/android/opensl-stream.h>
#include <jni.h>

#ifdef MOD_STK
#include <stk/Guitar.h>
#include <stk/Plucked.h>
#endif


void android_main::snd_init(int isample_rate, int ibuffer_size)
{
	sample_rate = isample_rate;
	buffer_size = ibuffer_size;
#ifdef MOD_STK
	stk::Stk::setSampleRate(sample_rate);
#endif
}

void android_main::snd_close()
{
	//opensl_close(ctx.os);
}


#ifdef MOD_STK

// Audio processing callback; this is the heart and soul of this file.
static void process(
	void *context, int sample_rate, int buffer_frames, int input_channels, const short *input_buffer, int output_channels, short *output_buffer);

struct snd_context
{
	OPENSL_STREAM *os;
	std::weak_ptr<guitar_note_player_impl> gnp;
};


class guitar_note_player_impl
{
public:
	guitar_note_player_impl()
	{
		set_note(musical_note::get_note(musical_note::a_note, 2));
	}

	~guitar_note_player_impl()
	{
		if (ctx.os)
		{
			opensl_close(ctx.os);
		}
	}

	bool is_running()
	{
		if (ctx.os)
		{
			return opensl_is_running(ctx.os) == 0 ? false : true;
		}

		return false;
	}

	void play()
	{
		if (!ctx.os)
		{
			open();
		}

		if (ctx.os)
		{
			guitar_inst.clear();
			guitar_inst.noteOn(note->get_pitch(), 2.);
			//guitar_inst.noteOn(musical_note::get_note(musical_note::a_note, 2)->get_pitch(), 2.);
			guitar_inst.setLoopGain(0.9965);

			int result = opensl_start(ctx.os);
		}
	}

	void pause()
	{
		if (ctx.os)
		{
			opensl_pause(ctx.os);
		}
	}

	void resume()
	{
		if (ctx.os)
		{
		}
	}

	void stop()
	{
		if (ctx.os)
		{
			opensl_pause(ctx.os);
		}
	}

	void set_note(std::shared_ptr<musical_note> inote)
	{
		note = inote;
		guitar_inst.noteOn(note->get_pitch(), 2.);
	}

	void open()
	{
		if (!ctx.os)
		{
			int sample_rate = android_main::get_instance()->sample_rate;
			int buffer_size = android_main::get_instance()->buffer_size;

			ctx.os = opensl_open(sample_rate, 1, 2, buffer_size, process, &ctx);
		}
	}

	snd_context ctx;
	stk::Guitar guitar_inst;
	stk::StkFrames me_frm;
	std::shared_ptr<musical_note> note;
};


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


// Audio processing callback; this is the heart and soul of this file.
void process(
	void *context, int sample_rate, int buffer_frames, int input_channels, const short *input_buffer, int output_channels, short *output_buffer)
{
	if (input_channels > 0)
	{
		snd_context *lp = (snd_context *)context;
		auto gnp = lp->gnp.lock();

		// Get buffer in 32-bit format
		//float* stereo32BitBuffer = (float*)data;
		int length = buffer_frames * output_channels * sizeof(short);
		// A 2-channel 32-bit stereo stream uses 8 bytes per sample
		int sample_count = length / (sizeof(short) * output_channels);
		stk::StkFrames& frm = gnp->me_frm;

		frm.resize(sample_count, output_channels, 0);

		stk::StkFrames& frm2 = gnp->guitar_inst.tick(frm, 0);
		stk::StkFloat& src = frm2[0];
		stk::StkFloat* srcp = &src;
		//int size = frm2.size() * sizeof(stk::StkFloat);

		for (int k = 0; k < frm2.size(); k++)
		{
			output_buffer[k] = short(srcp[k] * 32767);
		}
	}
}

#endif

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
	}
	else
	{
		vprint("gl_ctrl::init: this method must only be called once\n");

		throw ia_exception("this method must only be called once");
	}
}

void snd::update()
{
}

#else

#include "main.hxx"

void android_main::snd_init(int isample_rate, int ibuffer_size)
{
	sample_rate = isample_rate;
	buffer_size = ibuffer_size;
}

void android_main::snd_close()
{
}

#endif // PLATFORM_ANDROID
