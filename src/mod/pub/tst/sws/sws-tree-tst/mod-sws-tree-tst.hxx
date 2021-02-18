#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_SWS_TREE_TST

#include "mws-mod.hxx"


class mod_sws_tree_tst : public mws_mod
{
public:
   static mws_sp<mod_sws_tree_tst> nwi();
   virtual void init_mws() override;

private:
   mod_sws_tree_tst();
};

#endif
