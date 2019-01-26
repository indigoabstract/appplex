#include "stdafx.h"

#include "unit-texxed.hpp"

#ifdef UNIT_TEXXED

#include "com/mws/mws-text-box.hpp"
#include "com/mws/mws-camera.hpp"


namespace unit_texxed_ns
{
   class main_page : public mws_page
   {
   public:
      main_page() {}

      virtual void init()
      {
         mws_page::init();

			int w = get_unit()->get_width();
			int h = get_unit()->get_height();

			tx_res = pfm::filesystem::load_res_as_string("kappaxx-src.kxmd");

			if (!tx_res)
			{
				trx("error: kappaxx-src.kxmd not found.");

				return;
			}

			ta = text_box::nwi();
			ta->set_position(glm::vec2(50, 70));
			ta->set_dimension(glm::vec2(w - 80, h - 130));
			ta->set_text(*tx_res);
         ta->push_back_text("nl 1\n");
         ta->push_back_text("nl 2\n");
         ta->push_back_text("nl 3\n");
         ta->push_back_text("nl 4\n");
         ta->push_back_text("nl 5\n");
         ta->push_back_text("nl 6\n");
         ta->push_back_text("nl 7\n");
         ta->push_back_text("nl 8\n");
         ta->push_back_text("nl 9\n");
         ta->push_back_text("nl 10\n");
         ta->push_back_text("nl 11\n");
         attach(ta);

			shared_ptr<mws_button> b = mws_button::nwi();
         attach(b);
         b->set_rect(mws_rect(10, h - 50.f, 50, 20));
         b->set_text("btn");
         b->set_bg_color(gfx_color(0xff8200b4));
      }

		virtual void receive(shared_ptr<iadp> idp)
		{
			if (idp->is_processed())
			{
				return;
			}
			
			if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
			{
				ta->receive(idp);
			}

			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			std::string text = "texxed : texxt-editor";

			g->drawText(text, 10, 20);
		}

		shared_ptr<text_box> ta;
		shared_ptr<std::string > tx_res;
	};
}


unit_texxed::unit_texxed() : unit(mws_stringify(UNIT_TEXXED)) {}

shared_ptr<unit_texxed> unit_texxed::nwi()
{
	return shared_ptr<unit_texxed>(new unit_texxed());
}

void unit_texxed::init()
{
}

void unit_texxed::init_mws()
{
   mws_root->new_page<unit_texxed_ns::main_page>();
   mws_cam->clear_color = true;
	//mws_cam->clear_color_value = gfx_color::colors::white;
}

void unit_texxed::load()
{
}

#endif
