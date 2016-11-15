#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_SBMD

#include "kx-parser.hpp"
#include <string>
#include <vector>

class kx_block;


class kx_krte
{
public:
	kx_krte(){}
	virtual ~kx_krte(){}

	void set_src(std::shared_ptr<std::string> isrc);
	void run();

public:
	std::shared_ptr<std::string> src;
	std::shared_ptr<kx_block> kxb;
	kx_parser kxp;
};

#endif
