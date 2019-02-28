#pragma once

#include "pfm-def.h"
#include <string>
#include <vector>


struct kxmd_elem
{
   std::string val;
   std::vector<mws_sp<kxmd_elem>> vect;
};


class kxmd
{
public:
   static mws_sp<kxmd_elem> nwi() { return mws_sp<kxmd_elem>(new kxmd_elem()); }
   static size_t size(mws_sp<kxmd_elem> i_r) { return i_r->vect.size(); }
   static bool is_leaf(mws_sp<kxmd_elem> i_r) { return i_r->vect.empty() && !i_r->val.empty(); };
   static bool is_node(mws_sp<kxmd_elem> i_r) { return !i_r->vect.empty(); };
   // gets a reference to a kxmd_elem that's inside the given kxmd_elem
   // i_path is like xxx.yyy.zzz.etc
   static mws_sp<kxmd_elem> elem_at(mws_sp<kxmd_elem> i_r, const std::string& i_path);
   // fast way to get the first value in a node list
   // for example, for "default.start.mod-list" in "default[ start[ mod-list[ kxmdb ], exclusive [ true ], launch-mod [ true ], ], ]", returns "mod-list"
   // returns i_default_val if path is empty or does not exist
   static std::string val_at(mws_sp<kxmd_elem> i_r, const std::string& i_path, const std::string& i_default_val = "");
   // fast way to get the leaf list of node-leaf-list elems
   // for example, for "default.start" in "default[ start[ mod-list[ kxmdb ], exclusive [ true ], launch-mod [ true ], ], ]", returns "[ mod-list, exclusive, launch-mod, ]"
   // returns i_default_val if path is empty or does not exist
   static std::vector<std::string> val_seq_at(mws_sp<kxmd_elem> i_r, const std::string& i_path, const std::vector<std::string>& i_default_val = {});
   // checks if path exists, starting from a given root
   // path is like xxx.yyy.zzz.etc
   static bool path_exists(mws_sp<kxmd_elem> i_r, const std::string& i_path);
   static void push_back(mws_sp<kxmd_elem> i_r, const mws_sp<kxmd_elem> i_val);
   static void push_back(mws_sp<kxmd_elem> i_r, const std::string& i_val);
   static void push_back(mws_sp<kxmd_elem> i_r, const std::vector<std::string>& i_list);
   static mws_sp<kxmd_elem> find_by_name(mws_sp<kxmd_elem> i_r, const std::string& i_name, bool i_recursive = false);
   static std::string to_string(mws_sp<kxmd_elem> i_r);
   static std::string to_string_list(mws_sp<kxmd_elem> i_r);
   static std::string indent_by_level(int i_level);
   static std::string to_string_impl(mws_sp<kxmd_elem> i_r, int i_level);
   static std::string strip_quotes(const std::string& i_text);
   static mws_sp<kxmd_elem> parse(mws_sp<std::string> i_src);
};
