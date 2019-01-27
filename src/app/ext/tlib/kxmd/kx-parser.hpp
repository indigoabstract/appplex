#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "unit.hpp"
#include <string>
#include <vector>


class kx_elem;


class kx_parser
{
public:
   mws_sp<kx_elem> parse_src(mws_sp<std::string> src);
};

#endif
