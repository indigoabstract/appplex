#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_KXMD

#include "unit.hxx"
#include <string>
#include <vector>


class kx_elem;


class kx_parser
{
public:
   mws_sp<kx_elem> parse_src(mws_sp<std::string> src);
};

#endif
