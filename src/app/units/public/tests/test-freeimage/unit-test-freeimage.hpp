#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_FREEIMAGE

#include "unit.hpp"
#include <string>
#include <vector>


class unit_test_freeimage : public unit
{
public:
   static mws_sp<unit_test_freeimage> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

   static void save_image(std::string ifilename, mws_sp<std::vector<uint32> > ibgra);

private:
   unit_test_freeimage();
};

#endif
