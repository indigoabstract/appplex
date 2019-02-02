#include "stdafx.hxx"

#include "gfx-obj.hxx"
#include "gfx.hxx"


uint32 gfx_obj::obj_idx = 0;

mws_sp<gfx_obj> gfx_obj::get_inst()
{
   return shared_from_this();
}

//const std::string& gfx_obj::get_id()
//{
//   return oid;
//}

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
