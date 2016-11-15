#include "stdafx.h"

#include "min.hpp"
#include "unit-ctrl.hpp"


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
