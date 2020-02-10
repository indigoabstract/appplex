#include "stdafx.hxx"

#include "mod-storage-dir-tst.hxx"
#include "pfm.hxx"


mod_storage_dir_tst::mod_storage_dir_tst() : mws_mod(mws_stringify(MOD_STORAGE_DIR_TST)) {}

mws_sp<mod_storage_dir_tst> mod_storage_dir_tst::nwi()
{
   return mws_sp<mod_storage_dir_tst>(new mod_storage_dir_tst());
}

void mod_storage_dir_tst::init()
{
   mws_log::set_enabled(true);
   mws_log::i()->push("mod_storage_dir_tst::init()");
   mws_path test_path("path_0/path_1/path_2");
   mws_path test_parent_path = test_path.parent_path();
   mws_path private_path(mws::filesys::prv_dir());
   mws_path parent_path = private_path.parent_path();
   mws_path res_path(mws::filesys::res_dir());
   mws_path tmp_path(mws::filesys::tmp_dir());
   const unicode_string& app_path_wstr = mws::args::get_app_path();
   std::string app_path_str = mws_str::unicode_string_to_string(app_path_wstr);
   mws_path app_path(app_path_str);
   mws_sp<std::string> sample = mws::filesys::load_res_as_string("sample.txt");

   mws_println_all_builds("test_path [ %s ]", test_path.string().c_str());
   mws_println_all_builds("test_parent_path [ %s ]", test_parent_path.string().c_str());
   mws_println_all_builds("private_path [ %s ]", private_path.string().c_str());
   mws_println_all_builds("private_path_parent [ %s ]", parent_path.string().c_str());
   mws_println_all_builds("res_path [ %s ]", res_path.string().c_str());
   mws_println_all_builds("tmp_path [ %s ]", tmp_path.string().c_str());
   mws_println_all_builds("app_path [ %s ]", app_path.parent_path().string().c_str());
   mws_println_all_builds("crt_path [ %s ]", mws_path::current_path().c_str());
   if (sample) { mws_println_all_builds("sample-text [ %s ]", sample->c_str()); }
}
