#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GL_UNIFORM_BLOCKS

#include "unit.hpp"
#include "pfm.hpp"


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
