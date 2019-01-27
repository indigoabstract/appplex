#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "kxmd.hpp"
#include <kxmd/kx-krte.hpp>
#include <kxmd/kx-elem.hpp>


using std::string;
using std::vector;


class kxmd_scn;
class kxmd_scn_whitespace;
class kxmd_scn_comma;
class kxmd_scn_symbol;
class kxmd_scn_text;
class kxmd_scn_async_flowop;
class kxmd_scn_flowop;
class kxmd_scn_match_block;
class kxmd_scn_meta_block;
class kxmd_scn_ignore_block;
class kxmd_scn_block;
class kxmd_scn_main;


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

std::vector<std::string> kxmd_ops::get_process_name_list(const std::vector<mws_sp<kx_process> >& i_list)
{
   std::vector<std::string> list;

   for (auto& i : i_list)
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


bool kxmd_ops::get_bool_from_list(const std::vector<std::string>& i_list)
{
   if (i_list.empty())
   {
      mws_throw mws_exception("list is empty");
   }

   if (i_list[0] == "false")
   {
      return false;
   }
   else if (i_list[0] == "true")
   {
      return true;
   }

   mws_throw mws_exception("parse error");
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

void kxmd_ops::push_val(mws_sp<kx_block> i_block, const mws_sp<kx_elem> i_val)
{
   i_block->list.push_back(i_val);
}

void kxmd_ops::push_val(mws_sp<kx_block> i_block, const std::string& i_val)
{
   auto val = kx_symbol::nwi();

   val->name = i_val;
   i_block->list.push_back(val);
}

void kxmd_ops::push_val(mws_sp<kx_block> i_block, const std::vector<std::string>& i_list)
{
   for (auto& str : i_list)
   {
      auto val = kx_symbol::nwi();

      val->name = str;
      i_block->list.push_back(val);
   }
}


mws_sp<kxmd_elem> kxmd::get_elem(const std::string& i_path, mws_sp<kxmd_elem> i_root)
{
   mws_sp<kxmd_elem> ke = i_root;
   std::vector<std::string> tokens = str_split(i_path, ".");

   for (auto& ke_name : tokens)
   {
      ke = ke->find_by_name(ke_name, false);

      if (!ke)
      {
         return nullptr;
      }
   }

   return ke;
}

bool kxmd::path_exists(const std::string& i_path, mws_sp<kxmd_elem> i_root)
{
   mws_sp<kxmd_elem> ke = i_root;
   std::vector<std::string> tokens = str_split(i_path, ".");

   for (auto& ke_name : tokens)
   {
      ke = ke->find_by_name(ke_name, false);

      if (!ke)
      {
         return false;
      }
   }

   return true;
}

void kxmd::push_back(mws_sp<kxmd_elem> i_root, const mws_sp<kxmd_elem> i_val)
{
   i_root->vect.push_back(i_val);
}

void kxmd::push_back(mws_sp<kxmd_elem> i_root, const std::string& i_val)
{
   auto val = kxmd_elem::nwi();

   val->val = i_val;
   i_root->vect.push_back(val);
}

void kxmd::push_back(mws_sp<kxmd_elem> i_root, const std::vector<std::string>& i_list)
{
   for (auto& str : i_list)
   {
      auto val = kxmd_elem::nwi();

      val->val = str;
      i_root->vect.push_back(val);
   }
}


mws_sp<kxmd_elem> kxmd_elem::find_by_name(const std::string& i_name, bool i_recursive) const
{
   for (auto& ke : vect)
   {
      if (ke)
      {
         if (ke->val == i_name)
         {
            return ke;
         }

         if (i_recursive)
         {
            mws_sp<kxmd_elem> tke = ke->find_by_name(i_name, true);

            if (tke)
            {
               return tke;
            }
         }
      }
   }

   return nullptr;
}

std::string kxmd_elem::to_string() const
{
   return to_string_impl(0);
}

std::string kxmd_elem::to_string_list() const
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

std::string kxmd_elem::indent_by_level(int i_level) const
{
   static const std::string indent_str = "    ";
   std::string ret;

   for (int k = 0; k < i_level; k++)
   {
      ret += indent_str;
   }

   return ret;
}

std::string kxmd_elem::to_string_impl(int i_level) const
{
   std::string s;

   if (!val.empty())
   {
      s += val;
   }

   if (!vect.empty())
   {
      int size = vect.size();

      if (!val.empty())
      {
         s += "\n";
         s += indent_by_level(i_level);
      }

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



enum class kxmd_elem_type
{
   kxe_invalid,
   kxe_block,
   kxe_symbol,
   kxe_text,
   kxe_async_flowop,
   kxe_flowop, // relation op?
   kxe_ignore_block,
   kxe_match_block,
   kxe_meta_block,
   kxe_whitespace,
   kxe_comma,
};
class kxmd_process;

class kxs_elem : public kxmd_elem
{
public:
   virtual kxmd_elem_type get_type() const { return kxmd_elem_type::kxe_invalid; }

protected:
   kxs_elem() {}
};


class kxmd_whitespace : public kxs_elem
{
public:
   static mws_sp<kxmd_whitespace> nwi() { return mws_sp<kxmd_whitespace>(new kxmd_whitespace()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_whitespace; }
   virtual bool is_leaf() const override { return false; };
   virtual bool is_node() const override { return false; };

private:
   kxmd_whitespace() {}
};


class kxmd_comma : public kxs_elem
{
public:
   static mws_sp<kxmd_comma> nwi() { return mws_sp<kxmd_comma>(new kxmd_comma()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_comma; }
   virtual bool is_leaf() const override { return false; };
   virtual bool is_node() const override { return false; };

private:
   kxmd_comma() {}
};


class kxmd_async_flowop : public kxs_elem
{
public:
   enum aflow_types
   {
      afl_left,
      afl_right,
   };

   static mws_sp<kxmd_async_flowop> nwi() { return mws_sp<kxmd_async_flowop>(new kxmd_async_flowop()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_async_flowop; }

   int fltype;
   std::vector<mws_sp<kxmd_process> > cnx;
   int capacity;

private:
   kxmd_async_flowop()
   {
      fltype = afl_left;
      capacity = 1;
   }
};

class kxmd_flowop : public kxs_elem
{
public:
   enum flow_types
   {
      fl_left,
      fl_right,
   };

   static mws_sp<kxmd_flowop> nwi() { return mws_sp<kxmd_flowop>(new kxmd_flowop()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_flowop; }

   int fltype;
   mws_wp<kxmd_process> src, dst;
   std::vector<mws_sp<kxmd_process> > cnx;
   int capacity;

private:
   kxmd_flowop()
   {
      fltype = fl_left;
      capacity = 1;
   }
};


class kxmd_process : public kxs_elem
{
public:
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_invalid; }
   virtual std::string get_name()const = 0;

protected:
   kxmd_process() {}
};


class kxmd_symbol : public kxmd_process
{
public:
   static mws_sp<kxmd_symbol> nwi() { return mws_sp<kxmd_symbol>(new kxmd_symbol()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_symbol; }
   virtual std::string get_name()const { return val; }

private:
   kxmd_symbol() {}
};


class kxmd_text : public kxmd_process
{
public:
   static mws_sp<kxmd_text> nwi() { return mws_sp<kxmd_text>(new kxmd_text()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_text; }
   virtual std::string get_name()const { return val; }

private:
   kxmd_text() {}
};


class kxmd_block : public kxmd_process
{
public:
   static mws_sp<kxmd_block> nwi() { return mws_sp<kxmd_block>(new kxmd_block()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_block; }
   virtual std::string get_name()const
   {
      if (val.empty()) { return "block-nn/a"; }
      return val;
   }

protected:
   kxmd_block() {}
};


class kxmd_ignore_block : public kxmd_process
{
public:
   static mws_sp<kxmd_ignore_block> nwi() { return mws_sp<kxmd_ignore_block>(new kxmd_ignore_block()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_ignore_block; }
   virtual std::string get_name()const { return "comment"; }

private:
   kxmd_ignore_block() {}
};


class kxmd_match_block : public kxmd_process
{
public:
   static mws_sp<kxmd_match_block> nwi() { return mws_sp<kxmd_match_block>(new kxmd_match_block()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_match_block; }
   virtual std::string get_name()const { return "match-block"; }

private:
   kxmd_match_block() {}
};


class kxmd_meta_block : public kxmd_process
{
public:
   static mws_sp<kxmd_meta_block> nwi() { return mws_sp<kxmd_meta_block>(new kxmd_meta_block()); }
   virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_meta_block; }
   virtual std::string get_name()const { return "meta-block"; }

private:
   kxmd_meta_block() {}
};


class kxmd_util
{
public:
   bool static is_white_space(char c)
   {
      return std::isspace(c);
   }

   bool static is_symbol_start_char(char c)
   {
      bool is_symbol = false;

      if ((c >= 48 && c <= 57) || (c >= 'A' && c <= 'Z') || c == '_' || (c >= 'a' && c <= 'z'))
      {
         is_symbol = true;
      }

      return is_symbol;
   }

   bool static is_symbol_body_char(char c)
   {
      bool is_symbol = false;

      if (c >= '!' && c <= 'z')
      {
         is_symbol = true;
      }

      switch (c)
      {
      case '"':
      case '\'':
      case '(':
      case ')':
      case ',':
      case '[':
      case ']':
      case '{':
      case '}':
         is_symbol = false;
         break;
      }

      return is_symbol;
   }
};


// scanner
class kxmd_shared_state : public enable_shared_from_this < kxmd_shared_state >
{
public:
   static mws_sp<kxmd_shared_state> nwi() { return mws_sp<kxmd_shared_state>(new kxmd_shared_state()); }
   mws_sp<kxmd_shared_state> get_instance() { return shared_from_this(); }

   mws_sp<kxmd_shared_state> clone()
   {
      mws_sp<kxmd_shared_state> ss = kxmd_shared_state::nwi();

      *ss = *get_instance();

      return ss;
   }

   virtual ~kxmd_shared_state() {}

   bool is_end_of_line()
   {
      return crt_idx >= src->length();
   }

   mws_sp<std::string> src;
   int crt_idx;
   mws_sp<kxs_elem> kxel;

private:
   kxmd_shared_state() { crt_idx = 0; }
};


enum kxmd_scanner_type
{
   kxs_main,
   kxs_block,
   kxs_symbol,
   kxs_text,
   kxs_async_flowop,
   kxs_flowop,
   kxs_ignore_block,
   kxs_ignore_block_body,
   kxs_match_block,
   kxs_meta_block,
   kxs_whitespace,
   kxs_comma,
};


class kxmd_scn_factory
{
public:
   static mws_sp<kxmd_scn> nwi(kxmd_scanner_type type, mws_sp<kxmd_shared_state> ss);
};


class kxmd_scn
{
public:
   virtual ~kxmd_scn() {}

   mws_sp<kxs_elem> scan()
   {
      if (ss->crt_idx < ss->src->length())
      {
         return scan_impl();
      }

      return nullptr;
   }

   void set_state(mws_sp<kxmd_shared_state> iss)
   {
      ss = iss;
   }

   virtual mws_sp<kxs_elem> scan_impl() { return nullptr; }

   mws_sp<kxmd_shared_state> ss;
   bool token_found;

protected:
   kxmd_scn() { token_found = false; }
};


class kxmd_scn_whitespace : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_whitespace> nwi() { return mws_sp<kxmd_scn_whitespace>(new kxmd_scn_whitespace()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;

      for (int k = start_idx; k < ss->src->length(); k++)
      {
         char c = ss->src->at(k);

         if (kxmd_util::is_white_space(c))
         {
            ss->crt_idx++;
         }
         else
         {
            if (ss->crt_idx > start_idx)
            {
               token_found = true;

               mws_sp<kxmd_whitespace> ke = kxmd_whitespace::nwi();
               ke->val = ss->src->substr(start_idx, ss->crt_idx - start_idx);

               return ke;
            }

            break;
         }
      }

      return nullptr;
   }
};


class kxmd_scn_comma : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_comma> nwi() { return mws_sp<kxmd_scn_comma>(new kxmd_scn_comma()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (c == ',')
      {
         ss->crt_idx++;
         token_found = true;

         return kxmd_comma::nwi();
      }

      return nullptr;
   }
};


class kxmd_scn_symbol : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_symbol> nwi() { return mws_sp<kxmd_scn_symbol>(new kxmd_scn_symbol()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (!kxmd_util::is_symbol_start_char(c))
      {
         return nullptr;
      }

      ss->crt_idx++;

      for (int k = ss->crt_idx; k < ss->src->length(); k++)
      {
         c = ss->src->at(k);

         if (kxmd_util::is_symbol_body_char(c))
         {
            ss->crt_idx++;
         }
         else
         {
            break;
         }
      }

      token_found = true;

      mws_sp<kxmd_symbol> ke = kxmd_symbol::nwi();
      ke->val = ss->src->substr(start_idx, ss->crt_idx - start_idx);

      return ke;
   }
};


class kxmd_scn_text : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_text> nwi() { return mws_sp<kxmd_scn_text>(new kxmd_scn_text()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (c != '"' && c != '\'')
      {
         return nullptr;
      }

      char end = c;
      ss->crt_idx++;

      for (int k = ss->crt_idx; k < ss->src->length(); k++)
      {
         char c = ss->src->at(k);

         if (c == end)
         {
            ss->crt_idx++;
            token_found = true;

            mws_sp<kxmd_text> ke = kxmd_text::nwi();
            ke->val = ss->src->substr(start_idx, ss->crt_idx - start_idx);

            return ke;
         }
         else
         {
            ss->crt_idx++;
         }
      }

      if (!token_found)
      {
         mws_throw mws_exception("unterminated text quote");
      }

      return nullptr;
   }
};


class kxmd_scn_async_flowop : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_async_flowop> nwi() { return mws_sp<kxmd_scn_async_flowop>(new kxmd_scn_async_flowop()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (c == '<')
      {
         c = ss->src->at(start_idx + 1);

         if (c == '<')
         {
            c = ss->src->at(start_idx + 2);

            if (c == '-')
            {
               ss->crt_idx += 3;
               token_found = true;

               mws_sp<kxmd_async_flowop> ke = kxmd_async_flowop::nwi();
               ke->fltype = kxmd_async_flowop::afl_left;

               return ke;
            }
         }
      }
      else if (c == '-')
      {
         c = ss->src->at(start_idx + 1);

         if (c == '-')
         {
            c = ss->src->at(start_idx + 2);

            if (c == '>')
            {
               ss->crt_idx += 2;
               token_found = true;

               mws_sp<kxmd_async_flowop> ke = kxmd_async_flowop::nwi();
               ke->fltype = kxmd_async_flowop::afl_right;

               return ke;
            }
         }
      }

      return nullptr;
   }
};


class kxmd_scn_flowop : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_flowop> nwi() { return mws_sp<kxmd_scn_flowop>(new kxmd_scn_flowop()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (c == '<')
      {
         c = ss->src->at(start_idx + 1);

         if (c == '-')
         {
            ss->crt_idx += 2;
            token_found = true;

            mws_sp<kxmd_flowop> ke = kxmd_flowop::nwi();
            ke->fltype = kxmd_flowop::fl_left;

            return ke;
         }
      }
      else if (c == '-')
      {
         c = ss->src->at(start_idx + 1);

         if (c == '>')
         {
            ss->crt_idx += 2;
            token_found = true;

            mws_sp<kxmd_flowop> ke = kxmd_flowop::nwi();
            ke->fltype = kxmd_flowop::fl_right;

            return ke;
         }
      }

      return nullptr;
   }
};


class kxmd_scn_match_block : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_match_block> nwi() { return mws_sp<kxmd_scn_match_block>(new kxmd_scn_match_block()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = 0;
      mws_sp<kxmd_scn> sc;

      c = ss->src->at(start_idx);

      if (c != '?')
      {
         return nullptr;
      }

      mws_sp<kxmd_match_block> ke = kxmd_match_block::nwi();
      mws_sp<kxs_elem> kxt;

      ss->crt_idx++;
      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      sc = kxmd_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->val = kxt->val;
      }

      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a '['", start_idx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      ss->crt_idx++;

      sc = kxmd_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a ']'", start_idx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke->vect.push_back(kxt);
      }

      ss->crt_idx++;
      token_found = true;

      return ke;
   }
};


class kxmd_scn_meta_block : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_meta_block> nwi() { return mws_sp<kxmd_scn_meta_block>(new kxmd_scn_meta_block()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = 0;
      mws_sp<kxmd_scn> sc;

      c = ss->src->at(start_idx);

      if (c != '@')
      {
         return nullptr;
      }

      c = ss->src->at(start_idx + 1);

      if (c == '@')
      {
         return nullptr;
      }

      mws_sp<kxmd_meta_block> ke = kxmd_meta_block::nwi();
      mws_sp<kxs_elem> kxt;

      ss->crt_idx++;
      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      sc = kxmd_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->val = kxt->val;
      }

      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      int idx = ss->crt_idx;
      sc = kxmd_scn_factory::nwi(kxs_comma, ss);
      kxt = sc->scan();

      if (sc->token_found)
      {
         ss->crt_idx = idx;
         token_found = true;

         return ke;
      }

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, '[');
         mws_throw mws_exception(msg);
      }

      ss->crt_idx++;

      sc = kxmd_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, ']');
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke->vect.push_back(kxt);
      }

      ss->crt_idx++;
      token_found = true;

      return ke;
   }
};


class kxmd_scn_ignore_block : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_ignore_block> nwi() { return mws_sp<kxmd_scn_ignore_block>(new kxmd_scn_ignore_block()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = 0;
      mws_sp<kxmd_scn> sc;

      c = ss->src->at(start_idx);

      if (c != '@')
      {
         return nullptr;
      }

      c = ss->src->at(start_idx + 1);

      if (c != '@')
      {
         return nullptr;
      }

      mws_sp<kxmd_ignore_block> ke = kxmd_ignore_block::nwi();
      mws_sp<kxs_elem> kxt;

      ss->crt_idx += 2;
      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      sc = kxmd_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->val = kxt->val;
      }

      sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      int idx = ss->crt_idx;
      sc = kxmd_scn_factory::nwi(kxs_comma, ss);
      kxt = sc->scan();

      if (sc->token_found || ss->is_end_of_line())
      {
         ss->crt_idx = idx;
         token_found = true;

         return ke;
      }

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("ignoreblock parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, '[');
         mws_throw mws_exception(msg);
      }

      sc = kxmd_scn_factory::nwi(kxs_ignore_block_body, ss);
      kxt = sc->scan();

      if (!sc->token_found)
      {
         string msg = trs("ignoreblock-body parse error");
         mws_throw mws_exception(msg);
      }

      mws_sp<kxmd_text> body = static_pointer_cast<kxmd_text>(kxt);
      mws_sp<kxmd_elem> text = kxmd_elem::nwi();
      ke->vect.push_back(text);
      text->val = body->val;
      token_found = true;

      return ke;
   }
};


class kxmd_scn_ignore_block_body : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_ignore_block_body> nwi() { return mws_sp<kxmd_scn_ignore_block_body>(new kxmd_scn_ignore_block_body()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;

      if (ss->src->at(start_idx) != '[')
      {
         return nullptr;
      }

      ss->crt_idx++;

      kxmd_scanner_type sct[] =
      {
         kxs_whitespace,
         kxs_ignore_block_body,
         //kxs_text,
      };

      // search for closing ']'
      while (ss->crt_idx < ss->src->length())
      {
         bool ttoken_found = false;

         do
         {
            ttoken_found = false;

            // check every character for start / end of text / blocks pairs : {", ', [, ]},
            // to make sure they are matched correctly. ignore everything else
            for (auto et : sct)
            {
               mws_sp<kxmd_scn> scn = kxmd_scn_factory::nwi(et, ss);
               scn->scan();

               if (scn->token_found)
               {
                  ttoken_found = true;
               }

               if (ss->crt_idx >= ss->src->length())
               {
                  if (ss->crt_idx > ss->src->length())
                  {
                     mws_throw mws_exception("kxscnignoreblock_body - passed the end of the string. this shouldn't happen...");
                  }

                  string msg = trs("ex 1 mismatched block started at {}", start_idx + 1);
                  mws_throw mws_exception(msg);
               }
            }
         } while (ttoken_found);

         // no more recognized sequences. check for end of block, or skip character
         char c = ss->src->at(ss->crt_idx);

         // found the end of the block
         if (c == ']')
         {
            ss->crt_idx++;
            token_found = true;
            break;
         }
         // not a whitespace, start/end of block/text, just ignore and move on
         else
         {
            ss->crt_idx++;
         }
      }

      if (!token_found)
      {
         string msg = trs("ex 2 mismatched block started at {}", start_idx + 1);
         mws_throw mws_exception(msg);
      }

      mws_sp<kxmd_text> ke = kxmd_text::nwi();
      ke->val = ss->src->substr(start_idx + 1, ss->crt_idx - start_idx - 2);

      return ke;
   }
};


class kxmd_scn_block : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_block> nwi() { return mws_sp<kxmd_scn_block>(new kxmd_scn_block()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      if (c != '[')
      {
         return nullptr;
      }

      mws_sp<kxmd_block> ke;
      mws_sp<kxs_elem> kxt;

      ss->crt_idx++;

      mws_sp<kxmd_scn> sc = kxmd_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("block parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, ']');
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke = static_pointer_cast<kxmd_block>(kxt);
      }
      else
      {
         ke = kxmd_block::nwi();
      }

      ss->crt_idx++;
      token_found = true;

      return ke;
   }
};


class kxmd_scn_main : public kxmd_scn
{
public:
   static mws_sp<kxmd_scn_main> nwi() { return mws_sp<kxmd_scn_main>(new kxmd_scn_main()); }

   virtual mws_sp<kxs_elem> scan_impl()
   {
      int start_idx = ss->crt_idx;
      char c = ss->src->at(start_idx);

      kxmd_scanner_type sct[] =
      {
         kxs_whitespace,
         kxs_comma,
         kxs_symbol,
         kxs_block,
         kxs_text,
         kxs_async_flowop,
         kxs_flowop,
         kxs_ignore_block,
         kxs_match_block,
         kxs_meta_block,
      };

      mws_sp<kxmd_block> ke = kxmd_block::nwi();
      mws_sp<kxs_elem> kxt;

      while (!ss->is_end_of_line())
      {
         bool ttoken_found = false;

         for (auto et : sct)
         {
            mws_sp<kxmd_scn> scn = kxmd_scn_factory::nwi(et, ss);
            kxt = scn->scan();

            if (scn->token_found)
            {
               ttoken_found = true;

               // find out if this block has a name
               if (kxt->is_node() && ke->vect.size() >= 1)
               {
                  mws_sp<kxmd_block> kb = static_pointer_cast<kxmd_block>(kxt);

                  if (ke->vect.back()->is_leaf())
                  {
                     kb->val = ke->vect.back()->val;
                     ke->vect.pop_back();
                  }
               }

               // discard everything except nodes and leaves
               if (kxt->is_leaf() || kxt->is_node())
               {
                  ke->vect.push_back(kxt);
               }
            }

            if (ss->crt_idx >= ss->src->length())
            {
               if (ss->crt_idx > ss->src->length())
               {
                  mws_throw mws_exception("passed the end of the string. this shouldn't happen...");
               }
               break;
            }
         }

         if (!ttoken_found)
         {
            break;
         }
      }

      int idx = ss->crt_idx;
      bool parse_error = false;

      if (idx >= ss->src->length())
      {
         idx = ss->src->length() - 1;
      }

      c = ss->src->at(idx);

      // scanner for the whole program/file
      if (start_idx == 0)
      {
         if (ss->crt_idx != ss->src->length())
         {
            parse_error = true;
         }
      }
      // scanner for a block
      else
      {
         if (c != ']')
         {
            parse_error = true;
         }
      }

      if (parse_error)
      {
         string msg = trs("main line scan error {0}-{1}. unknown token '{2}'", start_idx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      token_found = true;

      return ke;
   }
};


mws_sp<kxmd_scn> kxmd_scn_factory::nwi(kxmd_scanner_type type, mws_sp<kxmd_shared_state> ss)
{
   mws_sp<kxmd_scn> inst;

   switch (type)
   {
   case kxs_main:
      inst = kxmd_scn_main::nwi();
      break;

   case kxs_block:
      inst = kxmd_scn_block::nwi();
      break;

   case kxs_symbol:
      inst = kxmd_scn_symbol::nwi();
      break;

   case kxs_text:
      inst = kxmd_scn_text::nwi();
      break;

   case kxs_async_flowop:
      inst = kxmd_scn_async_flowop::nwi();
      break;

   case kxs_flowop:
      inst = kxmd_scn_flowop::nwi();
      break;

   case kxs_ignore_block:
      inst = kxmd_scn_ignore_block::nwi();
      break;

   case kxs_ignore_block_body:
      inst = kxmd_scn_ignore_block_body::nwi();
      break;

   case kxs_match_block:
      inst = kxmd_scn_match_block::nwi();
      break;

   case kxs_meta_block:
      inst = kxmd_scn_meta_block::nwi();
      break;

   case kxs_whitespace:
      inst = kxmd_scn_whitespace::nwi();
      break;

   case kxs_comma:
      inst = kxmd_scn_comma::nwi();
      break;
   }

   if (inst)
   {
      inst->set_state(ss);
   }

   return inst;
}


mws_sp<kxmd_elem> kxmd::parse(mws_sp<std::string> src)
{
   mws_sp<kxmd_scn> sc;
   mws_sp<kxmd_shared_state> ss;

   ss = kxmd_shared_state::nwi();
   ss->src = src;
   sc = kxmd_scn_factory::nwi(kxs_main, ss);

   return sc->scan();
}

#endif
