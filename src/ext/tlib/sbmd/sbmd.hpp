#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_SBMD

#include "pfm.hpp"
#include <boost/any.hpp>
#include <string>
#include <vector>

class kx_process;
class kx_block;


class sbmd_ops
{
public:
	static std::vector<shared_ptr<kx_process> > get_process_list(const shared_ptr<kx_process> ikp);
	static std::vector<std::string> get_process_name_list(const std::vector<shared_ptr<kx_process> >& ilist);
	static std::vector<std::string> get_process_name_list(const shared_ptr<kx_process> ikp);
	static bool get_bool_from_list(const std::vector<std::string>& ilist);
	// ipath is like xxx.yyy.zzz
	static boost::any get_sbmd_value(std::string ipath, shared_ptr<kx_block> iroot, boost::any default_val = boost::any());
	static std::vector<std::string> get_sbmd_str_seq(std::string ipath, shared_ptr<kx_block> iroot, std::vector<std::string> default_val = {});
	// ipath is like xxx.yyy.zzz
	static bool sbmd_path_exists(std::string ipath, shared_ptr<kx_block> iroot);
};


#endif
