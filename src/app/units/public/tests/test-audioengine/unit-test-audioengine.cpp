#include "stdafx.h"

#include "unit-test-audioengine.hpp"

#ifdef UNIT_TEST_AUDIOENGINE

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"


unit_test_audioengine::unit_test_audioengine()
{
	set_name("test_audioengine");
}

shared_ptr<unit_test_audioengine> unit_test_audioengine::nwi()
{
	return shared_ptr<unit_test_audioengine>(new unit_test_audioengine());
}

void unit_test_audioengine::init()
{
}


namespace unit_test_audioengine_ns
{
	class main_page : public mws_page
	{
	public:
		main_page(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			const std::string& text = get_unit()->get_name();

			g->drawText(text, 10, 20);
		}
	};
}


void unit_test_audioengine::init_mws()
{
   mws_root->new_page<unit_test_audioengine_ns::mainpage>();
	mws_cam->clear_color = true;
}

void unit_test_audioengine::load()
{
}

#endif
