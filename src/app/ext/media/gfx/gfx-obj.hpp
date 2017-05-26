#pragma once

#include "pfm.hpp"
#include <string>

class gfx;


class gfx_obj : public enable_shared_from_this<gfx_obj>
{
public:
	enum e_gfx_obj_type
	{
		e_gfx_rt,
		e_gfx_shader,
		e_gfx_tex,
	};

	shared_ptr<gfx_obj> get_inst();
	const std::string& get_id();
	virtual e_gfx_obj_type get_type()const = 0;
	virtual bool is_valid()const = 0;

protected:
	gfx_obj();

	std::string oid;

private:
	friend class gfx;

	static uint32 obj_idx;
};
