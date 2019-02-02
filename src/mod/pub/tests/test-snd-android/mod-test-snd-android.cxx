#include "stdafx.hxx"

#include "mod-test-snd-android.hxx"

#ifdef MOD_TEST_SND_ANDROID

#include "com/mws/mws-camera.hxx"
#include "com/mws/mws-com.hxx"
#include "snd.hxx"
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
		main_page(mws_sp<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();

			player1 = std::make_shared<guitar_note_player>();
			player2 = std::make_shared<guitar_note_player>();

			struct b_toggle_ui : mws_button
			{
				b_toggle_ui(mws_sp<mws_page> ip, mws_sp<guitar_note_player> ipl) : p(ipl), snd_is_playing(false), mws_button(ip) {}

				virtual void init(mws_rect i_mwsr, int icolor)
				{
					mws_button::init(i_mwsr, icolor, "");
					update_visibility();
				}

				virtual void on_click()
				{
					snd_is_playing = !snd_is_playing;
					update_visibility();

                    if(snd_is_playing)
                    {
						p.lock()->play();
                    }
                    else
                    {
						p.lock()->stop();
                    }
				}

				void update_visibility()
				{
					if (snd_is_playing)
					{
						set_text("stop");
					}
					else
					{
						set_text("start");
					}
				}

				mws_wp<guitar_note_player> p;
				bool snd_is_playing;
			};

			int w = 300;
			int h = 200;
			mws_rect toggle_ui_rect1 = mws_rect(100, get_mod()->get_height() - h - 100, w, h);
			auto b1 = mws_button::new_shared_instance(new b_toggle_ui(get_mws_page_instance(), player1));
			mws_sp<b_toggle_ui> bui1 = std::static_pointer_cast<b_toggle_ui>(b1);
			bui1->init(toggle_ui_rect1, 0x8200b4);

			mws_rect toggle_ui_rect2 = mws_rect(500, get_mod()->get_height() - h - 100, w, h);
			auto b2 = mws_button::new_shared_instance(new b_toggle_ui(get_mws_page_instance(), player2));
			mws_sp<b_toggle_ui> bui2 = std::static_pointer_cast<b_toggle_ui>(b2);
			bui2->init(toggle_ui_rect2, 0x8200b4);

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

		virtual void update_view(mws_sp<mws_camera> g)
		{
			mws_page::update_view(g);

			const std::string& text = get_mod()->get_name();

			g->drawText(text, 10, 20);
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

#endif
