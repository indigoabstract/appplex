#include "stdafx.h"

#include "gfx-obj.hpp"
#include "gfx.hpp"


uint32 gfx_obj::obj_idx = 0;

std::shared_ptr<gfx_obj> gfx_obj::get_inst()
{
   return shared_from_this();
}

//const std::string& gfx_obj::get_id()
//{
//   return oid;
//}

gfx_obj::gfx_obj(std::shared_ptr<gfx> i_gi)
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

std::shared_ptr<gfx> gfx_obj::gi()
{
   return g.lock();
}
