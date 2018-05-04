#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_TEST_GL_UNIFORM_BLOCKS

#include "unit-test-gl-uniform-blocks.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-material.hpp"


unit_test_gl_uniform_blocks::unit_test_gl_uniform_blocks()
{
   set_name("test-gl-uniform-blocks");
}

mws_sp<unit_test_gl_uniform_blocks> unit_test_gl_uniform_blocks::nwi()
{
   return mws_sp<unit_test_gl_uniform_blocks>(new unit_test_gl_uniform_blocks());
}


namespace unit_test_gl_uniform_blocks_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init()
      {
      }

      virtual void receive(mws_sp<iadp> idp)
      {
         mws_page::receive(idp);
      }

      virtual void update_state()
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> g)
      {
         mws_page::update_view(g);
      }
   };
}


void unit_test_gl_uniform_blocks::init_mws()
{
   auto page = mws_page::new_shared_instance(mws_root, new unit_test_gl_uniform_blocks_ns::main_page());
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::indigo;
}

#endif
