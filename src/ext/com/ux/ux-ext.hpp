#pragma once

#include "ux-com.hpp"


class ux_list_menu_item
{
public:
	ux_list_menu_item(std::string idisplay, std::string iid = "");
	ux_list_menu_item(std::string idisplay, shared_ptr<ux_page> ipage);
	ux_list_menu_item(std::string idisplay, shared_ptr<ux_page_transition> iupt);
	virtual ~ux_list_menu_item(){}

	shared_ptr<ux_page> get_page(shared_ptr<ux> u);

	std::string display;
	std::string id;
	shared_ptr<ux_page_transition> upt;
};


#define xlmdata(note_tab) note_tab, sizeof(note_tab) / sizeof(ux_list_menu_item)

class ux_list_menu_model : public ux_list_model
{
public:
	ux_list_menu_model();
	virtual ~ux_list_menu_model(){}

	virtual void notify_update();
	virtual int get_length();
	virtual std::string elem_at(int idx);
	virtual void on_elem_selected(int idx);
	void set_data(ux_list_menu_item ielems[], int ielemsLength);
	void set_data(std::vector<ux_list_menu_item> ielems);
	virtual void change_page_transitions(int idx);

protected:
	std::vector<ux_list_menu_item> elems;

private:
	void set_data_helper(shared_ptr<ux_page_tab> uxroot, ux_list_menu_item& ulmi);
};


class ux_list_menu_page : public ux_page
{
public:
	virtual ~ux_list_menu_page(){}
	static shared_ptr<ux_list_menu_page> new_instance(shared_ptr<ux_page_tab> iparent, std::string iid);

	virtual void receive(shared_ptr<iadp> idp);
	shared_ptr<ux_list_menu_model> get_list_menu_model();

protected:
	ux_list_menu_page(shared_ptr<ux_page_tab> iparent);

	shared_ptr<ux_list> item_list;
};
