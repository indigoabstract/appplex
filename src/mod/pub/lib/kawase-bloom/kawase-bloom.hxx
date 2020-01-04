#pragma once

#include "mws-mod.hxx"
#include "gfx-pbo.hxx"
#include <array>


class gfx_shader;


class mws_kawase_bloom
{
public:
   enum set_alpha_op_types
   {
      e_set_alpha_to_blur = 0,
      e_set_alpha_to_original,
      e_set_alpha_to_new_val,
   };
   float sample_offset_start_val = 1.5f;

   static mws_sp<mws_kawase_bloom> nwi(mws_sp<gfx_tex> i_input_tex = nullptr);
   mws_sp<gfx_tex> get_blurred_tex() const;
   mws_sp<gfx_tex> get_bloom_tex() const;
   void set_iter_count(uint32 i_iter_count, float i_weight_fact);
   void set_iter_count(uint32 i_iter_count, const std::vector<float>& i_weight_fact);
   void set_alpha_op_type(set_alpha_op_types i_alpha_op, float i_new_alpha_val = 1.f);
   void init(mws_sp<gfx_tex> i_input_tex);
   void update();

protected:
   mws_kawase_bloom();
   void init_shaders();

   set_alpha_op_types alpha_op = e_set_alpha_to_blur;
   float new_alpha_val = 1.f;
   uint32 iteration_count = 0;
   float weight_fact = 0.f;
   std::vector<float> weight_fact_vect;
   mws_sp<gfx_camera> ortho_cam;
   mws_sp<gfx_tex> input_tex;
   mws_sp<gfx_quad_2d> input_quad;
   mws_sp<gfx_tex> blurred_tex;
   mws_sp<gfx_tex> bloomed_tex;
   std::array<mws_gfx_ppb, 2> ping_pong_vect;
   mws_sp<gfx_shader> kawase_blur_shader;
   static const inline std::string kawase_blur_sh_id = "kawase-blur";
   std::array<mws_gfx_ppb, 2> accumulation_buff;
   mws_sp<gfx_shader> accumulation_shader;
   static const inline std::string accumulation_sh_id = "accumulation";
   static inline uint32 tex_count = 0;
};
