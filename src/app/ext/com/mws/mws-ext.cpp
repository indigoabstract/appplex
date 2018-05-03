#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS

#include "mws-ext.hpp"


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

mws_list_menu_item::mws_list_menu_item(string idisplay, shared_ptr<mws_page> ipage)
{
   display = idisplay;
   upt = mws_page_transition::nwi(ipage);
}

mws_list_menu_item::mws_list_menu_item(string idisplay, shared_ptr<mws_page_transition> iupt)
{
   display = idisplay;
   upt = iupt;
}

shared_ptr<mws_page> mws_list_menu_item::get_page(shared_ptr<mws> u)
{
   return upt->get_target_page();
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
   shared_ptr<mws_list> ul = static_pointer_cast<mws_list>(get_view());
   shared_ptr<mws_page> up = ul->get_mws_page_item_parent();
   shared_ptr<mws_page_tab> ut = up->get_mws_page_parent();
   shared_ptr<mws_page> np = elems[idx].get_page(ul);

   change_page_transitions(idx);

   if (np && np != mws_page::PAGE_NONE)
   {
      shared_ptr<mws_page_transition> upt = mws_page_transition::nwi(np)
         ->set_transition_type(mws_page_transition::PUSH_CURRENT_PAGE);

      send(ut, upt);
      //send(ut, up->tmap[touch_sym_evt::TS_FORWARD_SWIPE]);
   }
}

void mws_list_menu_model::set_data(mws_list_menu_item ielems[], int ielemsLength)
{
   shared_ptr<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (int k = 0; k < ielemsLength; k++)
   {
      set_data_helper(mws_root, ielems[k]);
   }

   notify_update();
}

void mws_list_menu_model::set_data(std::vector<mws_list_menu_item> ielems)
{
   shared_ptr<mws_page_tab> mws_root = get_view()->get_mws_root();

   elems.clear();

   for (auto e : ielems)
   {
      set_data_helper(mws_root, e);
   }

   notify_update();
}

void mws_list_menu_model::change_page_transitions(int idx)
{
   if (!elems.empty())
   {
      shared_ptr<mws_list> ul = static_pointer_cast<mws_list>(get_view());
      shared_ptr<mws_page> up = ul->get_mws_page_item_parent();

      up->tmap[touch_sym_evt::TS_FORWARD_SWIPE] = elems[idx].upt;
   }
}

void mws_list_menu_model::set_data_helper(shared_ptr<mws_page_tab> mws_root, mws_list_menu_item& ulmi)
{
   elems.push_back(ulmi);

   mws_list_menu_item& tulmi = elems.back();

   if (!tulmi.upt)
   {
      tulmi.upt = mws_page_transition::nwi(mws_root, tulmi.id);
   }
}


mws_list_menu_page::mws_list_menu_page()
{
}

shared_ptr<mws_list_menu_page> mws_list_menu_page::nwi(shared_ptr<mws_page_tab> iparent, string iid)
{
   shared_ptr<mws_list_menu_page> u = static_pointer_cast<mws_list_menu_page>(new_shared_instance(iparent, new mws_list_menu_page()));
   u->set_id(iid);
   u->item_list = mws_list::nwi(u);
   u->item_list->set_model(shared_ptr<mws_list_model>(new mws_list_menu_model()));
   return u;
}

void mws_list_menu_page::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      switch (ts->get_type())
      {
      case touch_sym_evt::TS_FORWARD_SWIPE:
      {
         shared_ptr<mws_list_menu_model> ulmm = get_list_menu_model();

         ulmm->change_page_transitions(ulmm->get_selected_elem());
         break;
      }
      }
   }

   mws_page::receive(idp);
}

shared_ptr<mws_list_menu_model> mws_list_menu_page::get_list_menu_model()
{
   return static_pointer_cast<mws_list_menu_model>(item_list->get_model());
}

#endif
