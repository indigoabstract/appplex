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
   float get_line_thickness();
   void set_line_thickness(float iline_thickness);
   int get_max_positions();
   void set_max_positions(int imax_positions);

private:

	int max_positions;
	float line_thickness;
	std::deque<glm::vec3> positions;
};
