#include "stdafx.hxx"

#include "unit-test-freetype-gl.hxx"

#ifdef UNIT_TEST_FREETYPE_GL

#include "com/unit/input-ctrl.hxx"
#include "com/mws/mws-camera.hxx"
#include "com/mws/mws-font.hxx"
#include "com/mws/font-db.hxx"
#include "gfx.hxx"
#include "gfx-rt.hxx"
#include "gfx-camera.hxx"
#include "gfx-shader.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-vxo.hxx"
#include "ext/gfx-surface.hxx"
#include "gfx-state.hxx"
#include "gfx-color.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>


class unit_test_freetype_gl_impl
{
public:
	unit_test_freetype_gl_impl()
	{
		last_time = 0;
	}

	mws_sp<gfx_plane> q2d;
	mws_sp<gfx_shader> texture_display;
	uint32 last_time;
	mws_sp<mws_font> f;
	std::string tx;

};


unit_test_freetype_gl::unit_test_freetype_gl() : unit(mws_stringify(UNIT_TEST_FREETYPE_GL)) {}

mws_sp<unit_test_freetype_gl> unit_test_freetype_gl::nwi()
{
	return mws_sp<unit_test_freetype_gl>(new unit_test_freetype_gl());
}

void unit_test_freetype_gl::load()
{
   mws_cam->clear_color = true;
   mws_cam->clear_color_value = gfx_color::colors::indigo;
  
   p = mws_sp<unit_test_freetype_gl_impl>(new unit_test_freetype_gl_impl());

	p->texture_display = gfx::i()->shader.get_program_by_name("basic-tex-shader");
	p->q2d = mws_sp<gfx_plane>(new gfx_plane());
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
	mws_sp<gfx_tex> atlas = font_db::inst()->get_texture_atlas();

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

void unit_test_freetype_gl::receive(mws_sp<mws_dp> idp)
{
}

#endif
