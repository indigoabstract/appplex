#pragma once

#include "gfx-vxo.hpp"
#include <glm/fwd.hpp>

class mws_font;
class text_vxo_impl;


class text_vxo : public gfx_vxo
{
public:
	static shared_ptr<text_vxo> new_inst();
	virtual void clear_text();
	virtual void add_text(const std::string& itext, const glm::vec2& ipos, const shared_ptr<mws_font> ifont);
	virtual void render_mesh(shared_ptr<gfx_camera> icamera);

private:
	text_vxo();

	shared_ptr<text_vxo_impl> p;
};
