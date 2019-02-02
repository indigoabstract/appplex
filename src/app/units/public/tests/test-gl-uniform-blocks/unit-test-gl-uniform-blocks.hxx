#pragma once

#include "appplex-conf.hxx"

#ifdef UNIT_TEST_GL_UNIFORM_BLOCKS

#include "unit.hxx"
#include "pfm.hxx"


class unit_test_gl_uniform_blocks : public unit
{
public:
   static mws_sp<unit_test_gl_uniform_blocks> nwi();

   virtual void load() override;
   virtual void update_view(int update_count) override;

private:
   unit_test_gl_uniform_blocks();
};

#endif
