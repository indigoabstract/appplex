#include "stdafx.hxx"

#include "mod-test-snd-android.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "snd/snd.hxx"
//#include <audio/android/opensl_stream.h>
//#include <audio/android/lowpass.h>
#include <stk/Guitar.h>
#include <stk/Plucked.h>


mod_test_snd_android::mod_test_snd_android() : mws_mod(mws_stringify(MOD_TEST_SND_ANDROID)) {}

mws_sp<mod_test_snd_android> mod_test_snd_android::nwi()
{
	return mws_sp<mod_test_snd_android>(new mod_test_snd_android());
}

void mod_test_snd_android::init()
{
}


namespace mod_test_snd_android_ns
{
	class main_page : public mws_page
	{
	public:
		virtual void init()
		{
			mws_page::init();

			player1 = std::make_shared<guitar_note_player>();
			player2 = std::make_shared<guitar_note_player>();
			auto update_visibility = [](mws_sp<mws_button> i_btn, bool i_snd_is_playing)
			{
				if (i_snd_is_playing)
				{
					i_btn->set_text("stop");
				}
				else
				{
					i_btn->set_text("start");
				}
			};

			int w = 300;
			int h = 200;
			{
				auto b = mws_button::nwi();
				b->set_rect(mws_rect(100, get_mod()->get_height() - h - 100, w, h));
				b->set_text("start");
				b->on_click = [b, update_visibility, player = player1, snd_is_playing = false]() mutable
				{
					snd_is_playing = !snd_is_playing;
					update_visibility(b, snd_is_playing);

					if (snd_is_playing)
					{
						player->play();
					}
					else
					{
						player->stop();
					}
				};
				attach(b);
			}
			{
				auto b = mws_button::nwi();
				b->set_rect(mws_rect(500, get_mod()->get_height() - h - 100, w, h));
				b->set_text("start");
				b->on_click = [b, update_visibility, player = player2, snd_is_playing = false]() mutable
				{
					snd_is_playing = !snd_is_playing;
					update_visibility(b, snd_is_playing);

					if (snd_is_playing)
					{
						player->play();
					}
					else
					{
						player->stop();
					}
				};
				attach(b);
			}
			player1->set_note(musical_note::get_note(musical_note::a_note, 2));
			player2->set_note(musical_note::get_note(musical_note::g_note, 3));
		}

		virtual void receive(mws_sp<mws_dp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(mws_sp<mws_camera> i_g)
		{
			mws_page::update_view(i_g);

			const std::string& text = get_mod()->name();

			i_g->drawText(text, 10, 20);
		}

		mws_sp<guitar_note_player> player1;
		mws_sp<guitar_note_player> player2;
	};
}


void mod_test_snd_android::init_mws()
{
   mws_root->new_page<mod_test_snd_android_ns::main_page>();
	mws_cam->clear_color = true;
}

void mod_test_snd_android::load()
{
}
