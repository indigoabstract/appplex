#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_FREEIMAGE

#include "mod.hxx"
#include <string>
#include <vector>


class mod_test_freeimage : public mws_mod
{
public:
   static mws_sp<mod_test_freeimage> nwi();

   virtual void init();
   virtual void init_mws();
   virtual void load();

   static void save_image(std::string i_filename, mws_sp<std::vector<uint32> > ibgra);

private:
   mod_test_freeimage();
};

#endif
