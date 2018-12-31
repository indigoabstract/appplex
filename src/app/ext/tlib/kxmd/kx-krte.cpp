#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef MOD_KXMD

#include "kx-krte.hpp"
#include "kx-elem.hpp"

using std::string;


void kx_krte::set_src(shared_ptr<std::string> isrc)
{
   src = isrc;
}

void kx_krte::run()
{
   shared_ptr<kx_elem> kxe;

   kxe = kxp.parse_src(src);

   if (!kxe)
   {
      trx("empty program");
      return;
   }

   kxb = static_pointer_cast<kx_block>(kxe);

   shared_ptr<kx_symbol> name(kx_symbol::nwi());
   name->name = "main-program";
   kxb->name = name;

   //for(auto ke : kxb->list)
   //{
   //	string s = ke->print();
   //	trx("{}", s);
   //}

   //trx("%1%") % kxb->print();

   //trx("\n\n");
   //kxb->eval();
   //trx("\n\n");
}

void kx_krte::print()
{
   for (auto ke : kxb->list)
   {
      string s = ke->print();
      trx("{}", s);
   }
}

#endif
