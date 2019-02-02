#include "stdafx.hxx"

#include "mod-test-mws.hxx"

#ifdef MOD_TEST_MWS

#include "mws/mws-ext.hxx"
#include "mws/mws-camera.hxx"


mod_test_mws::mod_test_mws() : mws_mod(mws_stringify(MOD_TEST_MWS)) {}

mws_sp<mod_test_mws> mod_test_mws::nwi()
{
	return mws_sp<mod_test_mws>(new mod_test_mws());
}

void mod_test_mws::init_mws()
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
			mws_list_menu_item("mws-mod-xxx"),
			mws_list_menu_item("mws-mod-dsds"),
			mws_list_menu_item("mws-mod-xzc"),
			mws_list_menu_item("mws-mod-757"),
			mws_list_menu_item("mws-mod-fg"),
			mws_list_menu_item("mws-mod-74gf"),
			mws_list_menu_item("mws-mod-g5u"),
			mws_list_menu_item("mws-mod-08b"),
			mws_list_menu_item("mws-mod-kif"),
			mws_list_menu_item("mws-mod-vbf"),
			mws_list_menu_item("mws-mod-fd7"),
			mws_list_menu_item("mws-mod-f09"),
			mws_list_menu_item("mws-mod-fe35"),
			mws_list_menu_item("mws-mod-23r"),
			mws_list_menu_item("mws-mod-s324"),
			mws_list_menu_item("mws-mod-2g4yh"),
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
