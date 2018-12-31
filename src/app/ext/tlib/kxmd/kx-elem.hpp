#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "pfm.hpp"
#include <string>
#include <vector>


class rectangle_2d;
class kx_process;


enum kx_elem_type
{
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

const std::string indent_str = "    ";

class kx_elem : public std::enable_shared_from_this<kx_elem>
{
public:
   virtual ~kx_elem() {}

   mws_sp<kx_elem> get_inst() { return shared_from_this(); }
   virtual bool is_block() const { return false; }
   virtual bool is_process() const { return false; }
   virtual std::string print(int ilevel = 0) { return "kx_elem"; }
   virtual void eval() {}

   kx_elem_type type;

protected:
   kx_elem() {}
   std::string indent_by_level(int ilevel)
   {
      std::string ret;

      for (int k = 0; k < ilevel; k++)
      {
         ret += indent_str;
      }

      return ret;
   }
};


class kx_whitespace : public kx_elem
{
public:
   static mws_sp<kx_whitespace> nwi();
   virtual ~kx_whitespace() {}

   virtual std::string print(int ilevel = 0);

   std::string data;

private:
   kx_whitespace() { type = kxe_whitespace; }
};


class kx_comma : public kx_elem
{
public:
   static mws_sp<kx_comma> nwi();
   virtual ~kx_comma() {}

   virtual std::string print(int ilevel = 0);

private:
   kx_comma() { type = kxe_comma; }
};


class kx_async_flowop : public kx_elem
{
public:
   enum aflow_types
   {
      afl_left,
      afl_right,
   };

   static mws_sp<kx_async_flowop> nwi();
   virtual ~kx_async_flowop() {}

   virtual std::string print(int ilevel = 0);

   int fltype;
   std::vector<mws_sp<kx_process> > cnx;
   int capacity;

private:
   kx_async_flowop()
   {
      type = kxe_async_flowop;
      fltype = afl_left;
      capacity = 1;
   }
};

class kx_flowop : public kx_elem
{
public:
   enum flow_types
   {
      fl_left,
      fl_right,
   };

   static mws_sp<kx_flowop> nwi();
   virtual ~kx_flowop() {}

   virtual std::string print(int ilevel = 0);

   int fltype;
   weak_ptr<kx_process> src, dst;
   std::vector<mws_sp<kx_process> > cnx;
   int capacity;

private:
   kx_flowop()
   {
      type = kxe_flowop;
      fltype = fl_left;
      capacity = 1;
   }
};


class kx_process : public kx_elem
{
public:
   virtual ~kx_process() {}

   virtual bool is_process() const override;
   virtual std::string print(int ilevel = 0);
   virtual std::string get_name()const = 0;
   // direct subelements(subblocks)
   virtual int get_elem_list_size() const { return 0; }
   virtual mws_sp<kx_elem> get_elem_at(int i_idx) const { return nullptr; }
   // search inside the component for a process with the given name
   // if recursive, searches all subcomponents too
   virtual mws_sp<kx_process> find_by_name(const std::string& iname, bool i_recursive) const { return nullptr; }
   // get index of a subelement in the list by name. returns index or -1 if not found
   virtual int index_of_name(const std::string& iname) const { return -1; }

   mws_sp<kx_flowop> in, ex;
   mws_sp<rectangle_2d> box;
   bool is_arranged;

protected:
   kx_process() {}
};


class kx_symbol : public kx_process
{
public:
   static mws_sp<kx_symbol> nwi();
   virtual ~kx_symbol() {}

   virtual std::string print(int ilevel = 0);
   virtual void eval();
   virtual std::string get_name()const { return name; }

   std::string name;

private:
   kx_symbol() { type = kxe_symbol; }
};


class kx_text : public kx_process
{
public:
   static mws_sp<kx_text> nwi();
   virtual ~kx_text() {}

   virtual std::string print(int ilevel = 0);
   virtual void eval();
   virtual std::string get_name()const { return data; }

   std::string data;

private:
   kx_text() { type = kxe_text; }
};


class kx_block : public kx_process
{
public:
   static mws_sp<kx_block> nwi();
   virtual ~kx_block() {}

   virtual bool is_block() const override { return true; }
   virtual std::string print(int ilevel = 0);
   virtual void eval();
   virtual std::string get_name()const
   {
      if (!name) { return "block-nn/a"; }
      return name->name;
   }
   virtual int get_elem_list_size() const override { return list.size(); }
   virtual mws_sp<kx_elem> get_elem_at(int i_idx) const override { return list[i_idx]; }
   virtual mws_sp<kx_process> find_by_name(const std::string& iname, bool i_recursive) const override;
   virtual int index_of_name(const std::string& iname) const override;

   mws_sp<kx_symbol> name;
   std::vector<mws_sp<kx_elem> > list;

protected:
   kx_block() { type = kxe_block; }
};


class kx_ignore_block : public kx_process
{
public:
   static mws_sp<kx_ignore_block> nwi();
   virtual ~kx_ignore_block() {}

   virtual bool is_block() const override { return true; }
   virtual std::string print(int ilevel = 0);
   virtual std::string get_name()const { return "comment"; }

   mws_sp<kx_symbol> name;
   std::string body;

private:
   kx_ignore_block() { type = kxe_ignore_block; }
};


class kx_match_block : public kx_process
{
public:
   static mws_sp<kx_match_block> nwi();
   virtual ~kx_match_block() {}

   virtual bool is_block() const override { return true; }
   virtual std::string print(int ilevel = 0);
   virtual std::string get_name()const { return "match-block"; }

   mws_sp<kx_symbol> name;
   mws_sp<kx_block> blk;

private:
   kx_match_block() { type = kxe_match_block; }
};


class kx_meta_block : public kx_process
{
public:
   static mws_sp<kx_meta_block> nwi();
   virtual ~kx_meta_block() {}

   virtual bool is_block() const override { return true; }
   virtual std::string print(int ilevel = 0);
   virtual std::string get_name()const { return "meta-block"; }

   mws_sp<kx_symbol> name;
   mws_sp<kx_block> blk;

private:
   kx_meta_block() { type = kxe_meta_block; }
};

#endif
