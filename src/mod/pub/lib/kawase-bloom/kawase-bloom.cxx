#include "stdafx.hxx"

#include "kawase-bloom.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"
#include "mws/font-db.hxx"
#include "gfx.hxx"
#include "gfx-pbo.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "gfx-tex.hxx"

mws_sp<mws_kawase_bloom> mws_kawase_bloom::nwi(mws_sp<gfx_tex> i_input_tex)
{
   mws_sp<mws_kawase_bloom> inst = mws_sp<mws_kawase_bloom>(new mws_kawase_bloom());
   if (i_input_tex) { inst->init(i_input_tex); }
   return inst;
}

mws_sp<gfx_tex> mws_kawase_bloom::get_blurred_tex() const { return output_tex; }
mws_sp<gfx_tex> mws_kawase_bloom::get_bloom_tex() const { return accumulation_buff.get_tex(); }

void mws_kawase_bloom::init(mws_sp<gfx_tex> i_input_tex)
{
   input_tex = i_input_tex;

   if (!kawase_blur_shader)
   {
      init_shaders();
   }

   // ortho cam
   {
      ortho_cam = gfx_camera::nwi();
      ortho_cam->camera_id = "ortho_cam";
      ortho_cam->rendering_priority = 1;
      ortho_cam->projection_type = gfx_camera::e_orthographic_proj;
      ortho_cam->near_clip_distance = -100;
      ortho_cam->far_clip_distance = 100;
      ortho_cam->clear_color = true;
      ortho_cam->clear_color_value = gfx_color::colors::blue;
      ortho_cam->clear_depth = true;
   }
   // ping pong buffers
   {
      gfx_tex_params prm;

      prm.set_rt_params();
      kawase_blur_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(kawase_blur_sh_id);

      for (uint32 k = 0; k < ping_pong_vect.size(); k++)
      {
         mws_gfx_ppb& rt = ping_pong_vect[k];

         rt.init(mws_to_str_fmt("tex-%d", k), input_tex->get_width(), input_tex->get_width(), &prm);
         (*rt.get_quad())[MP_SHADER_NAME] = kawase_blur_sh_id;
         rt.get_quad()->set_v_flip(true);
         gfx::i()->rt.set_current_render_target(rt.get_rt());
         rt.get_rt()->clear_buffers();
      }

      (*ping_pong_vect[0].get_quad())["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[1].get_tex();
      (*ping_pong_vect[1].get_quad())["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[0].get_tex();

      // accumulation buffer
      {
         mws_gfx_ppb& rt = accumulation_buff;

         accumulation_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(accumulation_sh_id);
         rt.init(mws_to_str_fmt("tex-acc-buff"), input_tex->get_width(), input_tex->get_width(), &prm);
         auto& rvxo = *rt.get_quad();

         rvxo[MP_BLENDING] = MV_ADD;
         rvxo[MP_SHADER_NAME] = accumulation_sh_id;
         rvxo.set_v_flip(true);
         gfx::i()->rt.set_current_render_target(rt.get_rt());
         rt.get_rt()->clear_buffers();
      }

      gfx::i()->rt.set_current_render_target();
   }
   // tex quad
   {
      input_quad = gfx_quad_2d::nwi();
      auto& rvxo = *input_quad;

      rvxo.camera_id_list = { ortho_cam->camera_id };
      rvxo[MP_SHADER_NAME] = gfx::basic_tex_sh_id;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = input_tex;
      rvxo.set_scale((float)input_tex->get_width(), (float)input_tex->get_width());
      rvxo.set_v_flip(true);
   }
}

void mws_kawase_bloom::update()
{
   mws_sp<gfx_rt> rt = ping_pong_vect[1].get_rt();

   // put the input texture in input_tex ping_pong_vect[1].tex, to be used by ping_pong_vect[0]
   gfx::i()->rt.set_current_render_target(rt);
   input_quad->draw_out_of_sync(ortho_cam);
   gfx::i()->rt.set_current_render_target();
   // set mul fact
   (*accumulation_buff.get_quad())["u_v1_mul_fact"] = u_v1_mul_fact;

   for (uint32 k = 0; k < iteration_count; k++)
   {
      float sample_factor = sample_offset_start_val + k;
      mws_gfx_ppb& rt = ping_pong_vect[k % 2];

      gfx::i()->rt.set_current_render_target(rt.get_rt());
      (*rt.get_quad())["u_v2_offset"] = glm::vec2(sample_factor / rt.get_tex()->get_width(), sample_factor / rt.get_tex()->get_width());
      rt.get_quad()->draw_out_of_sync(ortho_cam);
      output_tex = rt.get_tex();
      gfx::i()->rt.set_current_render_target(accumulation_buff.get_rt());
      accumulation_buff.get_quad()->draw_out_of_sync(ortho_cam);
      gfx::i()->rt.set_current_render_target();
   }
}

mws_kawase_bloom::mws_kawase_bloom() {}

void mws_kawase_bloom::init_shaders()
{
   kawase_blur_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(kawase_blur_sh_id, true);
   if (!kawase_blur_shader)
   {
      auto vsh = mws_sp<std::string>(new std::string(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      layout(location = 0) in vec3 a_v3_position;
      layout(location = 1) in vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      smooth out vec2 v_v2_tex_coord;

      void main()
      { 	 	
	      v_v2_tex_coord = a_v2_tex_coord;
	      gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = mws_sp<std::string>(new std::string(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision highp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform sampler2D u_s2d_tex;
      uniform vec2 u_v2_offset;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
          vec3 v3_col;
	
	      v3_col = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
          v3_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
          v3_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
          v3_col += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
          v3_col *= 0.25;

	      v4_frag_color = vec4(v3_col, 1.0);
      }
      )"
      ));

      kawase_blur_shader = gfx::i()->shader.new_program_from_src(kawase_blur_sh_id, vsh, fsh);
   }
   accumulation_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(accumulation_sh_id, true);
   if (!accumulation_shader)
   {
      auto vsh = mws_sp<std::string>(new std::string(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      layout(location = 0) in vec3 a_v3_position;
      layout(location = 1) in vec2 a_v2_tex_coord;

      uniform mat4 u_m4_model_view_proj;

      smooth out vec2 v_v2_tex_coord;

      void main()
      { 	 	
	      v_v2_tex_coord = a_v2_tex_coord;
	      gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
      }
      )"
      ));

      auto fsh = mws_sp<std::string>(new std::string(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision highp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform sampler2D u_s2d_tex;
      uniform float u_v1_mul_fact;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
          vec3 v3_col = texture(u_s2d_tex, v_v2_tex_coord).rgb;
          v3_col *= u_v1_mul_fact;

	      v4_frag_color = vec4(v3_col, 1.0);
      }
      )"
      ));

      accumulation_shader = gfx::i()->shader.new_program_from_src(accumulation_sh_id, vsh, fsh);
   }
}