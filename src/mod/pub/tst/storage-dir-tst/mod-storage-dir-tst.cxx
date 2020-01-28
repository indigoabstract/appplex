#include "stdafx.hxx"

#include "mod-storage-dir-tst.hxx"


mod_storage_dir_tst::mod_storage_dir_tst() : mws_mod(mws_stringify(MOD_STORAGE_DIR_TST)) {}

mws_sp<mod_storage_dir_tst> mod_storage_dir_tst::nwi()
{
   return mws_sp<mod_storage_dir_tst>(new mod_storage_dir_tst());
}

void mod_storage_dir_tst::init()
{
}
