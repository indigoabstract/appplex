#include "stdafx.hxx"

#include "mws-ext.hxx"


mws_list_menu_item::mws_list_menu_item(std::string idisplay, std::string i_id)
{
   display = idisplay;
   id = i_id;

   if (id.length() == 0)
   {
      id = display;
   }
}

mws_list_menu_item::mws_list_menu_item(std::string idisplay, mws_sp<mws_page> ipage)
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

uint32_t mws_list_menu_model::get_length()
{
   return elems.size();
}

std::string mws_list_menu_model::elem_at(uint32_t idx)
{
   return elems[idx].display;
}

void mws_list_menu_model::on_elem_selected(uint32_t idx)
{
   mws_sp<mws_list> ul = static_pointer_cast<mws_list>(get_view());
   mws_sp<mws_page> up = ul->get_page();
   mws_sp<mws_page_tab> ut = up->get_mws_page_parent();

   change_page_transitions(idx);
}

void mws_list_menu_model::set_data(mws_list_menu_item ielems[], uint32_t ielemsLength)
{
   mws_sp<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (uint32_t k = 0; k < ielemsLength; k++)
   {
      set_data_helper(mws_root, ielems[k]);
   }

   notify_update();
}

void mws_list_menu_model::set_data(std::vector<mws_list_menu_item> ielems)
{
   mws_sp<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (auto& e : ielems)
   {
      set_data_helper(mws_root, e);
   }

   notify_update();
}

void mws_list_menu_model::change_page_transitions(uint32_t idx)
{
}

void mws_list_menu_model::set_data_helper(mws_sp<mws_page_tab> mws_root, mws_list_menu_item& ulmi)
{
   elems.push_back(ulmi);
}


mws_list_menu_page::mws_list_menu_page()
{
}

mws_sp<mws_list_menu_page> mws_list_menu_page::nwi(mws_sp<mws_page_tab> i_parent, std::string i_id)
{
   auto u = mws_sp<mws_list_menu_page>(new mws_list_menu_page());
   i_parent->add_page(u);
   u->set_id(i_id);
   u->item_list = mws_list::nwi();
   u->item_list->set_model(mws_sp<mws_list_model>(new mws_list_menu_model()));
   return u;
}

void mws_list_menu_page::receive(mws_sp<mws_dp> i_dp)
{
   mws_page::receive(i_dp);
}

mws_sp<mws_list_menu_model> mws_list_menu_page::get_list_menu_model()
{
   return static_pointer_cast<mws_list_menu_model>(item_list->get_model());
}
