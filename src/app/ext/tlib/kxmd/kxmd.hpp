#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "pfm.hpp"
#include "min.hpp"
#include <string>
#include <vector>

class kx_process;
class kx_block;
class kx_elem;


class kxmd_ops
{
public:
   // gets the direct subblock list of the given block, if any exist
   static std::vector<mws_sp<kx_process> > get_process_list(const mws_sp<kx_process> ikp);

   static std::vector<std::string> get_process_name_list(const std::vector<mws_sp<kx_process> >& i_list);
   static std::vector<std::string> get_process_name_list(const mws_sp<kx_process> ikp);
   static bool get_bool_from_list(const std::vector<std::string>& i_list);

   // gets a reference to a block that's inside the given block
   // i_path is like xxx.yyy.zzz
   // if recursive, searches for given path not only immediately under root, but also in all the subnodes
   static mws_sp<kx_process> get_inner_block(std::string i_path, mws_sp<kx_process> i_root, bool i_recursive = false);

   static mws_any get_value(std::string i_path, mws_sp<kx_process> i_root, mws_any i_default_val = mws_any());

   static std::vector<std::string> get_kxmd_str_seq(std::string i_path, mws_sp<kx_process> i_root, std::vector<std::string> i_default_val = {});

   // checks if path exists, starting from a given root
   // path is like xxx.yyy.zzz
   static bool kxmd_path_exists(std::string i_path, mws_sp<kx_process> i_root);

   static std::string as_string(mws_any const& i_val);

   static void push_val(mws_sp<kx_block> i_block, const mws_sp<kx_elem> i_val);
   static void push_val(mws_sp<kx_block> i_block, const std::string& i_val);
   static void push_val(mws_sp<kx_block> i_block, const std::vector<std::string>& i_list);
};


static const std::string indent_str_2 = "    ";
class kxmd_elem
{
public:
   std::string val;
   std::vector<mws_sp<kxmd_elem>> vect;

   static mws_sp<kxmd_elem> nwi() { return mws_sp<kxmd_elem>(new kxmd_elem()); }
   int elem_count() const { return vect.size(); }
   bool is_leaf() const { return vect.empty() && !val.empty(); };
   bool is_node() const { return !vect.empty(); };

   std::string to_string() const
   {
      return to_string_impl(0);
   }

   std::string to_string_list() const
   {
      std::string s;
      int size = vect.size();

      for (int k = 0; k < size; k++)
      {
         auto& ke = vect[k];
         s += ke->to_string();

         if (k < (size - 1))
         {
            s += ", ";

            if (ke->is_node())
            {
               s += "\n";
            }
         }
      }

      return s;
   }

   std::string indent_by_level(int i_level) const
   {
      std::string ret;

      for (int k = 0; k < i_level; k++)
      {
         ret += indent_str_2;
      }

      return ret;
   }

   std::string to_string_impl(int i_level) const
   {
      std::string s;

      if (!val.empty())
      {
         s += val;
      }

      if (!vect.empty())
      {
         int size = vect.size();

         s += "\n";
         s += indent_by_level(i_level);
         s += "[\n";
         s += indent_by_level(i_level + 1);

         for (int k = 0; k < size; k++)
         {
            auto& ke = vect[k];
            s += ke->to_string_impl(i_level + 1);

            if (k < (size - 1))
            {
               s += ", ";

               if (ke->is_node())
               {
                  s += "\n";
                  s += indent_by_level(i_level + 1);
               }
            }
         }

         s += "\n";
         s += indent_by_level(i_level);
         s += "]";
      }

      return s;
   }

protected:
   kxmd_elem() {}
};


class kxmd_parser
{
public:
   mws_sp<kxmd_elem> parse(mws_sp<std::string> i_src);
};

#endif
