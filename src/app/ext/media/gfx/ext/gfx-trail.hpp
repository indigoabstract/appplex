#pragma once

#include "gfx-vxo.hpp"
#include "ext/gfx-surface.hpp"
#include <deque>


class gfx_trail : public gfx_vxo
{
public:
	gfx_trail();
	virtual ~gfx_trail();
	void add_position(glm::vec3 ipos);

private:

	int max_positions;
	float line_thickness;
	std::deque<glm::vec3> positions;
};
