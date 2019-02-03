#include "stdafx.hxx"

#include "mws-ext.hxx"


using std::string;


mws_list_menu_item::mws_list_menu_item(string idisplay, string iid)
{
   display = idisplay;
   id = iid;

   if (id.length() == 0)
   {
      id = display;
   }
}

mws_list_menu_item::mws_list_menu_item(string idisplay, mws_sp<mws_page> ipage)
{
   display = idisplay;
}

mws_list_menu_model::mws_list_menu_model()
{
}

void mws_list_menu_model::notify_update()
{
   change_page_transitions(get_selected_elem());
   mws_list_model::notify_update();
}

int mws_list_menu_model::get_length()
{
   return elems.size();
}

string mws_list_menu_model::elem_at(int idx)
{
   return elems[idx].display;
}

void mws_list_menu_model::on_elem_selected(int idx)
{
   mws_sp<mws_list> ul = static_pointer_cast<mws_list>(get_view());
   mws_sp<mws_page> up = ul->get_mws_page_item_parent();
   mws_sp<mws_page_tab> ut = up->get_mws_page_parent();

   change_page_transitions(idx);
}

void mws_list_menu_model::set_data(mws_list_menu_item ielems[], int ielemsLength)
{
   mws_sp<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (int k = 0; k < ielemsLength; k++)
   {
      set_data_helper(mws_root, ielems[k]);
   }

   notify_update();
}

void mws_list_menu_model::set_data(std::vector<mws_list_menu_item> ielems)
{
   mws_sp<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (auto e : ielems)
   {
      set_data_helper(mws_root, e);
   }

   notify_update();
}

void mws_list_menu_model::change_page_transitions(int idx)
{
}

void mws_list_menu_model::set_data_helper(mws_sp<mws_page_tab> mws_root, mws_list_menu_item& ulmi)
{
   elems.push_back(ulmi);
}


mws_list_menu_page::mws_list_menu_page()
{
}

mws_sp<mws_list_menu_page> mws_list_menu_page::nwi(mws_sp<mws_page_tab> iparent, string iid)
{
   auto u = mws_sp<mws_list_menu_page>(new mws_list_menu_page());
   iparent->add_page(u);
   u->set_id(iid);
   u->item_list = mws_list::nwi();
   u->item_list->set_model(mws_sp<mws_list_model>(new mws_list_menu_model()));
   return u;
}

void mws_list_menu_page::receive(mws_sp<mws_dp> idp)
{
   mws_page::receive(idp);
}

mws_sp<mws_list_menu_model> mws_list_menu_page::get_list_menu_model()
{
   return static_pointer_cast<mws_list_menu_model>(item_list->get_model());
}
