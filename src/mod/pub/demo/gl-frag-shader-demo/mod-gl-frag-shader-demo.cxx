#include "stdafx.hxx"
#include "appplex-conf.hxx"

#ifdef MOD_GL_FRAG_SHADER_DEMO

#include "mod-gl-frag-shader-demo.hxx"
#include "com/mws/mws-camera.hxx"
#include "com/mws/mws-ext.hxx"
#include "com/util/util.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo.hxx"
#include "gfx-tex.hxx"
#include "gfx-scene.hxx"
#include "ext/gfx-surface.hxx"
#include "gfx.hxx"
#include "gfx-camera.hxx"
#include "gfx-rt.hxx"
#include "gfx-shader.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-vxo.hxx"
#include "gfx-state.hxx"
#include "ext/gfx-surface.hxx"
#include "utils/free-camera.hxx"
#include "rng/rng.hxx"
#include <glm/inc.hpp>
#include <string>
#include <vector>


class shader_state
{
public:
	enum e_input_type
	{
		e_no_input,
		e_keyboard,
		e_texture_2d,
		e_cube_map,
		e_music,
	};

	class shader_channel
	{
	public:
		static mws_sp<shader_channel> nwi()
		{
			return mws_sp<shader_channel>(new shader_channel("", e_no_input));
		}

		static mws_sp<shader_channel> nwi(std::string iinput_name, e_input_type iinput_type)
		{
			return mws_sp<shader_channel>(new shader_channel(iinput_name, iinput_type));
		}

		std::string get_uniform_value()
		{
			switch (input_type)
			{
			case shader_state::e_no_input:
			case shader_state::e_keyboard:
			case shader_state::e_music:
				return "";
			}

			return input_name;
		}

		std::string get_uniform_type()
		{
			switch (input_type)
			{
			case shader_state::e_no_input:
				return "sampler2D";

			case shader_state::e_keyboard:
				return "sampler2D";

			case shader_state::e_texture_2d:
				return "sampler2D";

			case shader_state::e_cube_map:
				return "samplerCube";

			case shader_state::e_music:
				return "sampler2D";
			}

			return "";
		}

		std::string input_name;
		e_input_type input_type;

	private:
		shader_channel(std::string iinput_name, e_input_type iinput_type)
		{
			input_name = iinput_name;
			input_type = iinput_type;
		}
	};

	shader_state()
	{
		needs_update = true;

		for (int k = 0; k < 4; k++)
		{
			channel_list.push_back(shader_channel::nwi());
		}
	}

	std::string name;
	bool needs_update;
	std::vector<mws_sp<shader_channel> > channel_list;
	uint32 start_time;
	uint32 stop_time;
	uint32 pause_time;

	std::string get_channel_uniform_type(int ichannel_idx)
	{
		return channel_list[ichannel_idx]->get_uniform_type();
	}

	void set_channel_input_name(int ichannel_idx, std::string iname)
	{
		auto idx = input_list.find(iname);

		if (idx != input_list.end())
		{
			mws_sp<shader_channel> ichannel = idx->second;

			channel_list[ichannel_idx] = ichannel;
		}
		else
		{
			channel_list[ichannel_idx] = shader_channel::nwi(iname, channel_list[ichannel_idx]->input_type);
		}
	}

private:
	friend class mod_gl_frag_shader_demo;
	static std::unordered_map<std::string, mws_sp<shader_channel> > input_list;

	static void init()
	{
		// no iput
		input_list["ni"] = shader_channel::nwi("ni", e_no_input);
		// keyboard
		input_list["kb"] = shader_channel::nwi("kb", e_keyboard);
		// 2d textures
		input_list["t0"] = shader_channel::nwi("tex00", e_texture_2d);
		input_list["t1"] = shader_channel::nwi("tex01", e_texture_2d);
		input_list["t2"] = shader_channel::nwi("tex02", e_texture_2d);
		input_list["t3"] = shader_channel::nwi("tex03", e_texture_2d);
		input_list["t4"] = shader_channel::nwi("tex04", e_texture_2d);
		input_list["t5"] = shader_channel::nwi("tex05", e_texture_2d);
		input_list["t6"] = shader_channel::nwi("tex06", e_texture_2d);
		input_list["t7"] = shader_channel::nwi("tex07", e_texture_2d);
		input_list["t8"] = shader_channel::nwi("tex08", e_texture_2d);
		input_list["t9"] = shader_channel::nwi("tex09", e_texture_2d);
		input_list["t10"] = shader_channel::nwi("tex10", e_texture_2d);
		input_list["t11"] = shader_channel::nwi("tex11", e_texture_2d);
		input_list["t12"] = shader_channel::nwi("tex12", e_texture_2d);
		input_list["t13"] = shader_channel::nwi("tex15", e_texture_2d);
		input_list["t14"] = shader_channel::nwi("tex16", e_texture_2d);
		input_list["t15"] = shader_channel::nwi("tex14", e_texture_2d);
		// cube maps
		input_list["c0"] = shader_channel::nwi("cube00", e_cube_map);
		input_list["c1"] = shader_channel::nwi("cube01", e_cube_map);
		input_list["c2"] = shader_channel::nwi("cube02", e_cube_map);
		input_list["c3"] = shader_channel::nwi("cube03", e_cube_map);
		input_list["c4"] = shader_channel::nwi("cube04", e_cube_map);
		input_list["c5"] = shader_channel::nwi("cube05", e_cube_map);
		// music
		input_list["m0"] = shader_channel::nwi("electronebulae", e_music);
		input_list["m1"] = shader_channel::nwi("experiment", e_music);
		input_list["m2"] = shader_channel::nwi("8 bit mentality", e_music);
		input_list["m3"] = shader_channel::nwi("X'TrackTure", e_music);
		input_list["m4"] = shader_channel::nwi("ourpithyator", e_music);
		input_list["m5"] = shader_channel::nwi("Tropical Beeper", e_music);
		input_list["m6"] = shader_channel::nwi("Most Geometric Person", e_music);
	}
};

std::unordered_map<std::string, mws_sp<shader_state::shader_channel> > shader_state::input_list;


class add_header_uniforms : public gfx_shader_listener
{
public:
	static mws_sp<add_header_uniforms> nwi(mws_sp<shader_state> istate)
	{
		return mws_sp<add_header_uniforms>(new add_header_uniforms(istate));
	}

	virtual const mws_sp<std::string> on_before_submit_vsh_source(mws_sp<gfx_shader> gp, const mws_sp<std::string> ishader_src)
	{
		return ishader_src;
	}

	virtual const mws_sp<std::string> on_before_submit_fsh_source(mws_sp<gfx_shader> gp, const mws_sp<std::string> ishader_src)
	{
		std::string fsh =
      "#ifdef GL_ES\n\
      precision highp float;\n\
      #else \n\
      #define lowp \n\
      #define mediump \n\
      #define highp \n\
      #endif \n\
      // shadertoy\n\
      uniform vec3 iResolution; \n\
      uniform float iGlobalTime; \n\
      uniform float iChannelTime[4]; \n\
      uniform vec3 iChannelResolution[4]; \n\
      uniform vec4 iMouse; \n\
      uniform vec4 iDate; \n\
      uniform float iSampleRate;\n";


		std::string channel_list = "";

		for (int k = 0; k < 4; k++)
			// find the data for each channel
		{
			std::string channel = trs("iChannel{}", k);
			std::string channel_id = channel + ":";
			auto idx = ishader_src->find(channel_id);
			bool invalid = true;

			if (idx != ishader_src->npos)
			{
				int idx_start = idx + channel_id.length();
				auto idx_end = ishader_src->find('\n', idx_start);

				if (idx_end != ishader_src->npos)
				{
					std::string name = ishader_src->substr(idx_start, idx_end - idx_start);
					name = trim(name);

					if (!name.empty())
					{
						state->set_channel_input_name(k, name);

						std::string uni_type = state->get_channel_uniform_type(k);
						channel_list += "uniform " + uni_type + " " + channel + ";\n";
						invalid = false;
					}
				}
			}

			if (invalid)
			{
				state->set_channel_input_name(k, "");
				channel_list += "uniform sampler2D " + channel + ";\n";
			}
		}

		fsh += channel_list;
		fsh.append(*ishader_src);

		if (ishader_src->find("mainImage") != std::string::npos)
		{
			fsh.append("\nvoid main()\n{\nmainImage(gl_FragColor, gl_FragCoord.xy);\n}\n");
		}
		
		state->needs_update = true;

		mws_sp<std::string> src = mws_sp<std::string>(new std::string(fsh));
		//mws_print("%s\n", src->c_str());

		return src;
	}

private:
	add_header_uniforms(mws_sp<shader_state> istate)
	{
		state = istate;
	}

	mws_sp<shader_state> state;
};


class mod_gl_frag_shader_demo_impl
{
public:
	mod_gl_frag_shader_demo_impl(mws_sp<mod_gl_frag_shader_demo> i_mod)
	{
		mws_mod = i_mod;
		current_fx_index = 0;
		is_active = true;
	}

	void load(mws_sp<mod_gl_frag_shader_demo> u)
	{
		std::string active_shader = "Antialiasing";
		active_shader = "";
		mws_sp<pfm_path> path = pfm_path::get_inst("", "fx-shaders");
		mws_sp<std::vector<mws_sp<pfm_file> > > file_list = path->list_directory(mws_mod.lock(), true);
		auto it = file_list->begin();

		shader_state_list.clear();

		for (; it != file_list->end(); it++)
		{
			mws_sp<pfm_file> file = *it;
			shader_state_list.push_back(mws_sp<shader_state>(new shader_state()));
			mws_sp<shader_state> ss = shader_state_list.back();

			ss->name = file->get_file_stem();
			ss->pause_time = ss->start_time = ss->stop_time = 0;
		}

		for (int k = 0; k < shader_state_list.size(); k++)
		{
			if (shader_state_list[k]->name == active_shader)
			{
				current_fx_index = k;
				break;
			}
		}

		ortho_cam = gfx_camera::nwi();
		ortho_cam->camera_id = "ortho_cam";
		ortho_cam->rendering_priority = 1;
		ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
		ortho_cam->near_clip_distance = -100;
		ortho_cam->far_clip_distance = 100;
		ortho_cam->clear_color = false;
		ortho_cam->clear_color_value = gfx_color(0xff, 0, 0);
		ortho_cam->clear_depth = true;


		float scale = 1.f;
		tex_width = u->get_width() * scale;
		tex_height = u->get_height() * scale;
		current_rt_index = 0;

		texture_display = gfx::i()->shader.new_program("texture_display", "basic_tex_fx");

		for (int k = 0; k < 2; k++)
		{
         gfx_tex_params prm;

         prm.set_format_id("RGBA8");
         prm.set_rt_params();
         rt_tex_vect[k] = gfx::i()->tex.nwi(gfx_tex::gen_id(), tex_width, tex_height, &prm);
			rt_vect[k] = gfx::i()->rt.new_rt();
			rt_vect[k]->set_color_attachment(rt_tex_vect[k]);
			gfx::i()->rt.set_current_render_target(rt_vect[k]);
		}

		gfx::i()->rt.set_current_render_target(nullptr);
		fx_quad = mws_sp<gfx_plane>(new gfx_plane());
		screen_quad = mws_sp<gfx_plane>(new gfx_plane());

		gfx_plane& rfx_quad = *fx_quad;
		rfx_quad.set_dimensions(2, 2);
		//fx_quad[MP_CULL_FRONT] = false;
		rfx_quad[MP_CULL_BACK] = false;
		rfx_quad[MP_DEPTH_TEST] = false;

		gfx_plane& rscreen_quad = *screen_quad;
		rscreen_quad.set_dimensions(2, 2);
		//rscreen_quad[MP_CULL_FRONT] = false;
		rscreen_quad[MP_CULL_BACK] = false;
		rscreen_quad[MP_DEPTH_TEST] = false;
		rscreen_quad[MP_SHADER_NAME] = "basic_tex_fx";
		mws_mod.lock()->gfx_scene_inst->attach(screen_quad);
		mws_mod.lock()->gfx_scene_inst->attach(ortho_cam);
		screen_quad->camera_id_list.push_back(ortho_cam->camera_id());

		set_fx(current_fx_index);

		mws_report_gfx_errs();
	}

	mws_sp<shader_state> get_shader_state()
	{
		return shader_state_list[current_fx_index];
	}

	void prev_fx()
	{
		int fx_index = (current_fx_index - 1);

		if (fx_index < 0)
		{
			fx_index = shader_state_list.size() - 1;
		}

		set_fx(fx_index);
	}

	void next_fx()
	{
		set_fx((current_fx_index + 1) % shader_state_list.size());
	}

	void set_fx(int ifx_index)
	{
		mws_sp<shader_state> ss_old = shader_state_list[current_fx_index];
		current_fx_index = ifx_index;
		mws_sp<shader_state> ss = get_shader_state();
		const std::string& shader_name = ss->name;

		fx_glsl = gfx::i()->shader.get_program_by_name(shader_name);

		if (!fx_glsl)
		{
			fx_glsl = gfx::i()->shader.new_program(shader_name, "fx", shader_name, add_header_uniforms::nwi(ss));
		}

		uint32 crt_time = pfm::time::get_time_millis();

		if (ss->start_time == 0)
		{
			ss->stop_time = ss->start_time = crt_time;
		}

		ss_old->stop_time = crt_time;
		ss->pause_time += crt_time - ss->stop_time;
		ss->needs_update = true;
	}

	void reset_time()
	{
		mws_sp<shader_state> ss = shader_state_list[current_fx_index];
		uint32 crt_time = pfm::time::get_time_millis();

		ss->pause_time = 0;
		ss->start_time = ss->stop_time = crt_time;
	}

	void set_active(bool iis_active)
	{
		if (is_active != iis_active)
		{
			mws_sp<shader_state> ss = shader_state_list[current_fx_index];
			uint32 crt_time = pfm::time::get_time_millis();

			is_active = iis_active;

			if (is_active)
			{
				ss->pause_time += crt_time - ss->stop_time;
			}
			else
			{
				ss->stop_time = crt_time;
			}
		}
	}

	void update_params()
	{
		mws_sp<shader_state> ss = get_shader_state();
		gfx_plane& rfx_quad = *fx_quad;

		//rfx_quad[MP_SHADER_INST] = fx_glsl;
		rfx_quad[MP_SHADER_NAME][MP_VSH_NAME] = fx_glsl->get_vertex_shader_name();
		rfx_quad[MP_SHADER_NAME][MP_FSH_NAME] = fx_glsl->get_fragment_shader_name();
		//gl_ctrl::set_current_program(fx_glsl);
		glm::vec2 dim((float)tex_width, (float)tex_height);
		glm::vec3 iResolution((float)tex_width, (float)tex_height, 0.f);
		//fx_glsl->update_uniform("u_v2_dim", glm::value_ptr(dim));
		//fx_glsl->update_uniform("iResolution", glm::value_ptr(iResolution));
		rfx_quad["u_v2_dim"] = dim;
		rfx_quad["iResolution"] = iResolution;

		for (int k = 0; k < ss->channel_list.size(); k++)
		{
			mws_sp<shader_state::shader_channel> ch = ss->channel_list[k];
			std::string name = trs("iChannel{}", k);
			std::string val = ch->get_uniform_value();

			if (val.empty())
			{
				val = "missing";
			}

			rfx_quad[name] = val;
		}
	}

	int tex_width;
	int tex_height;
	mws_sp<gfx_camera> ortho_cam;
	mws_sp<gfx_shader> fx_glsl;
	mws_sp<gfx_shader> texture_display;
	mws_sp<gfx_plane> fx_quad;
	mws_sp<gfx_plane> screen_quad;
	mws_sp<gfx_rt> rt_vect[2];
	mws_sp<gfx_tex> rt_tex_vect[2];
	int current_rt_index;
	int current_fx_index;
	std::vector<mws_sp<shader_state> > shader_state_list;
	bool is_active;
	mws_wp<mod_gl_frag_shader_demo> mws_mod;
};


class mod_gl_frag_shader_demo_page : public mws_page
{
public:
	mod_gl_frag_shader_demo_page() { set_id("gl-frag-shader-demo-page"); }
	virtual ~mod_gl_frag_shader_demo_page(){}

	virtual void init()
	{
	}

	void on_load()
	{
		reset();
	}

	virtual void receive(mws_sp<mws_dp> idp)
	{
		mws_page::update_input_sub_mws(idp);

		if (idp->is_processed())
		{
			return;
		}

		if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
		{
			mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);
			int tsh = 5;

			switch (ts->type)
			{
			case pointer_evt::touch_began:
			{
				pressed_pos = glm::vec2(ts->points[0].x, ts->points[0].y);
				ts->process();
				break;
			}

			case pointer_evt::touch_ended:
			{
				mws_sp<mod_gl_frag_shader_demo> u = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
				glm::vec2 screen_size((float)u->get_width(), (float)u->get_height());

				pressed_pos = glm::vec2(0.f, screen_size.y);
				ts->process();
				break;
			}

			case pointer_evt::touch_moved:
			{
				int x = ts->points[0].x;
				int y = ts->points[0].y;
				int k = std::max(std::min(x, pfm::screen::get_width() - tsh), tsh);
				int l = std::max(std::min(y, pfm::screen::get_height() - tsh), tsh);
				pointer_pos = glm::vec2(ts->points[0].x, ts->points[0].y);
				ts->process();
				break;
			}
			}
		}
		else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
		{
			mws_sp<key_evt> ke = key_evt::as_key_evt(idp);

			if (ke->get_type() == key_evt::KE_PRESSED)
			{
				bool do_action = true;

				switch (ke->get_key())
				{
				case KEY_LEFT:
				{
					prev_fx();
					break;
				}

				case KEY_RIGHT:
				{
					next_fx();
					break;
				}

				case KEY_SPACE:
				{
					toggle_active();
					break;
				}

				case KEY_R:
				{
					reset();
					break;
				}

				default:
					do_action = false;
				}

				if (do_action)
				{
					ke->process();
				}
			}
		}

		mws_page::update_input_std_behaviour(idp);
	}

	void update_state() override
	{
		mws_sp<mod_gl_frag_shader_demo> u = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = u->p;
		mws_sp<shader_state> ss = p->shader_state_list[p->current_fx_index];

		if (p->is_active)
		{
			int rt_index = p->current_rt_index;
			int next_rt_index = (rt_index + 1) % 2;
			back_buffer = p->rt_tex_vect[next_rt_index];
			front_buffer = p->rt_tex_vect[rt_index];

			gfx::i()->shader.set_current_program(p->fx_glsl);

			// update uniforms
			glm::vec2 screen_size((float)u->get_width(), (float)u->get_height());
			glm::vec2 tex_size((float)p->tex_width, (float)p->tex_height);
			glm::vec2 tex_scale = tex_size / screen_size;
			glm::vec2 pointer_position = glm::vec2(pointer_pos.x, screen_size.y - pointer_pos.y) * tex_scale;
			glm::vec2 pressed_position = glm::vec2(pressed_pos.x, screen_size.y - pressed_pos.y) * tex_scale;

			glm::vec2 dim = tex_size;
			float time = (pfm::time::get_time_millis() - ss->start_time - ss->pause_time) / 1000.f;
			//mws_print("timeee %f\n", time / 5.f);
			glm::vec2 mouse(pointer_position / tex_size);

			glm::vec3 iResolution(tex_size, 0.f);
			float iGlobalTime = time;
			glm::vec4 iMouse(pointer_position, pressed_position);
			glm::vec4 iDate(0.5f);
			float iSampleRate = 0.f;

			//p->fx_glsl->update_uniform("u_v2_dim", glm::value_ptr(dim));
			//p->fx_glsl->update_uniform("u_v1_time", &time);
			//p->fx_glsl->update_uniform("u_v2_mouse", glm::value_ptr(mouse));

			//p->fx_glsl->update_uniform("iResolution", glm::value_ptr(iResolution));
			//p->fx_glsl->update_uniform("iGlobalTime", &iGlobalTime);
			////p->fx_glsl->update_uniform("iChannelTime", &time);
			////p->fx_glsl->update_uniform("iChannelResolution", glm::value_ptr(dim));
			//p->fx_glsl->update_uniform("iMouse", glm::value_ptr(iMouse));
			//p->fx_glsl->update_uniform("iDate", glm::value_ptr(iDate));
			//p->fx_glsl->update_uniform("iSampleRate", &iSampleRate);


			p->current_rt_index = next_rt_index;

			mws_sp<shader_state> ss = p->get_shader_state();
			gfx_plane& fx_quad = *p->fx_quad;

			if (ss->needs_update)
			{
				ss->needs_update = false;
				p->update_params();
			}

			// tdx3
			fx_quad["u_s2d_tex"] = back_buffer->get_name();
			fx_quad["u_v2_dim"] = dim;
			fx_quad["u_v1_time"] = time;
			fx_quad["u_v2_mouse"] = mouse;

			// glslsandbox
			fx_quad["backbuffer"] = back_buffer->get_name();
			fx_quad["resolution"] = dim;
			// NEW: This 'uniform' vec2 indicates the size of the visible area of the virtual surface.
			fx_quad["surfaceSize"] = dim;
			fx_quad["time"] = time;
			fx_quad["mouse"] = mouse;

			// shadertoy
			fx_quad["iResolution"] = iResolution;
			fx_quad["iGlobalTime"] = iGlobalTime;
			fx_quad["iMouse"] = iMouse;
			fx_quad["iDate"] = iDate;
			fx_quad["iSampleRate"] = iSampleRate;
			//fx_quad["iChannelResolution[0]"] = iResolution;

			gfx::i()->rt.set_current_render_target(p->rt_vect[rt_index]);
			// draw into the frontbuffer
			mws_sp<gfx_state> gl_st = gfx::i()->get_gfx_state();
			decl_scgfxpl(plist)
			{
				{ gl::COLOR_CLEAR_VALUE, 0.f, 0.f, 0.f, 1.f }, { gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL }, {},
			};
			gl_st->set_state(plist);
			fx_quad.draw_out_of_sync(p->ortho_cam);
		}

		gfx_plane& screen_quad = *p->screen_quad;
		screen_quad["u_s2d_tex"] = front_buffer->get_name();
		gfx::i()->rt.set_current_render_target(nullptr);

		mws_report_gfx_errs();

		mws_page::update_state();
	}

	virtual void update_view(mws_sp<mws_camera> g)
	{
		mws_sp<mod_gl_frag_shader_demo> u = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = u->p;
		mws_sp<shader_state> ss = p->shader_state_list[p->current_fx_index];

		g->drawText(ss->name, 10, 10);

		mws_report_gfx_errs();

		mws_page::update_view(g);
	}

	void reset()
	{
		mws_sp<mod_gl_frag_shader_demo> u = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = u->p;
		glm::vec2 screen_size((float)u->get_width(), (float)u->get_height());

		pressed_pos = pointer_pos = glm::vec2(0.f, screen_size.y);

		reset_time();
	}

	void prev_fx()
	{
		mws_sp<mod_gl_frag_shader_demo> mws_mod = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = mws_mod->p;

		p->prev_fx();
	}

	void next_fx()
	{
		mws_sp<mod_gl_frag_shader_demo> mws_mod = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = mws_mod->p;

		p->next_fx();
	}

	void reset_time()
	{
		mws_sp<mod_gl_frag_shader_demo> mws_mod = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = mws_mod->p;

		p->reset_time();
	}

	void toggle_active()
	{
		mws_sp<mod_gl_frag_shader_demo> mws_mod = static_pointer_cast<mod_gl_frag_shader_demo>(get_mod());
		mws_sp<mod_gl_frag_shader_demo_impl> p = mws_mod->p;

		p->set_active(!p->is_active);
	}

	mws_sp<gfx_tex> back_buffer;
	mws_sp<gfx_tex> front_buffer;
	glm::vec2 pressed_pos;
	glm::vec2 pointer_pos;
};


void mod_gl_frag_shader_demo::init_mws()
{
	mws_cam->clear_color = false;
   mws_root->new_page<mod_gl_frag_shader_demo_page>();
}


mod_gl_frag_shader_demo::mod_gl_frag_shader_demo() : mws_mod(mws_stringify(MOD_GL_FRAG_SHADER_DEMO)) {}

mws_sp<mod_gl_frag_shader_demo> mod_gl_frag_shader_demo::nwi()
{
	mws_sp<mod_gl_frag_shader_demo> inst(new mod_gl_frag_shader_demo());
	inst->p = mws_sp<mod_gl_frag_shader_demo_impl>(new mod_gl_frag_shader_demo_impl(inst));
	return inst;
}

void mod_gl_frag_shader_demo::init()
{
   mws_print("init frag shader demo...");
	shader_state::init();
}

void mod_gl_frag_shader_demo::load()
{
	mws_sp<mod_gl_frag_shader_demo_page> page = static_pointer_cast<mod_gl_frag_shader_demo_page>(mws_root->page_tab[0]);

	p->load(static_pointer_cast<mod_gl_frag_shader_demo>(get_smtp_instance()));
	page->on_load();
}

#endif
