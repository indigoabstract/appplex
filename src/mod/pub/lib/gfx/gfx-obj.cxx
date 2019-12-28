#include "stdafx.hxx"

#include "gfx-obj.hxx"
#include "gfx.hxx"


gfx_obj::~gfx_obj() {}

mws_sp<gfx_obj> gfx_obj::get_inst()
{
   return shared_from_this();
}

gfx_obj::gfx_obj(mws_sp<gfx> i_gi)
{
   mws_assert(gfx::i() != nullptr);

   if (i_gi)
   {
      g = i_gi;
   }
   else
   {
      g = gfx::i();
   }

   obj_idx++;
}

mws_sp<gfx> gfx_obj::gi()
{
   return g.lock();
}
