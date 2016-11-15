#include "stdafx.h"

#include "unit-test-snd-android.hpp"

#ifdef UNIT_TEST_SND_ANDROID

#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-com.hpp"
#include "snd.hpp"
//#include <audio/android/opensl_stream.h>
//#include <audio/android/lowpass.h>
#include <stk/Guitar.h>
#include <stk/Plucked.h>


unit_test_snd_android::unit_test_snd_android()
{
	set_name("test-snd-android");
}

shared_ptr<unit_test_snd_android> unit_test_snd_android::new_instance()
{
	return shared_ptr<unit_test_snd_android>(new unit_test_snd_android());
}

void unit_test_snd_android::init()
{
}


namespace unit_test_snd_android_ns
{
	class main_page : public ux_page
	{
	public:
		main_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();

			player1 = std::make_shared<guitar_note_player>();
			player2 = std::make_shared<guitar_note_player>();

			struct b_toggle_ui : ux_button
			{
				b_toggle_ui(shared_ptr<ux_page> ip, std::shared_ptr<guitar_note_player> ipl) : p(ipl), snd_is_playing(false), ux_button(ip) {}

				virtual void init(ux_rect iuxr, int icolor)
				{
					ux_button::init(iuxr, icolor, "");
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

				std::weak_ptr<guitar_note_player> p;
				bool snd_is_playing;
			};

			int w = 300;
			int h = 200;
			ux_rect toggle_ui_rect1 = ux_rect(100, get_unit()->get_height() - h - 100, w, h);
			auto b1 = ux_button::new_shared_instance(new b_toggle_ui(get_ux_page_instance(), player1));
			shared_ptr<b_toggle_ui> bui1 = std::static_pointer_cast<b_toggle_ui>(b1);
			bui1->init(toggle_ui_rect1, 0x8200b4);

			ux_rect toggle_ui_rect2 = ux_rect(500, get_unit()->get_height() - h - 100, w, h);
			auto b2 = ux_button::new_shared_instance(new b_toggle_ui(get_ux_page_instance(), player2));
			shared_ptr<b_toggle_ui> bui2 = std::static_pointer_cast<b_toggle_ui>(b2);
			bui2->init(toggle_ui_rect2, 0x8200b4);

			player1->set_note(musical_note::get_note(musical_note::a_note, 2));
			player2->set_note(musical_note::get_note(musical_note::g_note, 3));
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			ux_page::receive(idp);
		}

		virtual void update_state()
		{
			ux_page::update_state();
		}

		virtual void update_view(shared_ptr<ux_camera> g)
		{
			ux_page::update_view(g);

			const std::string& text = get_unit()->get_name();

			g->drawText(text, 10, 20);
		}

		std::shared_ptr<guitar_note_player> player1;
		std::shared_ptr<guitar_note_player> player2;
	};
}


void unit_test_snd_android::init_ux()
{
	ux_page::new_shared_instance(new unit_test_snd_android_ns::main_page(uxroot));
	ux_cam->clear_color = true;
}

void unit_test_snd_android::load()
{
}

#endif
