#include "stdafx.h"

#include "ux-ext.hpp"


using std::string;


ux_list_menu_item::ux_list_menu_item(string idisplay, string iid)
{
	display = idisplay;
	id = iid;

	if(id.length() == 0)
	{
		id = display;
	}
}

ux_list_menu_item::ux_list_menu_item(string idisplay, shared_ptr<ux_page> ipage)
{
	display = idisplay;
	upt = ux_page_transition::new_instance(ipage);
}

ux_list_menu_item::ux_list_menu_item(string idisplay, shared_ptr<ux_page_transition> iupt)
{
	display = idisplay;
	upt = iupt;
}

shared_ptr<ux_page> ux_list_menu_item::get_page(shared_ptr<ux> u)
{
	return upt->get_target_page();
}

ux_list_menu_model::ux_list_menu_model()
{
}

void ux_list_menu_model::notify_update()
{
	change_page_transitions(get_selected_elem());
	ux_list_model::notify_update();
}

int ux_list_menu_model::get_length()
{
	return elems.size();
}

string ux_list_menu_model::elem_at(int idx)
{
	return elems[idx].display;
}

void ux_list_menu_model::on_elem_selected(int idx)
{
	shared_ptr<ux_list> ul = static_pointer_cast<ux_list>(get_view());
	shared_ptr<ux_page> up = ul->get_ux_page_item_parent();
	shared_ptr<ux_page_tab> ut = up->get_ux_page_parent();
	shared_ptr<ux_page> np = elems[idx].get_page(ul);

	change_page_transitions(idx);

	if(np && np != ux_page::PAGE_NONE)
	{
		shared_ptr<ux_page_transition> upt = ux_page_transition::new_instance(np)
			->set_transition_type(ux_page_transition::PUSH_CURRENT_PAGE);

		send(ut, upt);
		//send(ut, up->tmap[touch_sym_evt::TS_FORWARD_SWIPE]);
	}
}

void ux_list_menu_model::set_data(ux_list_menu_item ielems[], int ielemsLength)
{
	shared_ptr<ux_page_tab> uxroot = get_view()->get_root();

	elems.clear();

	for(int k = 0; k < ielemsLength; k++)
	{
		set_data_helper(uxroot, ielems[k]);
	}

	notify_update();
}

void ux_list_menu_model::set_data(std::vector<ux_list_menu_item> ielems)
{
	shared_ptr<ux_page_tab> uxroot = get_view()->get_root();

	elems.clear();

	for(auto e : ielems)
	{
		set_data_helper(uxroot, e);
	}

	notify_update();
}

void ux_list_menu_model::change_page_transitions(int idx)
{
	if(!elems.empty())
	{
		shared_ptr<ux_list> ul = static_pointer_cast<ux_list>(get_view());
		shared_ptr<ux_page> up = ul->get_ux_page_item_parent();

		up->tmap[touch_sym_evt::TS_FORWARD_SWIPE] = elems[idx].upt;
	}
}

void ux_list_menu_model::set_data_helper(shared_ptr<ux_page_tab> uxroot, ux_list_menu_item& ulmi)
{
	elems.push_back(ulmi);

	ux_list_menu_item& tulmi = elems.back();

	if(!tulmi.upt)
	{
		tulmi.upt = ux_page_transition::new_instance(uxroot, tulmi.id);
	}
}


ux_list_menu_page::ux_list_menu_page(shared_ptr<ux_page_tab> iparent) : ux_page(iparent)
{
}

shared_ptr<ux_list_menu_page> ux_list_menu_page::new_instance(shared_ptr<ux_page_tab> iparent, string iid)
{
	shared_ptr<ux_list_menu_page> u = static_pointer_cast<ux_list_menu_page>(new_shared_instance(new ux_list_menu_page(iparent)));
	u->set_id(iid);
	u->item_list = ux_list::new_instance(u);
	u->item_list->set_model(shared_ptr<ux_list_model>(new ux_list_menu_model()));
	return u;
}

void ux_list_menu_page::receive(shared_ptr<iadp> idp)
{
	if(idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
	{
		shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

		switch(ts->get_type())
		{
		case touch_sym_evt::TS_FORWARD_SWIPE:
			{
				shared_ptr<ux_list_menu_model> ulmm = get_list_menu_model();
				
				ulmm->change_page_transitions(ulmm->get_selected_elem());
				break;
			}
		}
	}

	ux_page::receive(idp);
}

shared_ptr<ux_list_menu_model> ux_list_menu_page::get_list_menu_model()
{
	return static_pointer_cast<ux_list_menu_model>(item_list->get_model());
}
