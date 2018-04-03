#include "stdafx.h"

#include "unit-test-video-recording.hpp"

#ifdef UNIT_TEST_VIDEO_RECORDING

#include "public/tests/test-ffmpeg/ffmpeg/venc-ffmpeg.hpp"
#include "com/unit/input-ctrl.hpp"
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
#include "pfm-gl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>


unit_test_video_recording::unit_test_video_recording()
{
   set_name("test-video-recording");
}

shared_ptr<unit_test_video_recording> unit_test_video_recording::new_instance()
{
   return shared_ptr<unit_test_video_recording>(new unit_test_video_recording());
}

void unit_test_video_recording::init()
{
   trx("test_video_recording...");
}

namespace test_video_recording
{
   class impl : public app_impl
   {
   public:
      void load()
      {
         video_scale = 1.f;
         int video_width = int(gfx::rt::get_screen_width() * video_scale);
         int video_height = int(gfx::rt::get_screen_height() * video_scale);

         // video width & height must be multiple of 2
         if (video_width % 2 != 0)
         {
            video_width--;
         }

         if (video_height % 2 != 0)
         {
            video_height--;
         }

         frame_idx = 0;
         mws_report_gfx_errs();

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
            gfx::rt::set_current_render_target();
         }

         {
            quad_mesh = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            float sx = 512, sy = 256, sz = 1;
            float tx = 50, ty = 100, tz = 0;
            auto& qm = *quad_mesh;

            qm.set_dimensions(1, 1);
            qm.scaling = glm::vec3(sx, sy, sz);
            qm.position = glm::vec3(sx / 2 + tx, sy / 2 + ty, tz);
            qm[MP_SHADER_NAME] = "basic_tex";
            qm[MP_BLENDING] = MV_ALPHA;
            qm["u_s2d_tex"] = rt_tex->get_name();
         }

         {
            scr_mirror_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), gfx::rt::get_screen_width(), gfx::rt::get_screen_height());
            scr_mirror_mesh = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());
            scr_mirror_bg_mesh = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            float aspect_ratio = (float)gfx::rt::get_screen_width() / gfx::rt::get_screen_height();
            float sx = 512;
            float sy = sx / aspect_ratio, sz = 1;
            float tx = 600, ty = 250, tz = 0;

            {
               auto& msh = *scr_mirror_mesh;

               msh.set_dimensions(1, 1);
               msh.scaling = glm::vec3(sx, sy, sz);
               msh.position = glm::vec3(sx / 2 + tx, sy / 2 + ty, tz);
               msh[MP_SHADER_NAME] = "basic_tex";
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
               msh.set_v_flip(true);
            }

            {
               float dt = 20;
               float sx2 = sx + 2 * dt, sy2 = sy + 2 * dt, sz2 = 1;
               float tx2 = tx - dt, ty2 = ty - dt, tz2 = 0;
               auto& msh = *scr_mirror_bg_mesh;
               msh.set_dimensions(1, 1);
               msh.scaling = glm::vec3(sx2, sy2, sz2);
               msh.position = glm::vec3(sx2 / 2 + tx2, sy2 / 2 + ty2, tz2);
               msh[MP_SHADER_NAME] = "c_o";
               msh["u_v4_color"] = gfx_color::colors::black.to_vec4();
            }
         }

         // y rt
         {
            int rt_y_width = video_width;
            int rt_y_height = video_height;

            rt_y_tex = gfx::tex::new_tex_2d("u_s2d_y_tex", rt_y_width, rt_y_height, "R8");
            rt_y = gfx::rt::new_rt();
            rt_y->set_color_attachment(rt_y_tex);
            rt_y_quad = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            auto& msh = *rt_y_quad;

            msh.set_dimensions(1, 1);
            msh.set_scale((float)rt_y_width, (float)rt_y_height);
            msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
            msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-y-420.fsh";
            msh["u_s2d_tex"] = scr_mirror_tex->get_name();
         }

         // uv rt
         {
            int rt_uv_width = video_width / 2;
            int rt_uv_height = video_height / 2;

            rt_uv_tex = gfx::tex::new_tex_2d("u_s2d_uv_tex", rt_uv_width, rt_uv_height, "RGBA8");
            rt_uv = gfx::rt::new_rt();
            rt_uv->set_color_attachment(rt_uv_tex);
            rt_uv_quad = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            auto& msh = *rt_uv_quad;

            msh.set_dimensions(1, 1);
            msh.set_scale((float)rt_uv_width, (float)rt_uv_height);
            msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
            msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-uv-420.fsh";
            msh["u_s2d_tex"] = scr_mirror_tex->get_name();
         }

         {
            venc = std::make_shared<venc_ffmpeg>();
            video_path = "screen-recording.mp4";
            video_params.bit_rate = 1200 * 1000;
            video_params.width = video_width;
            video_params.height = video_height;
            // frames per second
            video_params.time_base = { 2, 50 };
            video_params.ticks_per_frame = 2;
            // emit one intra frame every ten frames
            video_params.gop_size = 10;
            video_params.max_b_frames = 1;
            video_params.pix_fmt = AV_PIX_FMT_YUV420P;
            video_params.codec_id = AV_CODEC_ID_H264;
         }

         mws_report_gfx_errs();
      }

      void update_view(std::shared_ptr<ux_camera> ux_cam)
      {
         std::string frame_counter = trs("frame count: {}", frame_idx);

         mws_report_gfx_errs();
         quad_mesh->render_mesh(ux_cam);
         ux_cam->drawText(frame_counter, 50, 50);
         mws_report_gfx_errs();
      }

      void post_update_view(std::shared_ptr<ux_camera> ux_cam)
      {
         mws_report_gfx_errs();
         scr_mirror_tex->set_active(0);
         glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, gfx::rt::get_screen_width(), gfx::rt::get_screen_height());
         scr_mirror_bg_mesh->render_mesh(ux_cam);
         //(*scr_mirror_mesh)["u_s2d_tex"] = rt_uv_tex->get_name();
         scr_mirror_mesh->render_mesh(ux_cam);
         mws_report_gfx_errs();

         if (venc->is_encoding())
         {
            gfx::rt::set_current_render_target(rt_y);
            rt_y_quad->render_mesh(ux_cam);
            shared_ptr<std::vector<uint8> > pixels_y_tex = gfx::rt::get_render_target_pixels<uint8>(rt_y);
            mws_report_gfx_errs();

            gfx::rt::set_current_render_target(rt_uv);
            rt_uv_quad->render_mesh(ux_cam);
            shared_ptr<std::vector<uint32> > pixels_uv_tex = gfx::rt::get_render_target_pixels<uint32>(rt_uv);
            gfx::rt::set_current_render_target();
            mws_report_gfx_errs();

            venc->encode_yuv420_frame(pixels_y_tex->data(), pixels_uv_tex->data());
         }

         frame_idx++;
      }

      void receive(shared_ptr<iadp> idp)
      {
         if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
         {
            shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);
         }
         else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
         {
            shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

            if (ke->is_pressed())
            {
               bool isAction = true;

               switch (ke->get_key())
               {
               case KEY_F4:
                  toggle_encoding();
                  break;

               default:
                  isAction = false;
               }

               if (isAction)
               {
                  ke->process();
               }
            }
         }
      }

      void toggle_encoding()
      {
         if (venc->is_encoding())
         {
            venc->stop_encoding();
            vprint("stop_encoding\n");
         }
         else
         {
            venc->start_encoding(video_path.c_str(), video_params);
            vprint("start_encoding\n");
         }
      }

      shared_ptr<gfx_rt> rt;
      shared_ptr<gfx_tex> rt_tex;
      shared_ptr<gfx_quad_2d> quad_mesh;

      shared_ptr<gfx_tex> scr_mirror_tex;
      shared_ptr<gfx_quad_2d> scr_mirror_mesh;
      shared_ptr<gfx_quad_2d> scr_mirror_bg_mesh;

      // data for converting rgb to yuv420
      shared_ptr<gfx_rt> rt_y;
      shared_ptr<gfx_tex> rt_y_tex;
      shared_ptr<gfx_quad_2d> rt_y_quad;
      shared_ptr<gfx_rt> rt_uv;
      shared_ptr<gfx_tex> rt_uv_tex;
      shared_ptr<gfx_quad_2d> rt_uv_quad;
      shared_ptr<venc_ffmpeg> venc;
      std::string video_path;
      video_params_ffmpeg video_params;

      int frame_idx;
      float video_scale;
   };
}
using namespace test_video_recording;

void unit_test_video_recording::load()
{
   p = std::make_unique<impl>();

   auto i = static_cast<impl*>(p.get());
   gfx_color cc;

   ux_cam->clear_color = true;
   cc.from_float(0.5f, 0.f, 1.f, 1.f);
   ux_cam->clear_color_value = cc;

   i->load();
}

void unit_test_video_recording::update_view(int update_count)
{
   auto i = static_cast<impl*>(p.get());

   i->update_view(ux_cam);
   unit::update_view(update_count);
   i->post_update_view(ux_cam);
}

void unit_test_video_recording::receive(shared_ptr<iadp> idp)
{
   if (!idp->is_processed())
   {
      auto i = static_cast<impl*>(p.get());

      i->receive(idp);
   }
}

#endif
