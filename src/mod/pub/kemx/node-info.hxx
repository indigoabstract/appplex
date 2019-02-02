#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_KEMX

#include "mws-mod.hxx"
#include <exception>
#include <string>
#include <vector>

class IllegalStateException : public std::exception
{
public:
	IllegalStateException(std::string& s)
	{
		msg = s;
	}

	~IllegalStateException()
	{
	}

	const char* what() const noexcept
	{
		return msg.c_str();
	}

	std::string msg;
};


class node_info
{
public:
	node_info();
	~node_info();
	const std::string& getName()const{ return name; }
	//const std::string& getVal()const{return val;}
	const std::vector<mws_sp<node_info> > getList()const{ return list; }
	static int parseBlock(std::string& exp, std::vector<mws_sp<node_info> >& list, int level);
	static void printList(std::vector<mws_sp<node_info> >& list, int ilevel);

private:
	std::string name;
	//std::string val;
	std::vector<mws_sp<node_info> > list;
};

#endif
