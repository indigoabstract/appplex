#pragma once

#include "mws-mod.hxx"
#include <string>
#include <vector>


class kx_elem;


class kx_parser
{
public:
   mws_sp<kx_elem> parse_src(mws_sp<std::string> src);
};
