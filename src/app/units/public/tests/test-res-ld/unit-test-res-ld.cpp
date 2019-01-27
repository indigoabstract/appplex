#include "stdafx.h"

#include "unit-test-res-ld.hpp"

#ifdef UNIT_TEST_RES_LD

#include "gfx.hpp"
#include "gfx-camera.hpp"
#include "gfx-color.hpp"
#include "gfx-rt.hpp"
#include "gfx-shader.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-tex.hpp"
#include "gfx-state.hpp"
#include <glm/inc.hpp>


unit_test_res_ld::unit_test_res_ld() : unit(mws_stringify(UNIT_TEST_RES_LD)) {}

mws_sp<unit_test_res_ld> unit_test_res_ld::nwi()
{
	return mws_sp<unit_test_res_ld>(new unit_test_res_ld());
}

void unit_test_res_ld::init()
{
}

namespace test_gl
{
   mws_sp<gfx_camera> ortho_cam;
	mws_sp<gfx_quad_2d> quad_mesh;
}

using namespace test_gl;

void unit_test_res_ld::load()
{
   {
      ortho_cam = gfx_camera::nwi();
      ortho_cam->camera_id = "ortho_cam";
      ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
      ortho_cam->near_clip_distance = -100;
      ortho_cam->far_clip_distance = 100;
      ortho_cam->clear_color = true;
      ortho_cam->clear_color_value = gfx_color::colors::indigo;
   }

	mws_report_gfx_errs();

   {
      quad_mesh = gfx_quad_2d::nwi();
      auto& qm = *quad_mesh;

      qm.set_dimensions(1, 1);
      qm.set_scale(512, 512);
      qm.set_translation(40, 80.f);
      qm[MP_BLENDING] = MV_ALPHA;
      qm[MP_SHADER_NAME] = "basic-tex-shader";
      qm["u_s2d_tex"] = "rectangle.png";
   }

	mws_report_gfx_errs();
}

void unit_test_res_ld::update_view(int update_count)
{
   ortho_cam->clear_buffers();

	quad_mesh->draw_out_of_sync(ortho_cam);
	mws_report_gfx_errs();

	unit::update_view(update_count);
}

#endif
