#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "pfm.hpp"
#include "min.hpp"
#include <string>
#include <vector>

class kx_process;
class kx_block;


class kxmd_ops
{
public:
	static std::vector<shared_ptr<kx_process> > get_process_list(const shared_ptr<kx_process> ikp);
	static std::vector<std::string> get_process_name_list(const std::vector<shared_ptr<kx_process> >& ilist);
	static std::vector<std::string> get_process_name_list(const shared_ptr<kx_process> ikp);
	static bool get_bool_from_list(const std::vector<std::string>& ilist);
	// ipath is like xxx.yyy.zzz
	static ia_any get_kxmd_value(std::string ipath, shared_ptr<kx_block> iroot, ia_any default_val = ia_any());
	static std::vector<std::string> get_kxmd_str_seq(std::string ipath, shared_ptr<kx_block> iroot, std::vector<std::string> default_val = {});
	// ipath is like xxx.yyy.zzz
	static bool kxmd_path_exists(std::string ipath, shared_ptr<kx_block> iroot);
};


#endif
