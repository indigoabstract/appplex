#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_SBMD

#include "sbmd.hpp"
#include "min.hpp"
#include <sbmd/kx-krte.hpp>
#include <sbmd/kx-elem.hpp>
#include <boost/any.hpp>
#include <boost/algorithm/string.hpp>


std::vector<shared_ptr<kx_process> > sbmd_ops::get_process_list(const shared_ptr<kx_process> ikp)
{
	std::vector<shared_ptr<kx_process> > list;

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


std::vector<std::string> sbmd_ops::get_process_name_list(const std::vector<shared_ptr<kx_process> >& ilist)
{
	std::vector<std::string> list;

	for (auto& i : ilist)
	{
		list.push_back(i->get_name());
	}

	return list;
}


std::vector<std::string> sbmd_ops::get_process_name_list(const shared_ptr<kx_process> ikp)
{
	std::vector<shared_ptr<kx_process> > l1 = get_process_list(ikp);
	std::vector<std::string> l2;

	if (!l1.empty())
	{
		l2 = get_process_name_list(l1);
	}

	return l2;
}


bool sbmd_ops::get_bool_from_list(const std::vector<std::string>& ilist)
{
	if (ilist.empty())
	{
		throw ia_exception("list is empty");
	}

	if (ilist[0] == "false")
	{
		return false;
	}
	else if (ilist[0] == "true")
	{
		return true;
	}

	throw ia_exception("parse error");
}


// ipath is like xxx.yyy.zzz
boost::any sbmd_ops::get_sbmd_value(std::string ipath, shared_ptr<kx_block> iroot, boost::any default_val)
{
	std::vector<std::string> tokens;
	auto pred = boost::is_any_of(".");
	boost::split(tokens, ipath, pred, boost::token_compress_on);
	shared_ptr<kx_process> xdb = iroot;

	if (ipath == "units.kappaxx.platf")
	{
		int x = 3;
	}
	for (auto& xdb_name : tokens)
	{
		auto sub_xdb = xdb->find_by_name(xdb_name);
		xdb = sub_xdb;

		if (!xdb)
		{
			break;
		}
	}

	boost::any result;

	if (xdb)
	{
		auto values = get_process_name_list(xdb);
		result = values;
	}
	else
	{
		result = default_val;
	}

	return result;
}

std::vector<std::string> sbmd_ops::get_sbmd_str_seq(std::string ipath, shared_ptr<kx_block> iroot, std::vector<std::string> default_val)
{
	std::vector<std::string> seq;
	boost::any val = get_sbmd_value(ipath, iroot);

	if (!val.empty())
	{
		try
		{
			std::function<int(int)> is_quote = [](int c) { return c == '\'' || c == '"'; };

			seq = boost::any_cast<std::vector<std::string>>(val);

			// clear ' and " from the string
			for (std::string& s : seq)
			{
				s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(is_quote)));
				s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(is_quote)).base(), s.end());
			}
		}
		catch (boost::bad_any_cast& e)
		{
			trx(e.what());
		}
	}

	if (seq.empty())
	{
		seq = default_val;
	}

	return seq;
}


// ipath is like xxx.yyy.zzz
bool sbmd_ops::sbmd_path_exists(std::string ipath, shared_ptr<kx_block> iroot)
{
	std::size_t found = ipath.find_last_of(".");

	if (found > 0)
	{
		std::string stem = ipath.substr(0, found);
		std::string leaf = ipath.substr(found + 1, ipath.length() - found - 1);
		auto seq = get_sbmd_str_seq(stem, iroot);
		auto idx = std::find(seq.begin(), seq.end(), leaf);

		if (idx != seq.end())
		{
			return true;
		}
	}

	return false;
}


#endif
