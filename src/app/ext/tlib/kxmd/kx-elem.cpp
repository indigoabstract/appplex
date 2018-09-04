#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "kx-elem.hpp"
#include "min.hpp"

using std::string;
using std::vector;


class kx_scn;
class kx_scn_whitespace;
class kx_scn_comma;
class kx_scn_symbol;
class kx_scn_text;
class kx_scn_async_flowop;
class kx_scn_flowop;
class kx_scn_match_block;
class kx_scn_meta_block;
class kx_scn_ignore_block;
class kx_scn_block;
class kx_scn_main;


mws_sp<kx_whitespace> kx_whitespace::nwi() { return mws_sp<kx_whitespace>(new kx_whitespace()); }

string kx_whitespace::print(int ilevel) { return data; }


mws_sp<kx_comma> kx_comma::nwi() { return mws_sp<kx_comma>(new kx_comma()); }

string kx_comma::print(int ilevel) { return ", "; }


mws_sp<kx_async_flowop> kx_async_flowop::nwi() { return mws_sp<kx_async_flowop>(new kx_async_flowop()); }

string kx_async_flowop::print(int ilevel)
{
   std::string s = "->>";

   if (fltype == afl_left)
   {
      s = "<<-";
   }

   return s;
}

mws_sp<kx_flowop> kx_flowop::nwi() { return mws_sp<kx_flowop>(new kx_flowop()); }

string kx_flowop::print(int ilevel)
{
   std::string s = "->";

   if (fltype == fl_left)
   {
      s = "<-";
   }

   return s;
}


bool kx_process::is_process() { return true; }

string kx_process::print(int ilevel) { return "kx_process"; }


mws_sp<kx_symbol> kx_symbol::nwi() { return mws_sp<kx_symbol>(new kx_symbol()); }

string kx_symbol::print(int ilevel) { return name; }

void kx_symbol::eval()
{
   trx("sym[{}]", name);
}


mws_sp<kx_text> kx_text::nwi() { return mws_sp<kx_text>(new kx_text()); }

std::string kx_text::print(int ilevel) { return data; }

void kx_text::eval()
{
   trx("text[{}]", data);
}


mws_sp<kx_block> kx_block::nwi() { return mws_sp<kx_block>(new kx_block()); }

string kx_block::print(int ilevel)
{
   std::string s = "";

   if (name)
   {
      s.append(indent_by_level(ilevel) + name->name);
   }

   s.append("\n" + indent_by_level(ilevel) + "[\n");
   s.append(indent_by_level(ilevel + 1));
   for (auto ke : list)
   {
      s.append(ke->print(ilevel + 1));
   }
   s.append("\n" + indent_by_level(ilevel) + "]\n");

   return s;
}

void kx_block::eval()
{
   trc("block ");

   if (name)
   {
      trc("{}", name->name);
   }

   trc("[");

   for (auto ke : list)
   {
      ke->eval();
   }

   trc("]");
}

mws_sp<kx_process> kx_block::find_by_name(const std::string& iname, bool i_recursive) const
{
   for (auto ke : list)
   {
      if (ke->is_process())
      {
         auto px = static_pointer_cast<kx_process>(ke);

         if (px->get_name() == iname)
         {
            return px;
         }

         if (i_recursive)
         {
            mws_sp<kx_process> px_r = px->find_by_name(iname, true);

            if (px_r)
            {
               return px_r;
            }
         }
      }
   }

   return nullptr;
}

int kx_block::index_of_name(const std::string& iname) const
{
   int k = 0;

   for (auto ke : list)
   {
      if (ke->is_process())
      {
         auto px = static_pointer_cast<kx_process>(ke);

         if (px->get_name() == iname)
         {
            return k;
         }

         k++;
      }
   }

   return -1;
}



mws_sp<kx_ignore_block> kx_ignore_block::nwi() { return mws_sp<kx_ignore_block>(new kx_ignore_block()); }

string kx_ignore_block::print(int ilevel)
{
   std::string s = "@@";

   if (name)
   {
      s.append(indent_by_level(ilevel) + name->name);
   }

   if (!body.empty())
   {
      s.append(indent_by_level(ilevel) + "\n[\n");
      s.append(indent_by_level(ilevel) + body);
      s.append(indent_by_level(ilevel) + "\n]\n");
   }

   return s;
}


mws_sp<kx_match_block> kx_match_block::nwi() { return mws_sp<kx_match_block>(new kx_match_block()); }

string kx_match_block::print(int ilevel)
{
   std::string s = "?";

   if (name)
   {
      s.append(indent_by_level(ilevel) + name->name);
   }

   s.append(indent_by_level(ilevel) + "\n[\n");
   s.append(indent_by_level(ilevel));
   for (auto ke : blk->list)
   {
      s.append(ke->print(ilevel + 1));
   }
   s.append(indent_by_level(ilevel) + "\n]\n");

   return s;
}


mws_sp<kx_meta_block> kx_meta_block::nwi() { return mws_sp<kx_meta_block>(new kx_meta_block()); }

std::string kx_meta_block::print(int ilevel)
{
   std::string s = "@";

   if (name)
   {
      s.append(indent_by_level(ilevel) + name->name);
   }

   if (blk)
   {
      s.append(indent_by_level(ilevel) + "\n[\n");
      s.append(indent_by_level(ilevel));
      for (auto ke : blk->list)
      {
         s.append(ke->print(ilevel + 1));
      }
      s.append(indent_by_level(ilevel) + "\n]\n");
   }

   return s;
}

#endif
