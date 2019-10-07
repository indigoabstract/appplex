#pragma once

#include "mws-mod.hxx"
#include "gfx-pbo.hxx"
#include <array>


class gfx_shader;


class mws_kawase_bloom
{
public:
   uint32 iteration_count = 19;
   float u_v1_mul_fact = 0.745f;
   float sample_offset_start_val = 1.5f;

   static mws_sp<mws_kawase_bloom> nwi(mws_sp<gfx_tex> i_input_tex = nullptr);
   mws_sp<gfx_tex> get_blurred_tex() const;
   mws_sp<gfx_tex> get_bloom_tex() const;
   void init(mws_sp<gfx_tex> i_input_tex);
   void update();

protected:
   mws_kawase_bloom();
   void init_shaders();

   mws_sp<gfx_camera> ortho_cam;
   mws_sp<gfx_tex> input_tex;
   mws_sp<gfx_quad_2d> input_quad;
   mws_sp<gfx_tex> output_tex;
   std::array<mws_gfx_ppb, 2> ping_pong_vect;
   mws_sp<gfx_shader> kawase_blur_shader;
   static const inline std::string kawase_blur_sh_id = "kawase-blur";
   mws_gfx_ppb accumulation_buff;
   mws_sp<gfx_shader> accumulation_shader;
   static const inline std::string accumulation_sh_id = "accumulation";
   static inline uint32 tex_count = 0;
};
