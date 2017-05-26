#include "stdafx.h"

#include "unit-ux-test.hpp"

#ifdef UNIT_UX_TEST

#include "com/ux/ux-ext.hpp"
#include "com/ux/ux-camera.hpp"


unit_ux_test::unit_ux_test()
{
	set_name("ux-test");
}

shared_ptr<unit_ux_test> unit_ux_test::new_instance()
{
	return shared_ptr<unit_ux_test>(new unit_ux_test());
}

void unit_ux_test::init_ux()
{
	shared_ptr<ux_list_menu_page> up;
	shared_ptr<ux_list_menu_model> ulmm;

	{
		up = ux_list_menu_page::new_instance(uxroot, "p0");
		ux_list_menu_item lmt[] = 
		{
			ux_list_menu_item("menu-item-1", "p1"),
			ux_list_menu_item("menu-item-2", "p2"),
			ux_list_menu_item("menu-item-3"),
			ux_list_menu_item("menu-item-4"),
		};

		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	{
		up = ux_list_menu_page::new_instance(uxroot, "p1");
		ux_list_menu_item lmt[] = 
		{
			ux_list_menu_item("xp2", "p2"),
			ux_list_menu_item("xxxx", "pvbiob"),
			ux_list_menu_item("yyyffdd", "0ltjgf"),
			ux_list_menu_item("fffggggggg", "x4"),
			ux_list_menu_item("unit-xxx"),
			ux_list_menu_item("unit-dsds"),
			ux_list_menu_item("unit-xzc"),
			ux_list_menu_item("unit-757"),
			ux_list_menu_item("unit-fg"),
			ux_list_menu_item("unit-74gf"),
			ux_list_menu_item("unit-g5u"),
			ux_list_menu_item("unit-08b"),
			ux_list_menu_item("unit-kif"),
			ux_list_menu_item("unit-vbf"),
			ux_list_menu_item("unit-fd7"),
			ux_list_menu_item("unit-f09"),
			ux_list_menu_item("unit-fe35"),
			ux_list_menu_item("unit-23r"),
			ux_list_menu_item("unit-s324"),
			ux_list_menu_item("unit-2g4yh"),
		};

		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	{
		ux_list_menu_item lmt[] = 
		{
			ux_list_menu_item("menu-xx-1", "p1"),
			ux_list_menu_item("menu-x-2"),
			ux_list_menu_item("menu-x-3"),
		};

		up = ux_list_menu_page::new_instance(uxroot, "p2");
		ulmm = up->get_list_menu_model();
		ulmm->set_data(xlmdata(lmt));
	}

	ux_cam->clear_color = true;
}

#endif
