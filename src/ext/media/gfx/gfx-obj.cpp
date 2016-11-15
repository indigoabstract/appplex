#include "stdafx.h"

#include "gfx-obj.hpp"


uint32 gfx_obj::obj_idx = 0;

gfx_obj::gfx_obj()
{
	obj_idx++;
}

shared_ptr<gfx_obj> gfx_obj::get_inst()
{
	return shared_from_this();
}

const std::string& gfx_obj::get_id()
{
	return oid;
}
