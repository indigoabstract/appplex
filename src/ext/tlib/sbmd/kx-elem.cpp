#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_SBMD

#include "kx-elem.hpp"
#include "min.hpp"
#include <boost/foreach.hpp>

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


shared_ptr<kx_whitespace> kx_whitespace::new_instance(){ return shared_ptr<kx_whitespace>(new kx_whitespace()); }

string kx_whitespace::print(){ return data; }


shared_ptr<kx_comma> kx_comma::new_instance(){ return shared_ptr<kx_comma>(new kx_comma()); }

string kx_comma::print(){ return ","; }


shared_ptr<kx_async_flowop> kx_async_flowop::new_instance(){ return shared_ptr<kx_async_flowop>(new kx_async_flowop()); }

string kx_async_flowop::print()
{
	std::string s = "->>";

	if (fltype == afl_left)
	{
		s = "<<-";
	}

	return s;
}

shared_ptr<kx_flowop> kx_flowop::new_instance(){ return shared_ptr<kx_flowop>(new kx_flowop()); }

string kx_flowop::print()
{
	std::string s = "->";

	if (fltype == fl_left)
	{
		s = "<-";
	}

	return s;
}


bool kx_process::is_process(){ return true; }

string kx_process::print(){ return "kx_process"; }

shared_ptr<kx_process> kx_process::find_by_name(const std::string& iname)
{
	if (get_name() == iname)
	{
		return static_pointer_cast<kx_process>(get_inst());
	}

	return nullptr;
}


shared_ptr<kx_symbol> kx_symbol::new_instance(){ return shared_ptr<kx_symbol>(new kx_symbol()); }

string kx_symbol::print(){ return name; }

void kx_symbol::eval()
{
	trx("sym[%1%]") % name;
}


shared_ptr<kx_text> kx_text::new_instance(){ return shared_ptr<kx_text>(new kx_text()); }

std::string kx_text::print(){ return data; }

void kx_text::eval()
{
	trx("text[%1%]") % data;
}


shared_ptr<kx_block> kx_block::new_instance(){ return shared_ptr<kx_block>(new kx_block()); }

string kx_block::print()
{
	std::string s = "";

	if (name)
	{
		s.append(name->name);
	}

	s.append("[");
	BOOST_FOREACH(shared_ptr<kx_elem> ke, list)
	{
		s.append(ke->print());
	}
	s.append("]");

	return s;
}

void kx_block::eval()
{
	trc("block ");

	if (name)
	{
		trc("%1%") % name->name;
	}

	trc("[");

	BOOST_FOREACH(shared_ptr<kx_elem> ke, list)
	{
		ke->eval();
	}

	trc("]");
}

shared_ptr<kx_process> kx_block::find_by_name(const std::string& iname)
{
	if (get_name() == iname)
	{
		return static_pointer_cast<kx_process>(get_inst());
	}

	BOOST_FOREACH(shared_ptr<kx_elem> ke, list)
	{
		shared_ptr<kx_process> f = ke->find_by_name(iname);

		if (f)
		{
			return f;
		}
	}

	return nullptr;
}


shared_ptr<kx_ignore_block> kx_ignore_block::new_instance(){ return shared_ptr<kx_ignore_block>(new kx_ignore_block()); }

string kx_ignore_block::print()
{
	std::string s = "@@";

	if (name)
	{
		s.append(name->name);
	}

	if (!body.empty())
	{
		s.append("[");
		s.append(body);
		s.append("]");
	}

	return s;
}


shared_ptr<kx_match_block> kx_match_block::new_instance(){ return shared_ptr<kx_match_block>(new kx_match_block()); }

string kx_match_block::print()
{
	std::string s = "?";

	if (name)
	{
		s.append(name->name);
	}

	s.append("[");
	BOOST_FOREACH(shared_ptr<kx_elem> ke, blk->list)
	{
		s.append(ke->print());
	}
	s.append("]");

	return s;
}


shared_ptr<kx_meta_block> kx_meta_block::new_instance(){ return shared_ptr<kx_meta_block>(new kx_meta_block()); }

std::string kx_meta_block::print()
{
	std::string s = "@";

	if (name)
	{
		s.append(name->name);
	}

	if (blk)
	{
		s.append("[");
		BOOST_FOREACH(shared_ptr<kx_elem> ke, blk->list)
		{
			s.append(ke->print());
		}
		s.append("]");
	}

	return s;
}

#endif
