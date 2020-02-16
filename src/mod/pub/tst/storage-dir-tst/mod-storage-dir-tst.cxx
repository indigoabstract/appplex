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
   mws_path test_path_0("path_0/path_1/path_2");
   mws_path test_parent_path_0 = test_path_0.parent_path();
   mws_path test_path_1("path_0/path_1/path_2/");
   mws_path test_parent_path_1 = test_path_1.parent_path();
   mws_path prv_path(storage.prv_dir());
   mws_path res_path(storage.res_dir());
   mws_path tmp_path(storage.tmp_dir());
   mws_path parent_path = prv_path.parent_path();
   const unicode_string& app_path_wstr = mws::args::get_app_path();
   std::string app_path_str = mws_str::unicode_string_to_string(app_path_wstr);
   mws_path app_path(app_path_str);
   std::string sample = storage.load_as_string("sample.txt");

   mws_println_all_builds("test_path_0 [ %s ]", test_path_0.string().c_str());
   mws_println_all_builds("test_parent_path_0 [ %s ]", test_parent_path_0.string().c_str());
   mws_println_all_builds("test_path_1 [ %s ]", test_path_1.string().c_str());
   mws_println_all_builds("test_parent_path_1 [ %s ]", test_parent_path_1.string().c_str());
   mws_println_all_builds("prv_path [ %s ]", prv_path.string().c_str());
   mws_println_all_builds("private_path_parent [ %s ]", parent_path.string().c_str());
   mws_println_all_builds("res_path [ %s ]", res_path.string().c_str());
   mws_println_all_builds("tmp_path [ %s ]", tmp_path.string().c_str());
   mws_println_all_builds("app_path [ %s ]", app_path.parent_path().string().c_str());
   mws_println_all_builds("crt_path [ %s ]", mws_path::current_path().c_str());

   std::vector<mws_sp<mws_file>> prv_files = prv_path.list_directory(true);
   std::vector<mws_sp<mws_file>> res_files = res_path.list_directory(true);
   std::vector<mws_sp<mws_file>> tmp_files = tmp_path.list_directory(true);

   mws_println_all_builds("\n[ %d ] prv_files", prv_files.size());
   for (auto& e : prv_files)
   {
      mws_println_all_builds("[ %s ]", e->string_path().c_str());
   }

   mws_println_all_builds("\n[ %d ] res_files", res_files.size());
   for (auto& e : res_files)
   {
      mws_println_all_builds("[ %s ]", e->string_path().c_str());
   }

   mws_println_all_builds("\n[ %d ] tmp_files", tmp_files.size());
   for (auto& e : tmp_files)
   {
      mws_println_all_builds("[ %s ]", e->string_path().c_str());
   }

   if (!sample.empty()) { mws_println_all_builds("\nsample-text [ %s ]", sample.c_str()); }
}
