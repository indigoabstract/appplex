#include "stdafx.hxx"

#include "mod-test-audioengine.hxx"

#ifdef MOD_TEST_AUDIOENGINE

#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"


mod_test_audioengine::mod_test_audioengine() : mws_mod(mws_stringify(MOD_TEST_AUDIOENGINE)) {}

mws_sp<mod_test_audioengine> mod_test_audioengine::nwi()
{
	return mws_sp<mod_test_audioengine>(new mod_test_audioengine());
}

void mod_test_audioengine::init()
{
}


namespace mod_test_audioengine_ns
{
	class main_page : public mws_page
	{
	public:
		main_page(mws_sp<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();
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
	};
}


void mod_test_audioengine::init_mws()
{
   mws_root->new_page<mod_test_audioengine_ns::mainpage>();
	mws_cam->clear_color = true;
}

void mod_test_audioengine::load()
{
}

#endif
