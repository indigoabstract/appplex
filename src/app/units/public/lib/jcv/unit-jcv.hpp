#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_JCV

#include "unit.hpp"


class unit_jcv : public unit
{
public:
   static mws_sp<unit_jcv> nwi();

   virtual void init() override;
   virtual void init_mws() override;
   virtual void load() override;

private:
   unit_jcv();
};

#endif
