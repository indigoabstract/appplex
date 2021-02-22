#include "stdafx.hxx"

#include "mod-sws-drop-down-list-tst.hxx"
#include "mws/mws-ext.hxx"
#include "mws/mws-camera.hxx"


namespace ns_sws_drop_down_list_tst
{
   class main_ui_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_sp<mws_str_list_model> model = std::make_shared<mws_str_list_model>();
         mws_sp<mws_drop_down_list> list = mws_drop_down_list::nwi();
         std::vector<std::string> data;

         for (uint32_t k = 0; k < 13; k++)
         {
            data.push_back("node-" + std::to_string(k));
         }

         model->set_data(data);
         list->set_model(model);
         attach(list);
      }
   };
}
using namespace ns_sws_drop_down_list_tst;


mod_sws_drop_down_list_tst::mod_sws_drop_down_list_tst() : mws_mod(mws_stringify(MOD_SWS_DROP_DOWN_LIST_TST)) {}

mws_sp<mod_sws_drop_down_list_tst> mod_sws_drop_down_list_tst::nwi()
{
   return mws_sp<mod_sws_drop_down_list_tst>(new mod_sws_drop_down_list_tst());
}

void mod_sws_drop_down_list_tst::init_mws()
{
   mws_root->new_page<main_ui_page>();
   mws_cam->clear_color = true;
}
