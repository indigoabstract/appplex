#include "stdafx.hxx"

#include "kawase-bloom.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"
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

mws_sp<gfx_tex> mws_kawase_bloom::get_blurred_tex() const { return blurred_tex; }

mws_sp<gfx_tex> mws_kawase_bloom::get_bloom_tex() const { return bloomed_tex; }

void mws_kawase_bloom::set_iter_count(uint32_t i_iter_count, float i_weight_fact)
{
   if (i_iter_count == 0 || weight_fact <= 0.f)
   {
      mws_throw mws_exception("invalid arguments");
   }

   iteration_count = i_iter_count;
   weight_fact = i_weight_fact;
   weight_fact_vect.assign(iteration_count, weight_fact);
}

void mws_kawase_bloom::set_iter_count(uint32_t i_iter_count, const std::vector<float>& i_weight_fact)
{
   if (i_iter_count == 0 || i_weight_fact.size() != i_iter_count)
   {
      mws_throw mws_exception("invalid arguments");
   }

   iteration_count = i_iter_count;
   weight_fact_vect = i_weight_fact;
   weight_fact = 0.f;
}

void mws_kawase_bloom::set_alpha_op_type(set_alpha_op_types i_alpha_op, float i_new_alpha_val)
{
   if (i_new_alpha_val < 0.f || i_new_alpha_val > 1.f)
   {
      mws_throw mws_exception("new alpha val out of range");
   }

   alpha_op = i_alpha_op;
   new_alpha_val = i_new_alpha_val;
}

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
      //prm.set_format_id("RGBA32F");
      kawase_blur_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(kawase_blur_sh_id);

      for (uint32_t k = 0; k < 2; k++)
      {
         mws_gfx_ppb& rt = ping_pong_vect[k];

         rt.init(mws_to_str_fmt("mws-kawase-bloom-tex-%d", tex_count), input_tex->get_width(), input_tex->get_height(), &prm);
         auto& rvxo = *rt.get_quad();
         rvxo[MP_SHADER_NAME] = kawase_blur_sh_id;
         rvxo.set_v_flip(true);
         gfx::i()->rt.set_current_render_target(rt.get_rt());
         rt.get_rt()->clear_buffers();
         tex_count++;
      }

      // accumulation buffer
      for (uint32_t k = 0; k < 2; k++)
      {
         mws_gfx_ppb& rt = accumulation_buff[k];

         accumulation_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(accumulation_sh_id);
         rt.init(mws_to_str_fmt("mws-kawase-bloom-tex-%d", tex_count), input_tex->get_width(), input_tex->get_height(), &prm);
         auto& rvxo = *rt.get_quad();

         rvxo[MP_SHADER_NAME] = accumulation_sh_id;
         rvxo.set_v_flip(true);
         gfx::i()->rt.set_current_render_target(rt.get_rt());
         rt.get_rt()->clear_buffers();
         tex_count++;
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
      rvxo.set_scale((float)input_tex->get_width(), (float)input_tex->get_height());
      rvxo.set_v_flip(true);
   }
}

void mws_kawase_bloom::update()
{
   if (iteration_count == 0)
   {
      mws_throw mws_exception("iteration_count must > 0");
   }

   if (weight_fact == 0.f && weight_fact_vect.size() != iteration_count)
   {
      mws_throw mws_exception("weights are set incorrectly");
   }

   // set uniforms
   {
      auto& rvxo = *ping_pong_vect[0].get_quad();
      rvxo["u_v1_alpha_op"] = static_cast<float>(alpha_op);
      rvxo["u_v1_alpha_val"] = new_alpha_val;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[1].get_tex();
   }
   {
      auto& rvxo = *ping_pong_vect[1].get_quad();
      rvxo["u_v1_alpha_op"] = static_cast<float>(alpha_op);
      rvxo["u_v1_alpha_val"] = new_alpha_val;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = ping_pong_vect[0].get_tex();
   }
   {
      auto& rvxo = *accumulation_buff[0].get_quad();
      rvxo["u_s2d_tex_1"][MP_TEXTURE_INST] = accumulation_buff[1].get_tex();
   }
   {
      auto& rvxo = *accumulation_buff[1].get_quad();
      rvxo["u_s2d_tex_1"][MP_TEXTURE_INST] = accumulation_buff[0].get_tex();
   }
   {
      mws_sp<gfx_rt> rt = ping_pong_vect[1].get_rt();
      // put the input texture in input_tex ping_pong_vect[1].tex, to be used by ping_pong_vect[0]
      gfx::i()->rt.set_current_render_target(rt);
      input_quad->draw_out_of_sync(ortho_cam);
      gfx::i()->rt.set_current_render_target();
   }

   for (uint32_t k = 0; k < iteration_count; k++)
   {
      uint32_t idx = k % 2;
      float sample_factor = sample_offset_start_val + k;
      mws_gfx_ppb& rt = ping_pong_vect[idx];
      mws_gfx_ppb& rt_acc = accumulation_buff[idx];

      gfx::i()->rt.set_current_render_target(rt.get_rt());
      (*rt.get_quad())["u_v2_offset"] = glm::vec2(sample_factor / rt.get_tex()->get_width(), sample_factor / rt.get_tex()->get_height());
      rt.get_quad()->draw_out_of_sync(ortho_cam);
      blurred_tex = rt.get_tex();
      gfx::i()->rt.set_current_render_target(rt_acc.get_rt());
      // set weight fact
      (*rt_acc.get_quad())["u_v1_weight_fact"] = weight_fact_vect[k];
      (*rt_acc.get_quad())["u_s2d_tex_0"][MP_TEXTURE_INST] = blurred_tex;
      rt_acc.get_quad()->draw_out_of_sync(ortho_cam);
      bloomed_tex = rt_acc.get_tex();
      gfx::i()->rt.set_current_render_target();
   }
}

mws_kawase_bloom::mws_kawase_bloom() {}

void mws_kawase_bloom::init_shaders()
{
   kawase_blur_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(kawase_blur_sh_id, true);
   if (!kawase_blur_shader)
   {
      std::string vsh(
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
      );

      std::string fsh(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision highp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform float u_v1_alpha_op;
      uniform float u_v1_alpha_val;
      uniform sampler2D u_s2d_tex;
      uniform vec2 u_v2_offset;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
          vec4 v4_col;
	
	      // e_set_alpha_to_blur
	      if(u_v1_alpha_op == 0.)
	      {
		      v4_col = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgba;
		      v4_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgba;
		      v4_col += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgba;
		      v4_col += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgba;
		      v4_col *= 0.25;
	      }
	      // e_set_alpha_to_original
	      else if(u_v1_alpha_op == 1.)
	      {
		      float v1_alpha = texture(u_s2d_tex, v_v2_tex_coord).a;
		
		      v4_col.rgb = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
		      v4_col.rgb *= 0.25;
		      v4_col.a = v1_alpha;
	      }
	      // e_set_alpha_to_new_val
	      else if(u_v1_alpha_op == 2.)
	      {
		      v4_col.rgb = texture(u_s2d_tex, v_v2_tex_coord + u_v2_offset).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(u_v2_offset.x, -u_v2_offset.y)).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord + vec2(-u_v2_offset.x, u_v2_offset.y)).rgb;
		      v4_col.rgb += texture(u_s2d_tex, v_v2_tex_coord - u_v2_offset).rgb;
		      v4_col.rgb *= 0.25;
		      v4_col.a = u_v1_alpha_val;
	      }
	
	      v4_frag_color = vec4(v4_col);
      }
      )"
      );

      kawase_blur_shader = gfx::i()->shader.new_program_from_src(kawase_blur_sh_id, vsh, fsh);
   }
   accumulation_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(accumulation_sh_id, true);
   if (!accumulation_shader)
   {
      std::string vsh(
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
      );

      std::string fsh(
         R"(
      //@es #version 300 es
      //@dt #version 330 core

      #ifdef GL_ES
	      precision highp float;
      #endif

      layout(location = 0) out vec4 v4_frag_color;

      uniform sampler2D u_s2d_tex_0;
      uniform sampler2D u_s2d_tex_1;
      uniform float u_v1_weight_fact;

      smooth in vec2 v_v2_tex_coord;

      void main()
      {
          vec4 v4_col_0 = texture(u_s2d_tex_0, v_v2_tex_coord).rgba;
          vec4 v4_col_1 = texture(u_s2d_tex_1, v_v2_tex_coord).rgba;
          vec3 v3_col = v4_col_0.rgb * u_v1_weight_fact + v4_col_1.rgb;

          v4_frag_color = vec4(v3_col, v4_col_0.a);
      }
      )"
      );

      accumulation_shader = gfx::i()->shader.new_program_from_src(accumulation_sh_id, vsh, fsh);
   }
}