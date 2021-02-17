#pragma once

#include "mws-com.hxx"


class mws_list_menu_item
{
public:
   mws_list_menu_item(std::string idisplay, std::string i_id = "");
   mws_list_menu_item(std::string idisplay, mws_sp<mws_page> ipage);
   virtual ~mws_list_menu_item() {}

   std::string display;
   std::string id;
};


#define xlmdata(note_tab) note_tab, sizeof(note_tab) / sizeof(mws_list_menu_item)

class mws_list_menu_model : public mws_list_model
{
public:
   mws_list_menu_model();
   virtual ~mws_list_menu_model() {}

   virtual void notify_update();
   virtual uint32_t get_length();
   virtual std::string elem_at(uint32_t idx);
   virtual void on_elem_selected(uint32_t idx);
   void set_data(mws_list_menu_item ielems[], uint32_t ielemsLength);
   void set_data(std::vector<mws_list_menu_item> ielems);
   virtual void change_page_transitions(uint32_t idx);

protected:
   std::vector<mws_list_menu_item> elems;

private:
   void set_data_helper(mws_sp<mws_page_tab> mws_root, mws_list_menu_item& ulmi);
};


class mws_list_menu_page : public mws_page
{
public:
   virtual ~mws_list_menu_page() {}
   static mws_sp<mws_list_menu_page> nwi(mws_sp<mws_page_tab> i_parent, std::string i_id);

   virtual void receive(mws_sp<mws_dp> i_dp);
   mws_sp<mws_list_menu_model> get_list_menu_model();

protected:
   mws_list_menu_page();

   mws_sp<mws_list> item_list;
};
