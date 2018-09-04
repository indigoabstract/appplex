#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "kxmd.hpp"
#include <kxmd/kx-krte.hpp>
#include <kxmd/kx-elem.hpp>


std::vector<mws_sp<kx_process> > kxmd_ops::get_process_list(const mws_sp<kx_process> ikp)
{
   std::vector<mws_sp<kx_process> > list;

   if (ikp->type == kxe_block)
   {
      auto block = static_pointer_cast<kx_block>(ikp);

      for (auto& i : block->list)
      {
         if (i->is_process())
         {
            list.push_back(static_pointer_cast<kx_process>(i));
         }
      }
   }

   return list;
}

std::vector<std::string> kxmd_ops::get_process_name_list(const std::vector<mws_sp<kx_process> >& ilist)
{
   std::vector<std::string> list;

   for (auto& i : ilist)
   {
      list.push_back(i->get_name());
   }

   return list;
}


std::vector<std::string> kxmd_ops::get_process_name_list(const mws_sp<kx_process> ikp)
{
   std::vector<mws_sp<kx_process> > l1 = get_process_list(ikp);
   std::vector<std::string> l2;

   if (!l1.empty())
   {
      l2 = get_process_name_list(l1);
   }

   return l2;
}


bool kxmd_ops::get_bool_from_list(const std::vector<std::string>& ilist)
{
   if (ilist.empty())
   {
      mws_throw ia_exception("list is empty");
   }

   if (ilist[0] == "false")
   {
      return false;
   }
   else if (ilist[0] == "true")
   {
      return true;
   }

   mws_throw ia_exception("parse error");
}


// i_path is like xxx.yyy.zzz
mws_sp<kx_process> kxmd_ops::get_inner_block(std::string i_path, mws_sp<kx_process> i_root, bool i_recursive)
{
   mws_sp<kx_process> xdb;

   if (i_root->type == kxe_block)
   {
      auto block = static_pointer_cast<kx_block>(i_root);
      std::vector<std::string> tokens;
      tokens = str_split(i_path, ".");
      xdb = i_root;

      if (i_path == "units.kappaxx.platf")
      {
         int x = 3;
      }
      for (auto& xdb_name : tokens)
      {
         auto sub_xdb = xdb->find_by_name(xdb_name, i_recursive);
         xdb = sub_xdb;

         if (!xdb)
         {
            return nullptr;
         }
      }
   }

   return xdb;
}

mws_any kxmd_ops::get_value(std::string i_path, mws_sp<kx_process> i_root, mws_any i_default_val)
{
   mws_sp<kx_process> px = get_inner_block(i_path, i_root);
   mws_any result;

   if (px)
   {
      auto values = get_process_name_list(px);
      result = values;
   }
   else
   {
      result = i_default_val;
   }

   return result;
}

std::vector<std::string> kxmd_ops::get_kxmd_str_seq(std::string i_path, mws_sp<kx_process> i_root, std::vector<std::string> i_default_val)
{
   std::vector<std::string> seq;
   mws_any val = get_value(i_path, i_root);

   if (!val.empty())
   {
      mws_try
      {
         std::function<int(int)> is_quote = [](int c) { return c == '\'' || c == '"'; };

         seq = mws_any_cast<std::vector<std::string>>(val);

         // clear ' and " from the string
         for (std::string& s : seq)
         {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(is_quote)));
            s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(is_quote)).base(), s.end());
         }
      }
         mws_catch(mws_bad_any_cast& e)
      {
         trx(e.what());
      }
   }

   if (seq.empty())
   {
      seq = i_default_val;
   }

   return seq;
}


// i_path is like xxx.yyy.zzz
bool kxmd_ops::kxmd_path_exists(std::string i_path, mws_sp<kx_process> i_root)
{
   if (i_root->type == kxe_block)
   {
      std::size_t found = i_path.find_last_of(".");

      if (found > 0)
      {
         std::string stem = i_path.substr(0, found);
         std::string leaf = i_path.substr(found + 1, i_path.length() - found - 1);
         auto seq = get_kxmd_str_seq(stem, i_root);
         auto idx = std::find(seq.begin(), seq.end(), leaf);

         if (idx != seq.end())
         {
            return true;
         }
      }
   }

   return false;
}


std::string kxmd_ops::as_string(mws_any const& i_val)
{
   std::string str = mws_any_cast<std::string>(i_val);
   return str;
}

#endif
