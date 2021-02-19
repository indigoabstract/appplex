#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_FFMPEG

#include "mws-mod.hxx"


class mod_test_ffmpeg : public mws_mod
{
public:
   static mws_sp<mod_test_ffmpeg> nwi();

   virtual void init();
   virtual void load();
   virtual void update_view(uint32_t update_count);

private:
   mod_test_ffmpeg();
   virtual void receive(mws_sp<mws_dp> idp);
};

#endif
