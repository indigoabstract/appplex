#pragma once

#include "kx-parser.hxx"
#include <string>
#include <vector>

class kx_block;


class kx_krte
{
public:
   kx_krte() {}
   virtual ~kx_krte() {}

   void set_src(mws_sp<std::string> isrc);
   void run();
   std::string to_string();
   void print();

public:
   mws_sp<std::string> src;
   mws_sp<kx_block> kxb;
   kx_parser kxp;
};
