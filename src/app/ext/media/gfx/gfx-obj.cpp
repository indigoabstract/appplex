#include "stdafx.h"

#include "gfx-obj.hpp"


uint32 gfx_obj::obj_idx = 0;

std::shared_ptr<gfx_obj> gfx_obj::get_inst()
{
   return shared_from_this();
}

const std::string& gfx_obj::get_id()
{
   return oid;
}

gfx_obj::gfx_obj(std::shared_ptr<gfx> i_gi)
{
   g = i_gi;
   obj_idx++;
}

std::shared_ptr<gfx> gfx_obj::gfx_ref()
{
   return g.lock();
}
