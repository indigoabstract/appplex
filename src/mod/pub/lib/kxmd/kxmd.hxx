#pragma once

#include "pfm-def.h"
#include <string>
#include <vector>


namespace ns_kxmd
{
   class kxmd;
   class kxmd_impl;

   struct kxmd_kv
   {
      std::string key;
      std::vector<mws_sp<kxmd_kv>> val;
   };


   class kxmd_ref
   {
   public:
      // constructs an empty/nullptr object
      kxmd_ref();

      bool valid();
      mws_sp<kxmd> get_db() const;
      std::string key();
      size_t size();
      bool is_leaf();
      bool is_node();
      void del_val();
      void del_val_at_idx(uint32 i_idx);
      kxmd_ref elem_at_idx(uint32 i_idx);
      // gets a reference to an elem that's inside the given elem
      // i_path is like xxx.yyy.zzz.etc
      kxmd_ref elem_at_path(const std::string& i_path);
      // fast way to get the first value in a node list
      // for example, for "default.start.mod-list" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "mod-list"
      // returns i_default_val if path is empty or does not exist
      std::string val_at(const std::string& i_path, const std::string& i_default_val = "");
      // fast way to get the leaf list of node-leaf-list elems
      // for example, for "default.start" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "[ mod-list, exclusive, launch-mod, ]"
      // returns i_default_val if path is empty or does not exist
      std::vector<std::string> val_seq_at(const std::string& i_path, const std::vector<std::string>& i_default_val = {});
      // checks if path exists, starting from a given root
      // path is like xxx.yyy.zzz.etc
      bool path_exists(const std::string& i_path);
      void set_key(const std::string& i_key);
      kxmd_ref push_back(const std::string& i_key);
      void push_back(const std::vector<std::string>& i_list);
      kxmd_ref find_by_key(const std::string& i_key, bool i_recursive = false);
      std::string to_string();
      std::string to_string_list();

   private:
      friend class kxmd_impl;
      kxmd_ref(mws_sp<kxmd> i_db, mws_sp<kxmd_kv> i_kv);

      mws_wp<kxmd> db;
      mws_wp<kxmd_kv> kv;
   };


   class kxmd
   {
   public:
      static mws_sp<kxmd> nwi_kxmd(const std::string& i_kxmd_data = "");
      static mws_sp<kxmd> nwi_from_file(const std::string& i_filename = "");
      ~kxmd();
      kxmd_ref main();

      static mws_sp<kxmd_kv> nwi() { return mws_sp<kxmd_kv>(new kxmd_kv()); }

      static size_t size(mws_sp<kxmd_kv> i_r) { return i_r->val.size(); }
      static bool is_leaf(mws_sp<kxmd_kv> i_r) { return i_r->val.empty() && !i_r->key.empty(); };
      static bool is_node(mws_sp<kxmd_kv> i_r) { return !i_r->val.empty(); };
      // gets a reference to a kxmd_kv that's inside the given kxmd_kv
      // i_path is like xxx.yyy.zzz.etc
      static mws_sp<kxmd_kv> elem_at(mws_sp<kxmd_kv> i_r, const std::string& i_path);
      // fast way to get the first value in a node list
      // for example, for "default.start.mod-list" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "mod-list"
      // returns i_default_val if path is empty or does not exist
      static std::string val_at(mws_sp<kxmd_kv> i_r, const std::string& i_path, const std::string& i_default_val = "");
      // fast way to get the leaf list of node-leaf-list elems
      // for example, for "default.start" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "[ mod-list, exclusive, launch-mod, ]"
      // returns i_default_val if path is empty or does not exist
      static std::vector<std::string> val_seq_at(mws_sp<kxmd_kv> i_r, const std::string& i_path, const std::vector<std::string>& i_default_val = {});
      // checks if path exists, starting from a given root
      // path is like xxx.yyy.zzz.etc
      static bool path_exists(mws_sp<kxmd_kv> i_r, const std::string& i_path);
      static void push_back(mws_sp<kxmd_kv> i_r, const mws_sp<kxmd_kv> i_val);
      static void push_back(mws_sp<kxmd_kv> i_r, const std::string& i_val);
      static void push_back(mws_sp<kxmd_kv> i_r, const std::vector<std::string>& i_list);
      static mws_sp<kxmd_kv> find_by_name(mws_sp<kxmd_kv> i_r, const std::string& i_name, bool i_recursive = false);
      static std::string to_string(mws_sp<kxmd_kv> i_r);
      static std::string to_string_list(mws_sp<kxmd_kv> i_r);
      static std::string to_string_impl(mws_sp<kxmd_kv> i_r, int i_level);
      static mws_sp<kxmd_kv> parse(mws_sp<std::string> i_src);

   private:
      friend class kxmd_impl;
      kxmd();

      kxmd_impl* p = nullptr;
   };
}
using ns_kxmd::kxmd_ref;
using ns_kxmd::kxmd;
using ns_kxmd::kxmd_kv;
