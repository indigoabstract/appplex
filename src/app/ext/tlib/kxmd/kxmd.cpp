#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "kxmd.hpp"
#include <kxmd/kx-krte.hpp>
#include <kxmd/kx-elem.hpp>


std::vector<shared_ptr<kx_process> > kxmd_ops::get_process_list(const shared_ptr<kx_process> ikp)
{
   std::vector<shared_ptr<kx_process> > list;

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


std::vector<std::string> kxmd_ops::get_process_name_list(const std::vector<shared_ptr<kx_process> >& ilist)
{
   std::vector<std::string> list;

   for (auto& i : ilist)
   {
      list.push_back(i->get_name());
   }

   return list;
}


std::vector<std::string> kxmd_ops::get_process_name_list(const shared_ptr<kx_process> ikp)
{
   std::vector<shared_ptr<kx_process> > l1 = get_process_list(ikp);
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


// ipath is like xxx.yyy.zzz
mws_any kxmd_ops::get_kxmd_value(std::string ipath, shared_ptr<kx_block> iroot, mws_any default_val)
{
   std::vector<std::string> tokens;
   tokens = str_split(ipath, ".");
   shared_ptr<kx_process> xdb = iroot;

   if (ipath == "units.kappaxx.platf")
   {
      int x = 3;
   }
   for (auto& xdb_name : tokens)
   {
      auto sub_xdb = xdb->find_by_name(xdb_name);
      xdb = sub_xdb;

      if (!xdb)
      {
         break;
      }
   }

   mws_any result;

   if (xdb)
   {
      auto values = get_process_name_list(xdb);
      result = values;
   }
   else
   {
      result = default_val;
   }

   return result;
}

std::vector<std::string> kxmd_ops::get_kxmd_str_seq(std::string ipath, shared_ptr<kx_block> iroot, std::vector<std::string> default_val)
{
   std::vector<std::string> seq;
   mws_any val = get_kxmd_value(ipath, iroot);

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
      seq = default_val;
   }

   return seq;
}


// ipath is like xxx.yyy.zzz
bool kxmd_ops::kxmd_path_exists(std::string ipath, shared_ptr<kx_block> iroot)
{
   std::size_t found = ipath.find_last_of(".");

   if (found > 0)
   {
      std::string stem = ipath.substr(0, found);
      std::string leaf = ipath.substr(found + 1, ipath.length() - found - 1);
      auto seq = get_kxmd_str_seq(stem, iroot);
      auto idx = std::find(seq.begin(), seq.end(), leaf);

      if (idx != seq.end())
      {
         return true;
      }
   }

   return false;
}


#endif
