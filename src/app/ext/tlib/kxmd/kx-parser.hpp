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
   std::shared_ptr<kx_elem> parse_src(std::shared_ptr<std::string> src);
};

#endif
