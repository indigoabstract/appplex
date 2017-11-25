#include "stdafx.h"

#include "min.hpp"
#include "unit-ctrl.hpp"
#include <regex>
#include <sstream>


using std::string;
using std::wstring;


ia_exception::ia_exception() throw()
{
}

ia_exception::ia_exception(const char* msg) throw()
{
	exmsg = msg;
}

ia_exception::ia_exception(std::string msg) throw()
{
	exmsg = msg;
}

ia_exception::~ia_exception() throw()
{
}

const char* ia_exception::what() const throw()
{
	return exmsg.c_str();
}


iadp::iadp(const std::string& iname)
{
	set_name(iname);
	processed = false;
}

shared_ptr<iadp> iadp::new_instance(std::string iname)
{
	return shared_ptr<iadp>(new iadp(iname));
}

const std::string& iadp::get_name()
{
	return name;
}

bool iadp::is_type(const std::string& itype)
{
	return starts_with(get_name(), itype);
}

bool iadp::is_processed()
{
	return processed;
}

void iadp::process()
{
	if(processed)
	{
		throw ia_exception("datapacket is already processed");
	}

	processed = true;
}

shared_ptr<ia_sender> iadp::source()
{
	return src.lock();
}

shared_ptr<ia_receiver> iadp::destination()
{
	return dst.lock();
}

void iadp::set_name(const std::string& iname)
{
	name = iname;
}


void ia_sender::send(shared_ptr<ia_receiver> dst, shared_ptr<iadp> idp)
{
	idp->src = sender_inst();
	idp->dst = dst;
	dst->receive(idp);
}


void ia_broadcaster::add_receiver(shared_ptr<ia_receiver> ir)
{
	bool exists = false;
	int size = receivers.size();

	for(int k = 0; k < size; k++)
	{
		shared_ptr<ia_receiver> sr = receivers[k].lock();

		if(sr == ir)
		{
			exists = true;
			break;
		}
	}

	if(!exists)
	{
		receivers.push_back(ir);
	}
}

void ia_broadcaster::remove_receiver(shared_ptr<ia_receiver> ir)
{
	int idx = -1;
	int k = 0;
	int size = receivers.size();

	for(int k = 0; k < size; k++)
	{
		shared_ptr<ia_receiver> sr = receivers[k].lock();

		if(sr == ir)
		{
			idx = k;
			break;
		}
	}

	if(idx >= 0)
	{
		receivers.erase(receivers.begin() + idx);
	}
}

void ia_broadcaster::broadcast(shared_ptr<ia_sender> src, shared_ptr<iadp> idp)
{
	int size = receivers.size();

	for(int k = 0; k < size; k++)
	{
		shared_ptr<ia_receiver> dst = receivers[k].lock();

		if(dst)
		{
			send(dst, idp);
		}
	}
}

bool starts_with(const std::string& istr, const std::string& ifind)
{
	int size = istr.length();
	int size_find = ifind.length();

	if (size_find > size)
	{
		return false;
	}

	for (int k = 0; k < size_find; k++)
	{
		if(istr[k] != ifind[k])
		{
			return false;
		}
	}

	return true;
}

bool ends_with(const std::string& istr, const std::string& ifind)
{
	int size = istr.length();
	int size_find = ifind.length();
	
	if(size_find > size)
	{
		return false;
	}

	for (int k = size - size_find, l = 0; k < size; k++, l++)
	{
		if(istr[k] != ifind[l])
		{
			return false;
		}
	}

	return true;
}


std::string replace_string(std::string subject, const std::string& search, const std::string& replace)
{
   size_t pos = 0;

   while ((pos = subject.find(search, pos)) != std::string::npos)
   {
      subject.replace(pos, search.length(), replace);
      pos += replace.length();
   }

   return subject;
}

template<typename T2, typename T1, class unary_operation> std::vector<T2> map(const std::vector<T1> & original, unary_operation mapping_function)
{
   std::vector<T2> mapped;

   std::transform(begin(original), end(original), std::back_inserter(mapped), mapping_function);

   return mapped;
}

std::string escape_char(char character)
{
   const std::unordered_map<char, std::string> escaped_special_characters =
   {
      { '.', "\\." },{ '|', "\\|" },{ '*', "\\*" },{ '?', "\\?" },
      { '+', "\\+" },{ '(', "\\(" },{ ')', "\\)" },{ '{', "\\{" },
      { '}', "\\}" },{ '[', "\\[" },{ ']', "\\]" },{ '^', "\\^" },
      { '$', "\\$" },{ '\\', "\\\\" }
   };

   auto it = escaped_special_characters.find(character);

   if (it == escaped_special_characters.end())
   {
      return std::string(1, character);
   }

   return it->second;
}

std::string escape_string(const std::string& str)
{
   std::stringstream stream;

   std::for_each(begin(str), end(str), [&stream](const char character) { stream << escape_char(character); });

   return stream.str();
}

std::vector<std::string> escape_strings(const std::vector<std::string>& delimiters)
{
   return map<std::string>(delimiters, escape_string);
}

std::string str_join(const std::vector<std::string>& tokens, const std::string& delimiter)
{
   std::stringstream stream;

   stream << tokens.front();
   std::for_each(begin(tokens) + 1, end(tokens), [&](const std::string &elem) { stream << delimiter << elem; });

   return stream.str();
}

std::vector<std::string> str_split(const std::string& str, const std::vector<std::string> & delimiters)
{
   std::regex rgx(str_join(escape_strings(delimiters), "|"));

   std::sregex_token_iterator first{ begin(str), end(str), rgx, -1 }, last;

   return{ first, last };
}

std::vector<std::string> str_split(const std::string& str, const std::string& delimiter)
{
   std::vector<std::string> delimiters = { delimiter };

   return str_split(str, delimiters);
}
