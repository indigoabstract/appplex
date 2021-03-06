#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_TEST_GL_OUTLINES

#include "mws-mod.hxx"
#include "pfm.hxx"


class mod_test_gl_outlines : public mws_mod
{
public:
   static mws_sp<mod_test_gl_outlines> nwi();

   virtual void load() override;
   virtual void update_view(uint32_t update_count) override;

private:
   mod_test_gl_outlines();
};

#endif
