#include "stdafx.h"

#include "unit-test-gl.hpp"

#ifdef UNIT_TEST_GL

#include "gfx.hpp"
#include "gfx-camera.hpp"
#include "gfx-color.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-state.hpp"
#include <glm/inc.hpp>


unit_test_gl::unit_test_gl() : unit(mws_stringify(UNIT_TEST_GL)) {}

shared_ptr<unit_test_gl> unit_test_gl::nwi()
{
	return shared_ptr<unit_test_gl>(new unit_test_gl());
}

void unit_test_gl::init()
{
	mws_print("test-gl...\n");
}

namespace test_gl
{
   shared_ptr<gfx_camera> ortho_cam;
   shared_ptr<gfx_rt> rt;
	shared_ptr<gfx_tex> rt_tex;
	shared_ptr<gfx_quad_2d> quad_mesh;
}

using namespace test_gl;

void unit_test_gl::load()
{
   {
      gfx_color cc;

      ortho_cam = gfx_camera::nwi();
      ortho_cam->camera_id = "ortho_cam";
      ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
      ortho_cam->near_clip_distance = -100;
      ortho_cam->far_clip_distance = 100;
      ortho_cam->clear_color = true;
      cc.from_float(0.5f, 0.f, 1.f, 1.f);
      ortho_cam->clear_color_value = cc;

   }

	mws_report_gfx_errs();

	{
      gfx_tex_params prm;

      prm.set_format_id("RGBA8");
      prm.set_rt_params();
      rt_tex = gfx::i()->tex.nwi(gfx_tex::gen_id(), 256, 256, &prm);
		rt = gfx::i()->rt.new_rt();
		rt->set_color_attachment(rt_tex);

		shared_ptr<gfx_state> gl_st = gfx::i()->get_gfx_state();

		gfx::i()->rt.set_current_render_target(rt);
		decl_scgfxpl(pl1)
		{
			{gl::COLOR_CLEAR_VALUE, 1.f, 1.f, 1.f, 0.7f},
			{ gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL | gl::STENCIL_BUFFER_BIT_GL },
			{},
		};
		gl_st->set_state(pl1);
		gfx::i()->rt.set_current_render_target(shared_ptr<gfx_rt>());
	}

   {
      quad_mesh = gfx_quad_2d::nwi();
      auto& qm = *quad_mesh;

      qm.set_dimensions(1, 1);
      qm.set_scale(512, 256);
      qm.set_translation(40, 80.f);
      qm[MP_BLENDING] = MV_ALPHA;
      qm[MP_SHADER_NAME] = "basic-tex-shader";
      qm["u_s2d_tex"] = rt_tex->get_name();
   }

	mws_report_gfx_errs();
}

void unit_test_gl::update_view(int update_count)
{
   ortho_cam->clear_buffers();

	quad_mesh->draw_out_of_sync(ortho_cam);
	mws_report_gfx_errs();

	unit::update_view(update_count);
}

#endif
