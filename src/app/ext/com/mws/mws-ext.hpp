#pragma once

#include "mws-com.hpp"


class mws_list_menu_item
{
public:
	mws_list_menu_item(std::string idisplay, std::string iid = "");
	mws_list_menu_item(std::string idisplay, shared_ptr<mws_page> ipage);
	mws_list_menu_item(std::string idisplay, shared_ptr<mws_page_transition> iupt);
	virtual ~mws_list_menu_item(){}

	shared_ptr<mws_page> get_page(shared_ptr<mws> u);

	std::string display;
	std::string id;
	shared_ptr<mws_page_transition> upt;
};


#define xlmdata(note_tab) note_tab, sizeof(note_tab) / sizeof(mws_list_menu_item)

class mws_list_menu_model : public mws_list_model
{
public:
	mws_list_menu_model();
	virtual ~mws_list_menu_model(){}

	virtual void notify_update();
	virtual int get_length();
	virtual std::string elem_at(int idx);
	virtual void on_elem_selected(int idx);
	void set_data(mws_list_menu_item ielems[], int ielemsLength);
	void set_data(std::vector<mws_list_menu_item> ielems);
	virtual void change_page_transitions(int idx);

protected:
	std::vector<mws_list_menu_item> elems;

private:
	void set_data_helper(shared_ptr<mws_page_tab> mws_root, mws_list_menu_item& ulmi);
};


class mws_list_menu_page : public mws_page
{
public:
	virtual ~mws_list_menu_page(){}
	static shared_ptr<mws_list_menu_page> new_instance(shared_ptr<mws_page_tab> iparent, std::string iid);

	virtual void receive(shared_ptr<iadp> idp);
	shared_ptr<mws_list_menu_model> get_list_menu_model();

protected:
	mws_list_menu_page(shared_ptr<mws_page_tab> iparent);

	shared_ptr<mws_list> item_list;
};
