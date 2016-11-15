#include "stdafx.h"

#include "unit-test-audioengine.hpp"

#ifdef UNIT_TEST_AUDIOENGINE

#include "com/ux/ux-camera.hpp"
#include "com/ux/ux-com.hpp"


unit_test_audioengine::unit_test_audioengine()
{
	set_name("test_audioengine");
}

shared_ptr<unit_test_audioengine> unit_test_audioengine::new_instance()
{
	return shared_ptr<unit_test_audioengine>(new unit_test_audioengine());
}

void unit_test_audioengine::init()
{
}


namespace unit_test_audioengine_ns
{
	class main_page : public ux_page
	{
	public:
		main_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent){}

		virtual void init()
		{
			ux_page::init();
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
	};
}


void unit_test_audioengine::init_ux()
{
	ux_page::new_shared_instance(new unit_test_audioengine_ns::main_page(uxroot));
	ux_cam->clear_color = true;
}

void unit_test_audioengine::load()
{
}

#endif
