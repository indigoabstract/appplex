#include "stdafx.h"

#include "unit-test-mws.hpp"

#ifdef UNIT_TEST_MWS

#include "com/mws/mws-ext.hpp"
#include "com/mws/mws-camera.hpp"


unit_test_mws::unit_test_mws() : unit(mws_stringify(UNIT_TEST_MWS)) {}

mws_sp<unit_test_mws> unit_test_mws::nwi()
{
	return mws_sp<unit_test_mws>(new unit_test_mws());
}

void unit_test_mws::init_mws()
{
	mws_sp<mws_list_menu_page> up;
	mws_sp<mws_list_menu_model> ulmm;

	{
		up = mws_list_menu_page::nwi(mws_root, "p0");
		mws_list_menu_item lmt[] = 
		{
			mws_list_menu_item("menu-item-1", "p1"),
			mws_list_menu_item("menu-item-2", "p2"),
			mws_list_menu_item("menu-item-3"),
			mws_list_menu_item("menu-item-4"),
		};

		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	{
		up = mws_list_menu_page::nwi(mws_root, "p1");
		mws_list_menu_item lmt[] = 
		{
			mws_list_menu_item("xp2", "p2"),
			mws_list_menu_item("xxxx", "pvbiob"),
			mws_list_menu_item("yyyffdd", "0ltjgf"),
			mws_list_menu_item("fffggggggg", "x4"),
			mws_list_menu_item("unit-xxx"),
			mws_list_menu_item("unit-dsds"),
			mws_list_menu_item("unit-xzc"),
			mws_list_menu_item("unit-757"),
			mws_list_menu_item("unit-fg"),
			mws_list_menu_item("unit-74gf"),
			mws_list_menu_item("unit-g5u"),
			mws_list_menu_item("unit-08b"),
			mws_list_menu_item("unit-kif"),
			mws_list_menu_item("unit-vbf"),
			mws_list_menu_item("unit-fd7"),
			mws_list_menu_item("unit-f09"),
			mws_list_menu_item("unit-fe35"),
			mws_list_menu_item("unit-23r"),
			mws_list_menu_item("unit-s324"),
			mws_list_menu_item("unit-2g4yh"),
		};

		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	{
		mws_list_menu_item lmt[] = 
		{
			mws_list_menu_item("menu-xx-1", "p1"),
			mws_list_menu_item("menu-x-2"),
			mws_list_menu_item("menu-x-3"),
		};

		up = mws_list_menu_page::nwi(mws_root, "p2");
		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	mws_cam->clear_color = true;
}

#endif
