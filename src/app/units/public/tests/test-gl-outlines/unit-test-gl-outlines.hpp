#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GL_OUTLINES

#include "unit.hpp"
#include "pfm.hpp"


class unit_test_gl_outlines : public unit
{
public:
   static mws_sp<unit_test_gl_outlines> nwi();

   virtual void load() override;
   virtual void update_view(int update_count) override;

private:
   unit_test_gl_outlines();
};

#endif
