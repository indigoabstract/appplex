#include "stdafx.h"

#include "unit-test-freetype-gl.hpp"

#ifdef UNIT_TEST_FREETYPE_GL

#include "com/unit/input-ctrl.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-font.hpp"
#include "com/mws/font-db.hpp"
#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-camera.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-vxo.hpp"
#include "ext/gfx-surface.hpp"
#include "gfx-state.hpp"
#include "gfx-color.hpp"
#include <rng/rng.hpp>
#include <glm/glm.hpp>


class unit_test_freetype_gl_impl
{
public:
	unit_test_freetype_gl_impl()
	{
		last_time = 0;
	}

	shared_ptr<gfx_plane> q2d;
	shared_ptr<gfx_shader> texture_display;
	uint32 last_time;
	shared_ptr<mws_font> f;
	std::string tx;

};


unit_test_freetype_gl::unit_test_freetype_gl()
{
	set_name("test-freetype-gl");
}

shared_ptr<unit_test_freetype_gl> unit_test_freetype_gl::nwi()
{
	return shared_ptr<unit_test_freetype_gl>(new unit_test_freetype_gl());
}

void unit_test_freetype_gl::load()
{
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::indigo;
  
   p = shared_ptr<unit_test_freetype_gl_impl>(new unit_test_freetype_gl_impl());

	p->texture_display = gfx::i()->shader.get_program_by_name("basic-tex-shader");
	p->q2d = shared_ptr<gfx_plane>(new gfx_plane());
	gfx_plane& rq2d = *p->q2d;
	rq2d.set_dimensions(2, 2);
	rq2d[MP_CULL_BACK] = false;
	rq2d[MP_DEPTH_TEST] = false;
	rq2d[MP_BLENDING] = MV_ALPHA;
	rq2d[MP_SHADER_NAME] = "basic-tex-shader";
	rq2d.position = glm::vec3(850.f, 350.f, 0.f);
	rq2d.scaling = glm::vec3(256, 256, 1.f);

	mws_report_gfx_errs();
}

bool unit_test_freetype_gl::update()
{
	shared_ptr<gfx_tex> atlas = font_db::inst()->get_texture_atlas();

	if (atlas && atlas->is_valid())
	{
		(*p->q2d)["u_s2d_tex"] = atlas->get_name();
		mws_cam->draw_mesh(p->q2d);
	}

	uint32 crt_time = pfm::time::get_time_millis();

	if (crt_time - p->last_time > 750)
	{
		p->last_time = crt_time;

		RNG rng;
		float fnt_size = rng.range(7, 186);

		p->f = mws_font::nwi(fnt_size);
		p->f->set_color(gfx_color::colors::gold);
		p->tx.clear();
		int size = 5 + rng.nextInt(10);

		for (int k = 0; k < size; k++)
		{
			char c = (char)rng.range(32, 226);
			p->tx.push_back(c);
		}

	}

	mws_cam->drawText("alg-xxxxxxxx", 100, 100, p->f);
	glm::vec2 dim = p->f->get_text_dim("alg-xxxxxxxx");
	mws_cam->drawRect(100, 100, dim.x, dim.y);
	mws_cam->drawText("test freetype-gl", 20, 50);

	dim = p->f->get_text_dim(p->tx);
	mws_cam->drawText(p->tx, 50, 400, p->f);
	mws_cam->drawRect(50, 400, dim.x, dim.y);

	mws_report_gfx_errs();

	return unit::update();
}

void unit_test_freetype_gl::receive(shared_ptr<iadp> idp)
{
}

#endif
