#include "stdafx.h"

#include "unit-gen-texture.hpp"

#ifdef UNIT_GEN_TEXTURE

#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "gfx-tex.hpp"
#include "gfx-state.hpp"
#include <VG/openvg.h>


unit_gen_texture::unit_gen_texture()
{
	set_name("gen-texture");
}

shared_ptr<unit_gen_texture> unit_gen_texture::nwi()
{
	return shared_ptr<unit_gen_texture>(new unit_gen_texture());
}

void unit_gen_texture::init()
{
}


namespace unit_gen_texture_texture
{
	void draw_texture(shared_ptr<mws_camera> g, int iwidth, int iheight)
	{
		//spvg_path fg = vg_util::new_ellipse(0,0, 100, 60);
		//VGfloat white[] = {0,0,1,1};
		//VGfloat dashes[] = {4.0f, 2.0f};

		//spvg_paint strokePaint = vg_paint::nwi();
		//spvg_paint fillPaint = vg_paint::nwi();
		//strokePaint->set_color_rgba_ub(0x000000ff);
		//fillPaint->set_color_rgba_ub(0xffff00ff);

		//vgSetPaint(fillPaint->handle(), VG_FILL_PATH);
		//vgSetPaint(strokePaint->handle(), VG_STROKE_PATH);

		//g->clear(0, 0, iwidth, iheight, 0xff);
		////vgSetfv(VG_CLEAR_COLOR, 4, white);
		////vgClear(0, 0, iwidth, iheight);

		//vgSeti(VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);

		//g->sf.set_line_width(2.5f);
		//g->sf.set_miter_limit(10.5f);
		//g->sf.set_cap_style(vg::cap_style::CAP_BUTT);
		//g->sf.set_join_style(vg::join_style::JOIN_MITER);
		//g->sf.set_dash_pattern(dashes, 2);
		//g->sf.set_dash_phase(0.5f);
		//g->sf.set_dash_phase_reset(true);

		////vgSetf(VG_STROKE_LINE_WIDTH, 2.5f);
		////vgSetf(VG_STROKE_MITER_LIMIT, 10.5f);
		////vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
		////vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_MITER);
		////vgSetfv(VG_STROKE_DASH_PATTERN, 2, dashes);
		////vgSetf(VG_STROKE_DASH_PHASE, 0.5f);
		////vgSeti(VG_STROKE_DASH_PHASE_RESET, VG_TRUE);

		//float s = 3;

		//vgLoadIdentity();
		//vgScale(s, s);
		//vgTranslate(50, 30);
		//vgDrawPath(fg->handle(), VG_STROKE_PATH | VG_FILL_PATH);
		//vgLoadIdentity();
	}
}


namespace unit_gen_texture_rtt_screen
{
	shared_ptr<gfx_tex> shvg_tex;

	class page_rtt_screen : public mws_page
	{
	public:
		page_rtt_screen(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();

			//int width = 256;
			//int height = 256;
			//shvg_tex = vg_image::nwi(width, height);
			//shared_ptr<renderer> r = renderer::get_instance();

			//r->rtt.start_rtt_screen_from_image(shvg_tex);

			//unit_gen_texture_texture::draw_texture(gfx_openvg::get_instance(), width, height);

			//r->rtt.stop_rtt();

			//signal_opengl_error();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			//shared_ptr<renderer> r = renderer::get_instance();
			//r->tx.set_texture_id(shvg_tex->get_texture_id());
			//r->g2d.draw_quad(0, 0, shvg_tex->get_width(), shvg_tex->get_height());

			g->drawText("rtt_screen", 10, 20);
		}
	};
}


namespace unit_gen_texture_rtt_fbo
{
	shared_ptr<gfx_tex> shvg_tex;

	class page_rtt_fbo : public mws_page
	{
	public:
		page_rtt_fbo(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();

			//int width = 64;
			//int height = width;
			//shvg_tex = vg_image::nwi(width, height);
			//float d = 0.25;
			//shared_ptr<renderer> r = renderer::get_instance();

			//r->rtt.start_rtt_fbo_from_image(rtt_fbo::FBO_STENCIL_ATTACHMENT, shvg_tex);

			//r->tx.set_texture_id(unit_gen_texture_rtt_screen::shvg_tex->get_texture_id());
			//r->g2d.draw_quad(0-d, 0-d, width+2*d, height+2*d);
			////unitgentexture_texture::draw_texture(gfx_openvg::get_instance(), width, height);

			//r->rtt.stop_rtt();

			//signal_opengl_error();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			//shared_ptr<renderer> r = renderer::get_instance();
			//r->tx.set_texture_id(shvg_tex->get_texture_id());
			//r->g2d.draw_quad(0, 0, shvg_tex->get_width(), shvg_tex->get_height());

			g->drawText("rtt_fbo", 10, 20);
		}
	};
}


namespace unit_gen_texture_mainpage
{
	class mainpage : public mws_page
	{
	public:
		mainpage(shared_ptr<mws_page_tab> iparent) : mws_page(iparent){}

		virtual void init()
		{
			mws_page::init();
		}

		virtual void receive(shared_ptr<iadp> idp)
		{
			mws_page::receive(idp);
		}

		virtual void update_state()
		{
			mws_page::update_state();
		}

		virtual void update_view(shared_ptr<mws_camera> g)
		{
			mws_page::update_view(g);

			//shared_ptr<renderer> r = renderer::get_instance();
			////unitgentexture_texture::draw_texture(gfx_openvg::get_instance(), 256, 256);
			//decl_scgfxpl(pl1)
			//{
			//	{gl::BLEND, gl::TRUE_GL}, {gl::BLEND_SRC_DST, gl::ONE_GL, gl::ONE_GL},
			//	{},
			//};
			//r->st.set_state(pl1);

			//r->tx.set_texture_id(unit_gen_texture_rtt_screen::shvg_tex->get_texture_id());
			//r->g2d.draw_quad(0, 0, 256, 256);
			//r->tx.set_texture_id(unit_gen_texture_rtt_fbo::shvg_tex->get_texture_id());
			//r->g2d.draw_quad(0, 0, 256, 256);

			//decl_scgfxpl(pl2)
			//{
			//	{gl::BLEND, gl::FALSE_GL}, {gl::BLEND_SRC_DST, gl::ONE_GL, gl::ZERO_GL},
			//	{},
			//};
			//r->st.set_state(pl2);

			//signal_opengl_error();

			const std::string& text = get_unit()->get_name();

			g->drawText(text, 10, 20);
		}
	};
}


void unit_gen_texture::init_mws()
{
	mws_page::new_shared_instance(new unit_gen_texture_mainpage::mainpage(mws_root));
	mws_page::new_shared_instance(new unit_gen_texture_rtt_screen::page_rtt_screen(mws_root));
	mws_page::new_shared_instance(new unit_gen_texture_rtt_fbo::page_rtt_fbo(mws_root));
}

void unit_gen_texture::load()
{
}

#endif
