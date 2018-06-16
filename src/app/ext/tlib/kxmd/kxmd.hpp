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
	// i_path is like xxx.yyy.zzz
   static shared_ptr<kx_process> get_px(std::string i_path, shared_ptr<kx_block> i_root);
   static mws_any get_kxmd_value(std::string i_path, shared_ptr<kx_block> i_root, mws_any i_default_val = mws_any());
	static std::vector<std::string> get_kxmd_str_seq(std::string i_path, shared_ptr<kx_block> i_root, std::vector<std::string> i_default_val = {});
	// i_path is like xxx.yyy.zzz
	static bool kxmd_path_exists(std::string i_path, shared_ptr<kx_block> i_root);
};


#endif
