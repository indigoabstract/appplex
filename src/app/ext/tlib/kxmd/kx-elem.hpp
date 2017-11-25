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


class kx_elem : public std::enable_shared_from_this<kx_elem>
{
public:
	virtual ~kx_elem(){}

	shared_ptr<kx_elem> get_inst(){ return shared_from_this(); }
	virtual bool is_process(){ return false; }
	virtual std::string print(int ilevel = 0){ return "kx_elem"; }
	virtual void eval(){}
	virtual shared_ptr<kx_process> find_by_name(const std::string& iname){ return nullptr; }

	kx_elem_type type;

protected:
	kx_elem(){}
	std::string indent_by_level(int ilevel)
	{
		std::string ret;

		for (int k = 0; k < ilevel; k++)
		{
			ret += '\t';
		}

		return ret;
	}
};


class kx_whitespace : public kx_elem
{
public:
	static shared_ptr<kx_whitespace> new_instance();
	virtual ~kx_whitespace(){}

	virtual std::string print(int ilevel = 0);

	std::string data;

private:
	kx_whitespace(){ type = kxe_whitespace; }
};


class kx_comma : public kx_elem
{
public:
	static shared_ptr<kx_comma> new_instance();
	virtual ~kx_comma(){}

	virtual std::string print(int ilevel = 0);

private:
	kx_comma(){ type = kxe_comma; }
};


class kx_async_flowop : public kx_elem
{
public:
	enum aflow_types
	{
		afl_left,
		afl_right,
	};

	static shared_ptr<kx_async_flowop> new_instance();
	virtual ~kx_async_flowop(){}

	virtual std::string print(int ilevel = 0);

	int fltype;
	std::vector<shared_ptr<kx_process> > cnx;
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

	static shared_ptr<kx_flowop> new_instance();
	virtual ~kx_flowop(){}

	virtual std::string print(int ilevel = 0);

	int fltype;
	weak_ptr<kx_process> src, dst;
	std::vector<shared_ptr<kx_process> > cnx;
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
	virtual ~kx_process(){}

	virtual bool is_process();
	virtual std::string print(int ilevel = 0);
	virtual std::string get_name()const = 0;
	virtual shared_ptr<kx_process> find_by_name(const std::string& iname);

	shared_ptr<kx_flowop> in, ex;
	shared_ptr<rectangle_2d> box;
	bool is_arranged;

protected:
	kx_process(){}
};


class kx_symbol : public kx_process
{
public:
	static shared_ptr<kx_symbol> new_instance();
	virtual ~kx_symbol(){}

	virtual std::string print(int ilevel = 0);
	virtual void eval();
	virtual std::string get_name()const{ return name; }

	std::string name;

private:
	kx_symbol(){ type = kxe_symbol; }
};


class kx_text : public kx_process
{
public:
	static shared_ptr<kx_text> new_instance();
	virtual ~kx_text(){}

	virtual std::string print(int ilevel = 0);
	virtual void eval();
	virtual std::string get_name()const{ return data; }

	std::string data;

private:
	kx_text(){ type = kxe_text; }
};


class kx_block : public kx_process
{
public:
	static shared_ptr<kx_block> new_instance();
	virtual ~kx_block(){}

	virtual std::string print(int ilevel = 0);
	virtual void eval();
	virtual std::string get_name()const
	{
		if (!name){ return "block-nn/a"; }
		return name->name;
	}
	virtual shared_ptr<kx_process> find_by_name(const std::string& iname);

	shared_ptr<kx_symbol> name;
	std::vector<shared_ptr<kx_elem> > list;

protected:
	kx_block(){ type = kxe_block; }
};


class kx_ignore_block : public kx_process
{
public:
	static shared_ptr<kx_ignore_block> new_instance();
	virtual ~kx_ignore_block(){}

	virtual std::string print(int ilevel = 0);
	virtual std::string get_name()const{ return "comment"; }

	shared_ptr<kx_symbol> name;
	std::string body;

private:
	kx_ignore_block(){ type = kxe_ignore_block; }
};


class kx_match_block : public kx_process
{
public:
	static shared_ptr<kx_match_block> new_instance();
	virtual ~kx_match_block(){}

	virtual std::string print(int ilevel = 0);
	virtual std::string get_name()const{ return "match-block"; }

	shared_ptr<kx_symbol> name;
	shared_ptr<kx_block> blk;

private:
	kx_match_block(){ type = kxe_match_block; }
};


class kx_meta_block : public kx_process
{
public:
	static shared_ptr<kx_meta_block> new_instance();
	virtual ~kx_meta_block(){}

	virtual std::string print(int ilevel = 0);
	virtual std::string get_name()const{ return "meta-block"; }

	shared_ptr<kx_symbol> name;
	shared_ptr<kx_block> blk;

private:
	kx_meta_block(){ type = kxe_meta_block; }
};

#endif
