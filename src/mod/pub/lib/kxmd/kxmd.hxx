#pragma once

#include "pfm-def.h"
#include "min.hxx"
#include <string>
#include <vector>


namespace ns_kxmd
{
   class kxmd;
   class kxmd_impl;
   struct kxmd_kv;


   // keyval reference class
   class kv_ref
   {
   public:
      // constructs an empty/nullptr object
      kv_ref();

      virtual bool valid() const;
      virtual mws_sp<kxmd> get_db() const;
      virtual std::string key() const;
      template<typename T> T key_as() const { return mws_to<T>(key()); }
      virtual std::vector<kv_ref> val() const;
      template<typename T> T val_as(uint32 i_idx = 0) const { return mws_to<T>((*this)[i_idx]); }
      virtual size_t size() const;
      virtual bool is_leaf() const;
      virtual bool is_node() const;
      virtual void del_val() const;
      virtual void del_val_at_idx(uint32 i_idx) const;
      virtual bool operator==(const kv_ref& i_ref) const;
      virtual kv_ref operator[](uint32 i_idx) const;
      // gets a reference to an elem that's inside the given elem
      // i_path is like xxx.yyy.zzz.etc
      virtual kv_ref operator[](const std::string& i_path) const;
      // fast way to get the first value in a node list
      // for example, for "default.start.mod-list" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "mod-list"
      // returns i_default_val if path is empty or does not exist
      virtual std::string val_at(const std::string& i_path, const std::string& i_default_val = "") const;
      // fast way to get the leaf list of node-leaf-list elems
      // for example, for "default.start" in "default[ start[ mod-list[ kxmd ], exclusive [ true ], launch-mod [ true ], ], ]", returns "[ mod-list, exclusive, launch-mod, ]"
      // returns i_default_val if path is empty or does not exist
      virtual std::vector<std::string> val_seq_at(const std::string& i_path, const std::vector<std::string>& i_default_val = {}) const;
      // checks if path exists, starting from a given root
      // path is like xxx.yyy.zzz.etc
      virtual bool path_exists(const std::string& i_path) const;
      virtual void set_key(const std::string& i_key) const;
      virtual kv_ref push_back(const std::string& i_key) const;
      virtual void push_back(const std::vector<std::string>& i_list) const;
      virtual kv_ref find_by_key(const std::string& i_key, bool i_recursive = false) const;
      virtual std::string to_str_inc_self() const;
      virtual std::string to_str() const;

   private:
      friend class kxmd_impl;
      kv_ref(mws_sp<kxmd> i_db, mws_sp<kxmd_kv> i_kv);

      mws_wp<kxmd> db;
      mws_wp<kxmd_kv> kv;
   };


   class kxmd
   {
   public:
      static mws_sp<kxmd> nwi(const char* i_kxmd_data, uint32 i_size);
      static mws_sp<kxmd> nwi(const std::string& i_kxmd_data = "");
      static mws_sp<kxmd> nwi_from_file(const std::string& i_filename = "");
      ~kxmd();
      kv_ref main() const;

   private:
      friend class kxmd_impl;
      kxmd();

      kxmd_impl* p = nullptr;
   };
}
using ns_kxmd::kv_ref;
using ns_kxmd::kxmd;
using ns_kxmd::kxmd_kv;
