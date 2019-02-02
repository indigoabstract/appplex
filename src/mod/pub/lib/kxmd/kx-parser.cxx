#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_KXMD

#include "kx-parser.hxx"
#include "kx-elem.hxx"
#include "min.hxx"

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


class kx_util
{
public:
   bool static is_whiteSpace(char c)
   {
      return std::isspace(c);
   }

   bool static is_symbol_start_char(char c)
   {
      bool isSymbol = false;

      if ((c >= 48 && c <= 57) || (c >= 'A' && c <= 'Z') || c == '_' || (c >= 'a' && c <= 'z'))
      {
         isSymbol = true;
      }

      return isSymbol;
   }

   bool static is_symbol_body_char(char c)
   {
      bool isSymbol = false;

      if (c >= '!' && c <= 'z')
      {
         isSymbol = true;
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
         isSymbol = false;
         break;
      }

      return isSymbol;
   }
};


// scanner
class kx_shared_state : public enable_shared_from_this < kx_shared_state >
{
public:
   static mws_sp<kx_shared_state> nwi() { return mws_sp<kx_shared_state>(new kx_shared_state()); }
   mws_sp<kx_shared_state> get_instance() { return shared_from_this(); }

   mws_sp<kx_shared_state> clone()
   {
      mws_sp<kx_shared_state> ss = kx_shared_state::nwi();

      *ss = *get_instance();

      return ss;
   }

   virtual ~kx_shared_state() {}

   bool is_end_of_line()
   {
      return crt_idx >= src->length();
   }

   mws_sp<std::string> src;
   int start_idx;
   int crt_idx;
   mws_sp<kx_elem> kxel;

private:
   kx_shared_state() { start_idx = crt_idx = 0; }
};


enum kx_scanner_type
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


class kx_scn_factory
{
public:
   static mws_sp<kx_scn> nwi(kx_scanner_type type, mws_sp<kx_shared_state> ss);
};


class kx_scn
{
public:
   virtual ~kx_scn() {}

   mws_sp<kx_elem> scan()
   {
      if (ss->crt_idx < ss->src->length())
      {
         return scan_impl();
      }

      return mws_sp<kx_elem>();
   }

   void setState(mws_sp<kx_shared_state> iss)
   {
      ss = iss;
   }

   virtual mws_sp<kx_elem> scan_impl() { return mws_sp<kx_elem>(); }

   mws_sp<kx_shared_state> ss;
   bool tokenFound;

protected:
   kx_scn() { tokenFound = false; }
};


class kx_scn_whitespace : public kx_scn
{
public:
   static mws_sp<kx_scn_whitespace> nwi() { return mws_sp<kx_scn_whitespace>(new kx_scn_whitespace()); }
   virtual ~kx_scn_whitespace() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;

      for (int k = startidx; k < ss->src->length(); k++)
      {
         char c = ss->src->at(k);

         if (kx_util::is_whiteSpace(c))
         {
            ss->crt_idx++;
         }
         else
         {
            if (ss->crt_idx > startidx)
            {
               tokenFound = true;
               //trc("ws%1%-%2% ") % startidx % ss->crtidx;

               mws_sp<kx_whitespace> ke = kx_whitespace::nwi();
               ke->data = ss->src->substr(startidx, ss->crt_idx - startidx);

               return ke;
            }

            break;
         }
      }

      return mws_sp<kx_elem>();
   }
};


class kx_scn_comma : public kx_scn
{
public:
   static mws_sp<kx_scn_comma> nwi() { return mws_sp<kx_scn_comma>(new kx_scn_comma()); }
   virtual ~kx_scn_comma() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (c == ',')
      {
         ss->crt_idx++;
         tokenFound = true;
         //trc("comma [%1%] ") % ss->src->substr(startidx, ss->crtidx - startidx);

         return kx_comma::nwi();
      }

      return mws_sp<kx_elem>();
   }
};


class kx_scn_symbol : public kx_scn
{
public:
   static mws_sp<kx_scn_symbol> nwi() { return mws_sp<kx_scn_symbol>(new kx_scn_symbol()); }
   virtual ~kx_scn_symbol() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (!kx_util::is_symbol_start_char(c))
      {
         return mws_sp<kx_elem>();
      }

      ss->crt_idx++;

      for (int k = ss->crt_idx; k < ss->src->length(); k++)
      {
         c = ss->src->at(k);

         if (kx_util::is_symbol_body_char(c))
         {
            ss->crt_idx++;
         }
         else
         {
            break;
         }
      }

      tokenFound = true;
      //trc("sym %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      mws_sp<kx_symbol> ke = kx_symbol::nwi();
      ke->name = ss->src->substr(startidx, ss->crt_idx - startidx);

      return ke;
   }
};


class kx_scn_text : public kx_scn
{
public:
   static mws_sp<kx_scn_text> nwi() { return mws_sp<kx_scn_text>(new kx_scn_text()); }
   virtual ~kx_scn_text() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (c != '"' && c != '\'')
      {
         return mws_sp<kx_elem>();
      }

      char end = c;
      ss->crt_idx++;

      for (int k = ss->crt_idx; k < ss->src->length(); k++)
      {
         char c = ss->src->at(k);

         if (c == end)
         {
            ss->crt_idx++;
            tokenFound = true;
            //trc("txt %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

            mws_sp<kx_text> ke = kx_text::nwi();
            ke->data = ss->src->substr(startidx, ss->crt_idx - startidx);

            return ke;
         }
         else
         {
            ss->crt_idx++;
         }
      }

      if (!tokenFound)
      {
         mws_throw mws_exception("unterminated text quote");
      }

      return mws_sp<kx_elem>();
   }
};


class kx_scn_async_flowop : public kx_scn
{
public:
   static mws_sp<kx_scn_async_flowop> nwi() { return mws_sp<kx_scn_async_flowop>(new kx_scn_async_flowop()); }
   virtual ~kx_scn_async_flowop() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (c == '<')
      {
         c = ss->src->at(startidx + 1);

         if (c == '<')
         {
            c = ss->src->at(startidx + 2);

            if (c == '-')
            {
               ss->crt_idx += 3;
               tokenFound = true;
               //trc("flow [%1%] ") % ss->src->substr(startidx, ss->crtidx - startidx);

               mws_sp<kx_async_flowop> ke = kx_async_flowop::nwi();
               ke->fltype = kx_async_flowop::afl_left;

               return ke;
            }
         }
      }
      else if (c == '-')
      {
         c = ss->src->at(startidx + 1);

         if (c == '-')
         {
            c = ss->src->at(startidx + 2);

            if (c == '>')
            {
               ss->crt_idx += 2;
               tokenFound = true;
               //trc("flow %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

               mws_sp<kx_async_flowop> ke = kx_async_flowop::nwi();
               ke->fltype = kx_async_flowop::afl_right;

               return ke;
            }
         }
      }

      return mws_sp<kx_elem>();
   }
};


class kx_scn_flowop : public kx_scn
{
public:
   static mws_sp<kx_scn_flowop> nwi() { return mws_sp<kx_scn_flowop>(new kx_scn_flowop()); }
   virtual ~kx_scn_flowop() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (c == '<')
      {
         c = ss->src->at(startidx + 1);

         if (c == '-')
         {
            ss->crt_idx += 2;
            tokenFound = true;
            //trc("flow [%1%] ") % ss->src->substr(startidx, ss->crtidx - startidx);

            mws_sp<kx_flowop> ke = kx_flowop::nwi();
            ke->fltype = kx_flowop::fl_left;

            return ke;
         }
      }
      else if (c == '-')
      {
         c = ss->src->at(startidx + 1);

         if (c == '>')
         {
            ss->crt_idx += 2;
            tokenFound = true;
            //trc("flow %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

            mws_sp<kx_flowop> ke = kx_flowop::nwi();
            ke->fltype = kx_flowop::fl_right;

            return ke;
         }
      }

      return mws_sp<kx_elem>();
   }
};


class kx_scn_match_block : public kx_scn
{
public:
   static mws_sp<kx_scn_match_block> nwi() { return mws_sp<kx_scn_match_block>(new kx_scn_match_block()); }
   virtual ~kx_scn_match_block() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = 0;
      mws_sp<kx_scn> sc;

      c = ss->src->at(startidx);

      if (c != '?')
      {
         return mws_sp<kx_elem>();
      }

      mws_sp<kx_match_block> ke = kx_match_block::nwi();
      mws_sp<kx_elem> kxt;

      ss->crt_idx++;
      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      //if(kxt)
      //{
      //	ke->list.push_back(kxt);
      //}

      sc = kx_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->name = static_pointer_cast<kx_symbol>(kxt);
      }

      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      //if(kxt)
      //{
      //	ke->list.push_back(kxt);
      //}

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a '['", startidx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      ss->crt_idx++;

      sc = kx_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a ']'", startidx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke->blk = static_pointer_cast<kx_block>(kxt);
      }

      ss->crt_idx++;
      tokenFound = true;
      //trc("matchblk %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      return ke;
   }
};


class kx_scn_meta_block : public kx_scn
{
public:
   static mws_sp<kx_scn_meta_block> nwi() { return mws_sp<kx_scn_meta_block>(new kx_scn_meta_block()); }
   virtual ~kx_scn_meta_block() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = 0;
      mws_sp<kx_scn> sc;

      c = ss->src->at(startidx);

      if (c != '@')
      {
         return mws_sp<kx_elem>();
      }

      c = ss->src->at(startidx + 1);

      if (c == '@')
      {
         return mws_sp<kx_elem>();
      }

      mws_sp<kx_meta_block> ke = kx_meta_block::nwi();
      mws_sp<kx_elem> kxt;

      ss->crt_idx++;
      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      sc = kx_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->name = static_pointer_cast<kx_symbol>(kxt);
      }

      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      int idx = ss->crt_idx;
      sc = kx_scn_factory::nwi(kxs_comma, ss);
      kxt = sc->scan();

      if (sc->tokenFound)
      {
         ss->crt_idx = idx;
         tokenFound = true;
         //trc("metablk-short %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

         return ke;
      }

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", startidx, ss->crt_idx, c, '[');
         mws_throw mws_exception(msg);
      }

      ss->crt_idx++;

      sc = kx_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", startidx, ss->crt_idx, c, ']');
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke->blk = static_pointer_cast<kx_block>(kxt);
      }

      ss->crt_idx++;
      tokenFound = true;
      //trc("metablk %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      return ke;
   }
};


class kx_scn_ignore_block : public kx_scn
{
public:
   static mws_sp<kx_scn_ignore_block> nwi() { return mws_sp<kx_scn_ignore_block>(new kx_scn_ignore_block()); }
   virtual ~kx_scn_ignore_block() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = 0;
      mws_sp<kx_scn> sc;

      c = ss->src->at(startidx);

      if (c != '@')
      {
         return mws_sp<kx_elem>();
      }

      c = ss->src->at(startidx + 1);

      if (c != '@')
      {
         return mws_sp<kx_elem>();
      }

      mws_sp<kx_ignore_block> ke = kx_ignore_block::nwi();
      mws_sp<kx_elem> kxt;

      ss->crt_idx += 2;
      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      sc = kx_scn_factory::nwi(kxs_symbol, ss);
      kxt = sc->scan();

      if (kxt)
      {
         ke->name = static_pointer_cast<kx_symbol>(kxt);
      }

      sc = kx_scn_factory::nwi(kxs_whitespace, ss);
      kxt = sc->scan();

      int idx = ss->crt_idx;
      sc = kx_scn_factory::nwi(kxs_comma, ss);
      kxt = sc->scan();

      if (sc->tokenFound || ss->is_end_of_line())
      {
         ss->crt_idx = idx;
         tokenFound = true;
         //trc("ignblk-short %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

         return ke;
      }

      c = ss->src->at(ss->crt_idx);

      if (c != '[')
      {
         string msg = trs("ignoreblock parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", startidx, ss->crt_idx, c, '[');
         mws_throw mws_exception(msg);
      }

      sc = kx_scn_factory::nwi(kxs_ignore_block_body, ss);
      kxt = sc->scan();

      if (!sc->tokenFound)
      {
         string msg = trs("ignoreblock-body parse error");
         mws_throw mws_exception(msg);
      }

      mws_sp<kx_text> body = static_pointer_cast<kx_text>(kxt);
      ke->body = body->data;
      tokenFound = true;
      //trc("ignblk %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      return ke;
   }
};


class kx_scn_ignore_block_body : public kx_scn
{
public:
   static mws_sp<kx_scn_ignore_block_body> nwi() { return mws_sp<kx_scn_ignore_block_body>(new kx_scn_ignore_block_body()); }
   virtual ~kx_scn_ignore_block_body() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;

      if (ss->src->at(startidx) != '[')
      {
         return mws_sp<kx_elem>();
      }

      ss->crt_idx++;

      kx_scanner_type sct[] =
      {
         kxs_whitespace,
         kxs_ignore_block_body,
         //kxs_text,
      };

      while (ss->crt_idx < ss->src->length())
         // search for closing ']'
      {
         bool ttokenFound = false;

         do
         {
            ttokenFound = false;

            for (auto et : sct)
               // check every character for start / end of text / blocks pairs : {", ', [, ]},
               // to make sure they are matched correctly. ignore everything else
            {
               mws_sp<kx_scn> scn = kx_scn_factory::nwi(et, ss);
               scn->scan();

               if (scn->tokenFound)
               {
                  ttokenFound = true;
               }

               if (ss->crt_idx >= ss->src->length())
               {
                  if (ss->crt_idx > ss->src->length())
                  {
                     mws_throw mws_exception("kxscnignoreblock_body - passed the end of the string. this shouldn't happen...");
                  }

                  string msg = trs("ex 1 mismatched block started at {}", startidx + 1);
                  mws_throw mws_exception(msg);
               }
            }
         } while (ttokenFound);

         // no more recognized sequences. check for end of block, or skip character
         char c = ss->src->at(ss->crt_idx);

         if (c == ']')
            // found the end of the block
         {
            ss->crt_idx++;
            tokenFound = true;
            break;
         }
         else
            // not a whitespace, start/end of block/text, just ignore and move on
         {
            ss->crt_idx++;
         }
      }

      if (!tokenFound)
      {
         string msg = trs("ex 2 mismatched block started at {}", startidx + 1);
         mws_throw mws_exception(msg);
      }

      //trc("ignblk_body %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      mws_sp<kx_text> ke = kx_text::nwi();
      ke->data = ss->src->substr(startidx + 1, ss->crt_idx - startidx - 2);

      return ke;
   }
};


class kx_scn_block : public kx_scn
{
public:
   static mws_sp<kx_scn_block> nwi() { return mws_sp<kx_scn_block>(new kx_scn_block()); }
   virtual ~kx_scn_block() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      if (c != '[')
      {
         return mws_sp<kx_elem>();
      }

      mws_sp<kx_block> ke = kx_block::nwi();
      mws_sp<kx_elem> kxt;

      ss->crt_idx++;

      mws_sp<kx_scn> sc = kx_scn_factory::nwi(kxs_main, ss);
      kxt = sc->scan();

      c = ss->src->at(ss->crt_idx);

      if (c != ']')
      {
         string msg = trs("block parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", startidx, ss->crt_idx, c, ']');
         mws_throw mws_exception(msg);
      }

      if (kxt)
      {
         ke = static_pointer_cast<kx_block>(kxt);
      }

      ss->crt_idx++;
      tokenFound = true;
      //trc("blk %1% ") % ss->src->substr(startidx, ss->crtidx - startidx);

      return ke;
   }
};


class kx_scn_main : public kx_scn
{
public:
   static mws_sp<kx_scn_main> nwi() { return mws_sp<kx_scn_main>(new kx_scn_main()); }
   virtual ~kx_scn_main() {}

   virtual mws_sp<kx_elem> scan_impl()
   {
      int startidx = ss->crt_idx;
      char c = ss->src->at(startidx);

      kx_scanner_type sct[] =
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

      mws_sp<kx_block> ke = kx_block::nwi();
      mws_sp<kx_elem> kxt;

      while (!ss->is_end_of_line())
      {
         bool ttokenFound = false;

         for (auto et : sct)
         {
            mws_sp<kx_scn> scn = kx_scn_factory::nwi(et, ss);
            kxt = scn->scan();

            if (scn->tokenFound)
            {
               ttokenFound = true;

               if (kxt->type == kxe_block && ke->list.size() >= 1)
                  // find out if this block has a name
               {
                  mws_sp<kx_block> kb = static_pointer_cast<kx_block>(kxt);

                  if (ke->list.back()->type == kxe_symbol)
                  {
                     kb->name = static_pointer_cast<kx_symbol>(ke->list.back());
                     ke->list.pop_back();
                  }
                  else if (ke->list.size() >= 2 && ke->list.back()->type == kxe_whitespace && ke->list[ke->list.size() - 2]->type == kxe_symbol)
                  {
                     ke->list.pop_back();
                     kb->name = static_pointer_cast<kx_symbol>(ke->list.back());
                     ke->list.pop_back();
                  }
               }

               if (kxt->type != kxe_whitespace)// && kxt->type != kxe_comma)
                  // discard separators: whitespace and comma
               {
                  ke->list.push_back(kxt);

                  if (ke->list.size() >= 3)
                     // connect
                  {
                     int idx = ke->list.size() - 1;

                     if (ke->list[idx]->is_process() && ke->list[idx - 1]->type == kxe_flowop && ke->list[idx - 2]->is_process())
                     {
                        //trx("%1%%2%%3%") % ke->list[idx - 2]->print() % ke->list[idx - 1]->print() % ke->list[idx]->print();

                        mws_sp<kx_process> kxleft = static_pointer_cast<kx_process>(ke->list[idx - 2]);
                        mws_sp<kx_flowop> kfl = static_pointer_cast<kx_flowop>(ke->list[idx - 1]);
                        mws_sp<kx_process> kxright = static_pointer_cast<kx_process>(ke->list[idx]);

                        switch (kfl->fltype)
                        {
                        case kx_flowop::fl_left:
                           kxleft->in = kfl;
                           kfl->dst = kxleft;
                           kxright->ex = kfl;
                           kfl->src = kxright;
                           break;

                        case kx_flowop::fl_right:
                           kxleft->ex = kfl;
                           kfl->src = kxleft;
                           kxright->in = kfl;
                           kfl->dst = kxright;
                           break;
                        }
                     }
                  }
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

         if (!ttokenFound)
         {
            break;
         }
      }

      int idx = ss->crt_idx;
      bool parseError = false;

      if (idx >= ss->src->length())
      {
         idx = ss->src->length() - 1;
      }

      c = ss->src->at(idx);

      if (startidx == 0)
         // scanner for the whole program/file
      {
         if (ss->crt_idx != ss->src->length())
         {
            parseError = true;
         }
      }
      else
         // scanner for a block
      {
         if (c != ']')
         {
            parseError = true;
         }
      }

      if (parseError)
      {
         string msg = trs("main line scan error {0}-{1}. unknown token '{2}'", startidx, ss->crt_idx, c);
         mws_throw mws_exception(msg);
      }

      tokenFound = true;

      return ke;
   }
};


mws_sp<kx_scn> kx_scn_factory::nwi(kx_scanner_type type, mws_sp<kx_shared_state> ss)
{
   mws_sp<kx_scn> inst;

   switch (type)
   {
   case kxs_main:
      inst = kx_scn_main::nwi();
      break;

   case kxs_block:
      inst = kx_scn_block::nwi();
      break;

   case kxs_symbol:
      inst = kx_scn_symbol::nwi();
      break;

   case kxs_text:
      inst = kx_scn_text::nwi();
      break;

   case kxs_async_flowop:
      inst = kx_scn_async_flowop::nwi();
      break;

   case kxs_flowop:
      inst = kx_scn_flowop::nwi();
      break;

   case kxs_ignore_block:
      inst = kx_scn_ignore_block::nwi();
      break;

   case kxs_ignore_block_body:
      inst = kx_scn_ignore_block_body::nwi();
      break;

   case kxs_match_block:
      inst = kx_scn_match_block::nwi();
      break;

   case kxs_meta_block:
      inst = kx_scn_meta_block::nwi();
      break;

   case kxs_whitespace:
      inst = kx_scn_whitespace::nwi();
      break;

   case kxs_comma:
      inst = kx_scn_comma::nwi();
      break;
   }

   if (inst)
   {
      inst->setState(ss);
   }

   return inst;
}


mws_sp<kx_elem> kx_parser::parse_src(mws_sp<std::string> src)
{
   mws_sp<kx_scn> sc;
   mws_sp<kx_shared_state> ss;

   ss = kx_shared_state::nwi();
   ss->src = src;
   sc = kx_scn_factory::nwi(kxs_main, ss);

   return sc->scan();
}

#endif
