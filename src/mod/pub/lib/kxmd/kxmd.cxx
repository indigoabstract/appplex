#include "stdafx.hxx"

#include "kxmd.hxx"
#include "min.hxx"
#include "pfm.hxx"
#include <cctype>


namespace ns_kxmd
{
   struct kxmd_kv
   {
      std::string key;
      std::vector<mws_sp<kxmd_kv>> val;
   };


   mws_sp<kxmd_kv> parse_kxmd(mws_sp<std::string> i_src);

   std::string indent_by_level(int i_level)
   {
      static const std::string indent_str = "    ";
      std::string ret;

      for (int k = 0; k < i_level; k++)
      {
         ret += indent_str;
      }

      return ret;
   }


   std::string strip_quotes(const std::string& i_text)
   {
      const char single_quote = '\'';
      const char double_quote = '"';

      if (i_text.length() >= 2)
      {
         if ((i_text.front() == single_quote && i_text.back() == single_quote) || (i_text.front() == double_quote && i_text.back() == double_quote))
         {
            std::string result = i_text.substr(1, i_text.length() - 2);

            return result;
         }
      }

      return i_text;
   }


   class kxmd_impl
   {
   public:
      static bool check_valid(const kxmd_ref* i_ref, mws_sp<kxmd>& i_db, mws_sp<kxmd_kv>& i_kv)
      {
         i_db = i_ref->db.lock();

         if (i_db)
         {
            i_kv = i_ref->kv.lock();

            return i_kv != nullptr;
         }

         return false;
      }

      static bool check_valid(const kxmd_ref* i_ref, mws_sp<kxmd_kv>& i_kv)
      {
         mws_sp<kxmd> db;

         return check_valid(i_ref, db, i_kv);
      }

      static bool check_valid(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         return check_valid(i_ref, db, kv);
      }

      kxmd_ref get_main_ref() const
      {
         return kxmd_ref(db_ref.lock(), main);
      }

      static mws_sp<kxmd_kv> nwi() { return mws_sp<kxmd_kv>(new kxmd_kv()); }

      static std::string key(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            return kv->key.c_str();
         }

         return "";
      }

      static std::vector<kxmd_ref> val(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, db, kv))
         {
            std::vector<kxmd_ref> val_vect(kv->val.size());

            for (mws_sp<kxmd_kv>& kv : kv->val)
            {
               val_vect.push_back(kxmd_ref(db, kv));
            }

            return val_vect;
         }

         return std::vector<kxmd_ref>();
      }

      static size_t size(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            return kv->val.size();
         }

         return 0;
      }

      static bool is_leaf(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            return is_leaf_impl(kv);
         }

         return false;
      };

      static bool is_node(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            return is_node_impl(kv);
         }

         return false;
      };

      static void del_val(const kxmd_ref* i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            kv->val.clear();
         }
      }

      static void del_val_at_idx(const kxmd_ref* i_ref, uint32 i_idx)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            mws_sp<kxmd_kv> res = kv->val[i_idx];

            kv->val.erase(kv->val.begin() + i_idx);
         }
      }

      static kxmd_ref elem_at_idx(const kxmd_ref * i_ref, uint32 i_idx)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, db, kv))
         {
            mws_sp<kxmd_kv> res = kv->val[i_idx];
            return kxmd_ref(db, res);
         }

         return kxmd_ref();
      }

      static kxmd_ref elem_at_path(const kxmd_ref * i_ref, const std::string & i_path)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, db, kv))
         {
            mws_sp<kxmd_kv> res = elem_at_path_impl(kv, i_path);

            if (res)
            {
               return kxmd_ref(db, res);
            }
         }

         return kxmd_ref();
      }

      static std::string val_at(const kxmd_ref * i_ref, const std::string & i_path, const std::string & i_default_val)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            std::vector<std::string> tokens = str_split(i_path, ".");

            for (auto& ke_name : tokens)
            {
               kv = find_by_key_impl(kv, ke_name, false);

               if (!kv)
               {
                  return i_default_val;
               }
            }

            if (!kv->val.empty())
            {
               return strip_quotes(kv->val[0]->key);
            }

            return i_default_val;
         }

         return "";
      }

      static std::vector<std::string> val_seq_at(const kxmd_ref * i_ref, const std::string & i_path, const std::vector<std::string> & i_default_val)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            std::vector<std::string> val_vect;
            std::vector<std::string> tokens = str_split(i_path, ".");

            for (auto& ke_name : tokens)
            {
               kv = find_by_key_impl(kv, ke_name, false);

               if (!kv)
               {
                  return i_default_val;
               }
            }

            if (!kv->val.empty())
            {
               for (auto& kv2 : kv->val)
               {
                  val_vect.push_back(strip_quotes(kv2->key));
               }

               return val_vect;
            }

            return i_default_val;
         }

         return std::vector<std::string>();
      }

      static bool path_exists(const kxmd_ref * i_ref, const std::string & i_path)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            std::vector<std::string> tokens = str_split(i_path, ".");

            for (auto& ke_name : tokens)
            {
               kv = find_by_key_impl(kv, ke_name, false);

               if (!kv)
               {
                  return false;
               }
            }

            return true;
         }

         return false;
      }

      static void set_key(const kxmd_ref * i_ref, const std::string & i_key)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            kv->key = i_key;
         }
      }

      static kxmd_ref push_back(const kxmd_ref * i_ref, const std::string & i_key)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, db, kv))
         {
            mws_sp<kxmd_kv> val = nwi();

            val->key = i_key;
            kv->val.push_back(val);

            return kxmd_ref(db, val);
         }

         return kxmd_ref();
      }

      static void push_back(const kxmd_ref * i_ref, const std::vector<std::string> & i_list)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            for (auto& str : i_list)
            {
               mws_sp<kxmd_kv> val = nwi();

               val->key = str;
               kv->val.push_back(val);
            }
         }
      }

      static kxmd_ref find_by_key(const kxmd_ref * i_ref, const std::string & i_name, bool i_recursive)
      {
         mws_sp<kxmd> db;
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, db, kv))
         {
            mws_sp<kxmd_kv> res = find_by_key_impl(kv, i_name, i_recursive);

            if (res)
            {
               return kxmd_ref(db, res);
            }
         }

         return kxmd_ref();
      }

      static std::string to_str_inc_self(const kxmd_ref * i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            return to_string_impl(kv, 0);
         }

         return "";
      }

      static std::string to_str(const kxmd_ref * i_ref)
      {
         mws_sp<kxmd_kv> kv;

         if (check_valid(i_ref, kv))
         {
            std::string s;
            int size = kv->val.size();

            for (int k = 0; k < size; k++)
            {
               auto& ke = kv->val[k];
               s += to_string_impl(ke, 0);

               if (k < (size - 1))
               {
                  s += ", ";

                  if (is_node_impl(ke))
                  {
                     s += "\n";
                  }
               }
            }

            return s;
         }
         return "";
      }

      // implementation

      static bool is_leaf_impl(const mws_sp<kxmd_kv> & i_kv) { return i_kv->val.empty() && !i_kv->key.empty(); }

      static bool is_node_impl(const mws_sp<kxmd_kv> & i_kv) { return !i_kv->val.empty(); }

      static mws_sp<kxmd_kv> elem_at_path_impl(mws_sp<kxmd_kv> i_kv, const std::string & i_path)
      {
         std::vector<std::string> tokens = str_split(i_path, ".");

         for (auto& ke_name : tokens)
         {
            i_kv = find_by_key_impl(i_kv, ke_name, false);

            if (!i_kv)
            {
               return nullptr;
            }
         }

         return i_kv;
      }

      static mws_sp<kxmd_kv> find_by_key_impl(const mws_sp<kxmd_kv> i_kv, const std::string & i_name, bool i_recursive)
      {
         for (auto& kv : i_kv->val)
         {
            if (kv)
            {
               if (kv->key == i_name)
               {
                  return kv;
               }

               if (i_recursive)
               {
                  mws_sp<kxmd_kv> kvt = find_by_key_impl(kv, i_name, true);

                  if (kvt)
                  {
                     return kvt;
                  }
               }
            }
         }

         return nullptr;
      }

      static std::string to_string_impl(const mws_sp<kxmd_kv> i_kv, int i_level)
      {
         std::string s;

         if (!i_kv->key.empty())
         {
            s += i_kv->key;
         }

         if (!i_kv->val.empty())
         {
            int size = i_kv->val.size();

            if (!i_kv->key.empty())
            {
               s += "\n";
               s += indent_by_level(i_level);
            }

            s += "[\n";
            s += indent_by_level(i_level + 1);

            for (int k = 0; k < size; k++)
            {
               auto& ke = i_kv->val[k];
               s += to_string_impl(ke, i_level + 1);

               if (k < (size - 1))
               {
                  s += ", ";

                  if (is_node_impl(ke))
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

      std::string db_path;
      mws_wp<kxmd> db_ref;
      mws_sp<kxmd_kv> main;
   };


   kxmd_ref::kxmd_ref() {}

   bool kxmd_ref::valid() const
   {
      return kxmd_impl::check_valid(this);
   }

   mws_sp<kxmd> kxmd_ref::get_db() const
   {
      return db.lock();
   }

   std::string kxmd_ref::key() const
   {
      return kxmd_impl::key(this);
   }

   std::vector<kxmd_ref> kxmd_ref::val() const
   {
      return kxmd_impl::val(this);
   }

   size_t kxmd_ref::size() const
   {
      return kxmd_impl::size(this);
   }

   bool kxmd_ref::is_leaf() const
   {
      return kxmd_impl::is_leaf(this);
   }

   bool kxmd_ref::is_node() const
   {
      return kxmd_impl::is_node(this);
   }

   void kxmd_ref::del_val() const
   {
      kxmd_impl::del_val(this);
   }

   void kxmd_ref::del_val_at_idx(uint32 i_idx) const
   {
      kxmd_impl::del_val_at_idx(this, i_idx);
   }

   bool kxmd_ref::operator==(const kxmd_ref & i_ref) const
   {
      return i_ref.db.lock() == db.lock() && i_ref.kv.lock() == kv.lock();
   }

   kxmd_ref kxmd_ref::operator[](uint32 i_idx) const
   {
      return kxmd_impl::elem_at_idx(this, i_idx);
   }

   kxmd_ref kxmd_ref::operator[](const std::string & i_path) const
   {
      return kxmd_impl::elem_at_path(this, i_path);;
   }

   std::string kxmd_ref::val_at(const std::string & i_path, const std::string & i_default_val) const
   {
      return kxmd_impl::val_at(this, i_path, i_default_val);
   }

   std::vector<std::string> kxmd_ref::val_seq_at(const std::string & i_path, const std::vector<std::string> & i_default_val) const
   {
      return kxmd_impl::val_seq_at(this, i_path, i_default_val);
   }

   bool kxmd_ref::path_exists(const std::string & i_path) const
   {
      return kxmd_impl::path_exists(this, i_path);
   }

   void kxmd_ref::set_key(const std::string & i_key) const
   {
      kxmd_impl::set_key(this, i_key);
   }

   kxmd_ref kxmd_ref::push_back(const std::string & i_key) const
   {
      return kxmd_impl::push_back(this, i_key);
   }

   void kxmd_ref::push_back(const std::vector<std::string> & i_list) const
   {
      return kxmd_impl::push_back(this, i_list);
   }

   kxmd_ref kxmd_ref::find_by_key(const std::string & i_key, bool i_recursive) const
   {
      return kxmd_impl::find_by_key(this, i_key, i_recursive);
   }

   std::string kxmd_ref::to_str_inc_self() const
   {
      return kxmd_impl::to_str_inc_self(this);
   }

   std::string kxmd_ref::to_str() const
   {
      return kxmd_impl::to_str(this);
   }

   kxmd_ref::kxmd_ref(mws_sp<kxmd> i_db, mws_sp<kxmd_kv> i_kv) : db(i_db), kv(i_kv) {};


   mws_sp<kxmd> kxmd::nwi(const char* i_kxmd_data, uint32 i_size)
   {
      mws_sp<std::string> str(new std::string(i_kxmd_data, i_size));
      return nwi(*str);
   }

   mws_sp<kxmd> kxmd::nwi(const std::string & i_kxmd_data)
   {
      mws_sp<kxmd> db(new kxmd());
      mws_sp<kxmd_kv> main;;

      if (i_kxmd_data.empty())
      {
         main = mws_sp<kxmd_kv>(new kxmd_kv());
      }
      else
      {
         main = parse_kxmd(mws_sp<std::string>(new std::string(i_kxmd_data)));
      }

      db->p->db_ref = db;
      db->p->main = main;

      return db;
   }

   mws_sp<kxmd> kxmd::nwi_from_file(const std::string & i_filename)
   {
      auto str = pfm::filesystem::load_res_as_string(i_filename);
      mws_sp<kxmd> db;

      if (str)
      {
         db = mws_sp<kxmd>(new kxmd());
         mws_sp<kxmd_kv> main = parse_kxmd(str);
         db->p->db_ref = db;
         db->p->main = main;
      }

      return db;
   }

   kxmd::kxmd()
   {
      p = new kxmd_impl();
   }

   kxmd::~kxmd()
   {
      if (p)
      {
         delete p;
         p = nullptr;
      }
   }

   kxmd_ref kxmd::main() const
   {
      return p->get_main_ref();
   }


   // begin the parser stuff

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

   class kxs_elem : public kxmd_kv
   {
   public:
      virtual ~kxs_elem() {}
      size_t size() const { return val.size(); }
      virtual bool is_leaf() const { return val.empty() && !key.empty(); };
      virtual bool is_node() const { return !val.empty(); };
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
      virtual std::string get_name()const { return key; }

   private:
      kxmd_symbol() {}
   };


   class kxmd_text : public kxmd_process
   {
   public:
      static mws_sp<kxmd_text> nwi() { return mws_sp<kxmd_text>(new kxmd_text()); }
      virtual kxmd_elem_type get_type() const override { return kxmd_elem_type::kxe_text; }
      virtual std::string get_name()const { return key; }

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
         if (key.empty()) { return "block-nn/a"; }
         return key;
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
                  ke->key = ss->src->substr(start_idx, ss->crt_idx - start_idx);

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
         ke->key = ss->src->substr(start_idx, ss->crt_idx - start_idx);

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
               ke->key = ss->src->substr(start_idx, ss->crt_idx - start_idx);

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
            ke->key = kxt->key;
         }

         sc = kxmd_scn_factory::nwi(kxs_whitespace, ss);
         kxt = sc->scan();

         c = ss->src->at(ss->crt_idx);

         if (c != '[')
         {
            std::string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a '['", start_idx, ss->crt_idx, c);
            mws_throw mws_exception(msg);
         }

         ss->crt_idx++;

         sc = kxmd_scn_factory::nwi(kxs_main, ss);
         kxt = sc->scan();

         c = ss->src->at(ss->crt_idx);

         if (c != ']')
         {
            std::string msg = trs("matchblk parse error {0}-{1}. unknown token '{2}'. expected a ']'", start_idx, ss->crt_idx, c);
            mws_throw mws_exception(msg);
         }

         if (kxt)
         {
            ke->val.push_back(kxt);
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
            ke->key = kxt->key;
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
            std::string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, '[');
            mws_throw mws_exception(msg);
         }

         ss->crt_idx++;

         sc = kxmd_scn_factory::nwi(kxs_main, ss);
         kxt = sc->scan();

         c = ss->src->at(ss->crt_idx);

         if (c != ']')
         {
            std::string msg = trs("metablk parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, ']');
            mws_throw mws_exception(msg);
         }

         if (kxt)
         {
            ke->val.push_back(kxt);
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
            ke->key = kxt->key;
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
            std::string msg = trs("ignoreblock parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, '[');
            mws_throw mws_exception(msg);
         }

         sc = kxmd_scn_factory::nwi(kxs_ignore_block_body, ss);
         kxt = sc->scan();

         if (!sc->token_found)
         {
            std::string msg = trs("ignoreblock-body parse error");
            mws_throw mws_exception(msg);
         }

         mws_sp<kxmd_text> body = static_pointer_cast<kxmd_text>(kxt);
         mws_sp<kxmd_kv> text = mws_sp<kxmd_kv>(new kxmd_kv());
         ke->val.push_back(text);
         text->key = body->key;
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

                     std::string msg = trs("ex 1 mismatched block started at {}", start_idx + 1);
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
            std::string msg = trs("ex 2 mismatched block started at {}", start_idx + 1);
            mws_throw mws_exception(msg);
         }

         mws_sp<kxmd_text> ke = kxmd_text::nwi();
         ke->key = ss->src->substr(start_idx + 1, ss->crt_idx - start_idx - 2);

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
            std::string msg = trs("block parse error {0}-{1}. unknown token '{2}'. expected a '{3}'", start_idx, ss->crt_idx, c, ']');
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

         while (!ss->is_end_of_line())
         {
            bool ttoken_found = false;

            for (auto et : sct)
            {
               mws_sp<kxmd_scn> scn = kxmd_scn_factory::nwi(et, ss);
               mws_sp<kxs_elem> kxt = scn->scan();

               if (kxt && kxt->key == "vorbis")
               {
                  int x = 3;
               }

               if (scn->token_found)
               {
                  ttoken_found = true;

                  // discard comments
                  if (kxt->get_type() != kxmd_elem_type::kxe_ignore_block)
                  {
                     // find out if this block has a name
                     if (kxt->is_node() && ke->val.size() >= 1)
                     {
                        mws_sp<kxmd_block> kb = static_pointer_cast<kxmd_block>(kxt);

                        if (kxmd_impl::is_leaf_impl(ke->val.back()))
                        {
                           kb->key = ke->val.back()->key;
                           ke->val.pop_back();
                        }
                     }

                     // discard everything except nodes and leaves
                     if (kxt->is_leaf() || kxt->is_node())
                     {
                        ke->val.push_back(kxt);
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
            std::string msg = trs("main line scan error {0}-{1}. unknown token '{2}'", start_idx, ss->crt_idx, c);
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


   mws_sp<kxmd_kv> parse_kxmd(mws_sp<std::string> src)
   {
      mws_sp<kxmd_scn> sc;
      mws_sp<kxmd_shared_state> ss;

      ss = kxmd_shared_state::nwi();
      ss->src = src;
      sc = kxmd_scn_factory::nwi(kxs_main, ss);

      return sc->scan();
   }
}