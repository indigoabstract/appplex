#pragma once

#include "ext/gfx-surface.hpp"
#include <string>
#include <vector>

class gfx;


class gfx_quad_2d : public gfx_plane
{
public:
	enum e_anchor_types
	{
		e_top_left,
		e_center,
		e_btm_center,
	};

	gfx_quad_2d();
	void set_anchor(e_anchor_types ianchor_type);
	void set_translation(float ix, float iy);
	void set_rotation(float ia);
	void set_scale(float ix, float iy);
	void set_v_flip(bool iv_flip);
	void set_dimensions(float idx, float idy) override;
	void set_tex_coord(glm::vec2 lt, glm::vec2 rt, glm::vec2 rb, glm::vec2 lb);

private:
	e_anchor_types anchor_type;
	float dx, dy;
	float a;
	float tx, ty;
	float sx, sy;
};
