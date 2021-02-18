#include "stdafx.hxx"

#include "mod-sws-tree-tst.hxx"
#include "mws/mws-ext.hxx"
#include "mws/mws-camera.hxx"


namespace ns_sws_tree_tst
{
   class main_ui_page : public mws_page
   {
   public:
      virtual void init() override
      {
         mws_sp<mws_tree_model_node> node = std::make_shared<mws_tree_model_node>("nodexx");
         mws_sp<mws_tree_model> model = std::make_shared<mws_tree_model>();
         mws_sp<mws_tree> tree = mws_tree::nwi();

         for (uint32_t k = 0; k < 10; k++)
         {
            node->nodes.push_back(std::make_shared<mws_tree_model_node>("node-" + std::to_string(k)));
         }

         model->set_root_node(node);
         tree->set_model(model);
         attach(tree);
      }
   };
}
using namespace ns_sws_tree_tst;


mod_sws_tree_tst::mod_sws_tree_tst() : mws_mod(mws_stringify(MOD_SWS_TREE_TST)) {}

mws_sp<mod_sws_tree_tst> mod_sws_tree_tst::nwi()
{
   return mws_sp<mod_sws_tree_tst>(new mod_sws_tree_tst());
}

void mod_sws_tree_tst::init_mws()
{
   mws_root->new_page<main_ui_page>();
   mws_cam->clear_color = true;
}
