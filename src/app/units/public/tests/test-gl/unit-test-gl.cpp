#include "stdafx.h"

#include "unit-test-gl.hpp"

#ifdef UNIT_TEST_GL

#include "gfx.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-vxo.hpp"
#include "gfx-state.hpp"
#include "ext/gfx-surface.hpp"
#include "com/ux/ux-camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


unit_test_gl::unit_test_gl()
{
	set_name("test-gl");
}

shared_ptr<unit_test_gl> unit_test_gl::new_instance()
{
	return shared_ptr<unit_test_gl>(new unit_test_gl());
}

void unit_test_gl::init()
{
	trx("test-gl...");
}

namespace test_gl
{
	shared_ptr<gfx_rt> rt;
	shared_ptr<gfx_tex> rt_tex;
	shared_ptr<gfx_plane> quad_mesh;
}

using namespace test_gl;

void unit_test_gl::load()
{
	gfx_util::check_gfx_error();

	{
		rt_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), 256, 256);
		rt = gfx::rt::new_rt();
		rt->set_color_attachment(rt_tex);

		shared_ptr<gfx_state> gl_st = gfx::get_gfx_state();

		gfx::rt::set_current_render_target(rt);
		decl_scgfxpl(pl1)
		{
			{gl::COLOR_CLEAR_VALUE, 1.f, 1.f, 1.f, 0.7f},
			{ gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL },
			{},
		};
		gl_st->set_state(pl1);
		gfx::rt::set_current_render_target(shared_ptr<gfx_rt>());
	}

	quad_mesh = shared_ptr<gfx_plane>(new gfx_plane());

	float sx = 512, sy = 256, sz = 1;
	float tx = 50, ty = 100, tz = 0;
	auto& qm = *quad_mesh;

	qm.set_dimensions(1, 1);
	qm.scaling = glm::vec3(sx, sy, sz);
	qm.position = glm::vec3(sx / 2 + tx, sy / 2 + ty, tz);
	qm[MP_SHADER_NAME] = "basic_tex";
	qm[MP_BLENDING] = MV_ALPHA;
	qm["u_s2d_tex"] = rt_tex->get_name();

	gfx_color cc;

	ux_cam->clear_color = true;
	cc.from_float(0.5f, 0.f, 1.f, 1.f);
	ux_cam->clear_color_value = cc;

	gfx_util::check_gfx_error();
}

void unit_test_gl::update_view(int update_count)
{
	gfx_util::check_gfx_error();
	quad_mesh->render_mesh(ux_cam);
	gfx_util::check_gfx_error();

	unit::update_view(update_count);
}

#endif
