#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "mws-mod.hxx"
#include "mws-mod-ctrl.hxx"
#include "min.hxx"
#include "res-ld/res-ld.hxx"
#include "util/util.hxx"
#include "mws/mws.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "mws/mws-font.hxx"
#include "input/input-ctrl.hxx"
#include "input/update-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "gfx-scene.hxx"
#include "gfx-state.hxx"
#include "mws/font-db.hxx"
#include "mod-list.hxx"
#include <algorithm>
#include <cstdio>

#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "tests/test-ffmpeg/ffmpeg/venc-ffmpeg.hxx"
#include "pfm-gl.h"

#endif

using std::string;
using std::vector;


bool mws_mod_preferences::requires_gfx()
{
   return mod_gfx_on;
}

class mws_mod::app_storage_impl
{
public:
   app_storage_impl()
   {
   }

   void setup_video_encoding(int video_width, int video_height)
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      if (!venc)
      {
         venc = std::make_shared<venc_ffmpeg>();
         // prefer using a variable bit rate
         default_video_params.bit_rate = 0;// 2200 * 1000;
         default_video_params.width = 0;
         default_video_params.height = 0;
         // frames per second
         default_video_params.time_base_numerator = 2;
         default_video_params.time_base_denominator = 60;
         default_video_params.ticks_per_frame = 2;
         // emit one intra frame every ten frames
         default_video_params.gop_size = 10;
         default_video_params.max_b_frames = 1;
         default_video_params.pix_fmt = AV_PIX_FMT_YUV420P;
         default_video_params.codec_id = AV_CODEC_ID_H264;
         default_video_params.preset = "ultrafast";
         default_video_params.tune = "film";
         default_video_params.crf = 0;

         recording_fnt = mws_font::nwi(20.f);
         recording_fnt->set_color(gfx_color::colors::red);
         recording_txt = "[ recording ]";
         recording_txt_dim = recording_fnt->get_text_dim(recording_txt);
         date_fnt = mws_font::nwi(20.f);
         date_fnt->set_color(gfx_color::colors::cyan);
         pbo_supported = mws_is_gl_extension_supported("GL_ARB_pixel_buffer_object") != 0;
         y_pbo_ids = { 0, 0 };
         u_pbo_ids = { 0, 0 };
         v_pbo_ids = { 0, 0 };
      }

      default_video_params.width = gfx::i()->rt.get_screen_width();
      default_video_params.height = gfx::i()->rt.get_screen_height();

      if (!scr_mirror_tex || scr_mirror_tex->get_width() != video_width || scr_mirror_tex->get_height() != video_height)
      {
         {
            gfx_tex_params prm;

            prm.set_format_id("RGBA8");
            prm.set_rt_params();
            scr_mirror_tex = gfx::i()->tex.nwi(gfx_tex::gen_id(), gfx::i()->rt.get_screen_width(), gfx::i()->rt.get_screen_height(), &prm);
         }

         gfx_tex_params prm;

         prm.set_format_id("R8");
         prm.set_rt_params();

         // y rt
         {
            int rt_y_width = video_width;
            int rt_y_height = video_height;

            pixels_y_tex.resize(rt_y_width * rt_y_height);
            rt_y_tex = gfx::i()->tex.nwi("y-" + gfx_tex::gen_id(), rt_y_width, rt_y_height, &prm);
            rt_y = gfx::i()->rt.new_rt();
            rt_y->set_color_attachment(rt_y_tex);
            rt_y_quad = gfx_quad_2d::nwi();

            {
               auto& msh = *rt_y_quad;

               msh.set_dimensions(2, 2);
               msh.set_v_flip(true);
               msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
               msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-y-420.fsh";
               msh[MP_CULL_BACK] = false;
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
            }
         }

         // u rt
         {
            int rt_u_width = video_width / 2;
            int rt_u_height = video_height / 2;

            pixels_u_tex.resize(rt_u_width * rt_u_height);
            rt_u_tex = gfx::i()->tex.nwi("u-" + gfx_tex::gen_id(), rt_u_width, rt_u_height, &prm);
            rt_u = gfx::i()->rt.new_rt();
            rt_u->set_color_attachment(rt_u_tex);
            rt_u_quad = gfx_quad_2d::nwi();

            {
               auto& msh = *rt_u_quad;

               msh.set_dimensions(2, 2);
               msh.set_v_flip(true);
               msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
               msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-u-420.fsh";
               msh[MP_CULL_BACK] = false;
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
            }
         }

         // v rt
         {
            int rt_v_width = video_width / 2;
            int rt_v_height = video_height / 2;

            pixels_v_tex.resize(rt_v_width * rt_v_height);
            rt_v_tex = gfx::i()->tex.nwi("v-" + gfx_tex::gen_id(), rt_v_width, rt_v_height, &prm);
            rt_v = gfx::i()->rt.new_rt();
            rt_v->set_color_attachment(rt_v_tex);
            rt_v_quad = gfx_quad_2d::nwi();

            {
               auto& msh = *rt_v_quad;

               msh.set_dimensions(2, 2);
               msh.set_v_flip(true);
               msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
               msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-v-420.fsh";
               msh[MP_CULL_BACK] = false;
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
            }
         }

         if (pbo_supported)
         {
            std::vector<size_t> data_sizes = { pixels_y_tex.size(), pixels_u_tex.size(), pixels_v_tex.size() };
            std::vector<std::vector<gfx_uint>* > pbos = { &y_pbo_ids, &u_pbo_ids, &v_pbo_ids };

            for (size_t k = 0; k < pbos.size(); k++)
            {
               std::vector<gfx_uint>* p = pbos[k];
               int size = p->size();
               auto data_size = data_sizes[k];

               glGenBuffers(size, p->data());

               for (int l = 0; l < size; l++)
               {
                  glBindBuffer(GL_PIXEL_PACK_BUFFER, (*p)[l]);
                  glBufferData(GL_PIXEL_PACK_BUFFER, data_size, 0, GL_STREAM_READ);
               }
            }

            glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
         }
      }

#endif
   }

   void update_video_encoder()
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX && MOD_MWS

      mws_report_gfx_errs();

      auto mws_mod = u.lock();
      auto mws_cam = u.lock()->mws_cam;
      int width = mws_mod->get_width();
      int height = mws_mod->get_height();

      // show date of recording
      {
         auto crt_date_str = mws_util::time::get_current_date();
         auto txt_dim = date_fnt->get_text_dim(crt_date_str);
         float px = width - txt_dim.x;
         float py = height - txt_dim.y;

         mws_cam->drawText(crt_date_str, px, py, date_fnt);
      }

      // blit screen to a texture
      scr_mirror_tex->set_active(0);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

      struct helper
      {
         static void read_pixels_helper(bool pbo_supported, mws_sp<gfx_tex> i_tex, gfx_uint pbo_id, gfx_uint pbo_next_id, std::vector<uint8>& i_data_dst)
         {
            const gfx_tex_params& tex_prm = i_tex->get_params();

            glReadBuffer(GL_COLOR_ATTACHMENT0);

            // with PBO
            if (pbo_supported)
            {
               mws_report_gfx_errs();
               // copy pixels from framebuffer to PBO and use offset instead of ponter.
               // OpenGL should perform async DMA transfer, so glReadPixels() will return immediately.
               glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id);
               mws_report_gfx_errs();
               glPixelStorei(GL_PACK_ALIGNMENT, tex_prm.get_bpp());
               mws_report_gfx_errs();
               glReadPixels(0, 0, i_tex->get_width(), i_tex->get_height(), tex_prm.get_format(), tex_prm.get_type(), 0);
               mws_report_gfx_errs();

               // map the PBO containing the framebuffer pixels before processing it
               glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_next_id);
               mws_report_gfx_errs();

               GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

               mws_report_gfx_errs();
               if (src)
               {
                  std::memcpy(i_data_dst.data(), src, i_data_dst.size());
                  // release pointer to the mapped buffer
                  glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
               }
               mws_report_gfx_errs();

               glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
               mws_report_gfx_errs();
            }
            // without PBO
            else
            {
               glPixelStorei(GL_PACK_ALIGNMENT, tex_prm.get_bpp());
               glReadPixels(0, 0, i_tex->get_width(), i_tex->get_height(), tex_prm.get_format(), tex_prm.get_type(), i_data_dst.data());
            }
            mws_report_gfx_errs();
         }
      };

      // increment current index first then get the next index
      // pbo_index is used to read pixels from a framebuffer to a PBO
      pbo_index = (pbo_index + 1) % 2;
      // pbo_next_index is used to process pixels in the other PBO
      int pbo_next_index = (pbo_index + 1) % 2;

      mws_report_gfx_errs();
      gfx::i()->rt.set_current_render_target(rt_y);
      rt_y_quad->draw_out_of_sync(mws_cam);
      //gfx::i()->rt.get_render_target_pixels<uint8>(rt_y, pixels_y_tex);
      mws_report_gfx_errs();
      helper::read_pixels_helper(pbo_supported, rt_y_tex, y_pbo_ids[pbo_index], y_pbo_ids[pbo_next_index], pixels_y_tex);
      mws_report_gfx_errs();

      gfx::i()->rt.set_current_render_target(rt_u);
      rt_u_quad->draw_out_of_sync(mws_cam);
      //gfx::i()->rt.get_render_target_pixels<uint8>(rt_u, pixels_u_tex);
      helper::read_pixels_helper(pbo_supported, rt_u_tex, u_pbo_ids[pbo_index], u_pbo_ids[pbo_next_index], pixels_u_tex);
      gfx::i()->rt.set_current_render_target();
      mws_report_gfx_errs();

      gfx::i()->rt.set_current_render_target(rt_v);
      rt_v_quad->draw_out_of_sync(mws_cam);
      //gfx::i()->rt.get_render_target_pixels<uint8>(rt_v, pixels_v_tex);
      helper::read_pixels_helper(pbo_supported, rt_v_tex, v_pbo_ids[pbo_index], v_pbo_ids[pbo_next_index], pixels_v_tex);
      gfx::i()->rt.set_current_render_target();
      mws_report_gfx_errs();

      // skip this on the first frame as the frame data isn't ready yet
      // also skip on the second frame to avoid capturing the fps text (it's still in the backbuffer)
      if (frame_index > 1)
      {
         venc->encode_frame_m0_yuv420(pixels_y_tex.data(), pixels_u_tex.data(), pixels_v_tex.data());
      }
      //gfx_util::draw_tex(mws_cam, rt_u_tex, 700.f, 10.f, rt_u_tex->get_width() / 2, rt_u_tex->get_height() / 2);
      //gfx_util::draw_tex(mws_cam, rt_v_tex, 700.f, 280.f, rt_v_tex->get_width() / 2, rt_v_tex->get_height() / 2);
      //gfx_util::draw_tex(mws_cam, rt_y_tex, 10.f, 10.f, rt_y_tex->get_width() / 2, rt_y_tex->get_height() / 2);

      // show recording text
      {
         float off = 10.f;
         float px = off;
         float py = height - recording_txt_dim.y - off;

         const gfx_color& c = recording_fnt->get_color();
         gfx_color c2 = c;
         float sv = rec_txt_slider.get_value();
         float v = sv;

         v = 1.f - (1.f - v) * (1.f - v);
         c2.a = uint8(v * 255);
         recording_fnt->set_color(c2);
         mws_cam->drawText(recording_txt, px, py, recording_fnt);
         //mws_print("slider: %f %f\n", sv, v);
      }

      rec_txt_slider.update();
      frame_index++;

#endif
   }

   void start_recording_screen(std::string i_filename = "", const mws_video_params * i_params = nullptr)
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      int video_width = gfx::i()->rt.get_screen_width();
      int video_height = gfx::i()->rt.get_screen_height();

      if (venc&& venc->is_encoding())
      {
         venc->stop_encoding();
      }

      if (i_params)
      {
         video_width = i_params->width;
         video_height = i_params->height;
      }

      setup_video_encoding(video_width, video_height);
      rec_txt_slider.start(0.95f);
      pbo_index = 0;
      frame_index = 0;


      if (i_filename.empty())
      {
         i_filename = mws_to_str_fmt("app-%s-screen-capture.mp4", u.lock()->get_name().c_str());
      }

      const mws_video_params* video_params = (i_params) ? i_params : &default_video_params;

      venc->set_video_path(i_filename);
      venc->start_encoding(*video_params, mws_vid_enc_method::e_enc_m0);

#else

      mws_print("you need to enable MOD_FFMPEG and MOD_TEST_FFMPEG to record screen video\n");

#endif
   }

   void stop_recording_screen()
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      if (venc && venc->is_encoding())
      {
         venc->stop_encoding();
      }
      else
      {
         mws_print("not recording screen video\n");
      }

#else

      mws_print("you need to enable MOD_FFMPEG and MOD_TEST_FFMPEG to record screen video\n");

#endif
   }

   bool is_recording_screen()
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      return venc && venc->is_encoding();

#endif

      return false;
   }

   void toggle_screen_recording()
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      if (venc)
      {
         if (venc->is_encoding())
         {
            stop_recording_screen();
         }
         else
         {
            start_recording_screen();
         }
      }
      else
      {
         start_recording_screen();
      }

#else

      mws_print("you need to enable MOD_FFMPEG and MOD_TEST_FFMPEG to record screen video\n");

#endif
   }

#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

   // data for converting rgb to yuv420
   mws_sp<gfx_tex> scr_mirror_tex;
   mws_sp<gfx_rt> rt_y;
   mws_sp<gfx_tex> rt_y_tex;
   mws_sp<gfx_quad_2d> rt_y_quad;
   mws_sp<gfx_rt> rt_u;
   mws_sp<gfx_tex> rt_u_tex;
   mws_sp<gfx_quad_2d> rt_u_quad;
   mws_sp<gfx_rt> rt_v;
   mws_sp<gfx_tex> rt_v_tex;
   mws_sp<gfx_quad_2d> rt_v_quad;
   std::vector<uint8> pixels_y_tex;
   std::vector<uint8> pixels_u_tex;
   std::vector<uint8> pixels_v_tex;
   bool pbo_supported;
   std::vector<gfx_uint> y_pbo_ids;
   std::vector<gfx_uint> u_pbo_ids;
   std::vector<gfx_uint> v_pbo_ids;
   int frame_index;
   int pbo_index;

   mws_sp<venc_ffmpeg> venc;
   mws_video_params default_video_params;
   mws_sp<mws_font> date_fnt;
   mws_sp<mws_font> recording_fnt;
   std::string recording_txt;
   glm::vec2 recording_txt_dim;
   ping_pong_time_slider<float> rec_txt_slider;

#endif

   mws_wp<mws_mod> u;
};


mws_mod::app_storage::app_storage()
{
   p = std::make_unique<app_storage_impl>();
}

mws_sp<std::vector<uint8> > mws_mod::app_storage::load_mod_byte_vect(string name)
{
   return pfm::filesystem::load_mod_byte_vect(p->u.lock(), name);
}

//shared_array<uint8> mws_mod::app_storage::load_mod_byte_array(string name, int& size)
//{
//	return pfm::storage::load_mod_byte_array(u.lock(), name, size);
//}

bool mws_mod::app_storage::store_mod_byte_array(string name, const uint8 * resPtr, int size)
{
   return pfm::filesystem::store_mod_byte_array(p->u.lock(), name, resPtr, size);
}

bool mws_mod::app_storage::store_mod_byte_vect(string name, const std::vector<uint8> & resPtr)
{
   return pfm::filesystem::store_mod_byte_vect(p->u.lock(), name, resPtr);
}

mws_sp<pfm_file> mws_mod::app_storage::random_access(std::string name)
{
   return pfm::filesystem::random_access(p->u.lock(), name);
}

void mws_mod::app_storage::save_screenshot(std::string i_filename)
{
#if MOD_GFX && MOD_PNG
   if (!p->u.lock()->is_gfx_mod())
   {
      return;
   }

   mws_sp<pfm_file> screenshot_file;

   if (i_filename.empty())
   {
      string file_root = mws_to_str_fmt("%s-", p->u.lock()->get_name().c_str());
      string img_ext = ".png";
      string zeroes[] =
      {
         "00", "0"
      };
      std::string dir_name = pfm::filesystem::get_tmp_path("screens");
      auto dir = pfm_file::get_inst(dir_name);
      int screenshot_idx = 0;

      // if dir doesn't exist, create it
      if (!dir->exists())
      {
         bool success = dir->make_dir();

         if (!success)
         {
            mws_println("cannot create dir [ %s ]", dir_name.c_str());
            return;
         }
      }

      // find the first available file name.
      do
      {
         string idx_nr = "";
         int digits = 0;
         int ssi = screenshot_idx;

         // calc. number of digits in a number.
         while ((ssi /= 10) > 0)
         {
            digits++;
         }

         // assign a zero prefix.
         if (digits < 2)
         {
            idx_nr = mws_to_str_fmt("%s%s%d", file_root.c_str(), zeroes[digits].c_str(), screenshot_idx);
         }
         else
         {
            idx_nr = mws_to_str_fmt("%s%d", file_root.c_str(), screenshot_idx);
         }

         std::string file_name = mws_to_str_fmt("%s%s", idx_nr.c_str(), img_ext.c_str());
         screenshot_file = pfm_file::get_inst(dir_name + "/" + file_name);
         screenshot_idx++;
      } while (screenshot_file->exists());
   }
   else
   {
      screenshot_file = pfm_file::get_inst(i_filename);
   }

   {
      mws_print("saving screenshot to [ %s ] ... ", screenshot_file->get_full_path().c_str());
      int w = gfx::i()->rt.get_screen_width();
      int h = gfx::i()->rt.get_screen_height();
      mws_sp<std::vector<uint32>> pixels = gfx::i()->rt.get_render_target_pixels<uint32>();
      res_ld::inst()->save_image(screenshot_file, w, h, (uint8*)pixels->data(), res_ld::e_vertical_flip);
      mws_println("done.");
   }
#endif
}


void mws_mod::app_storage::start_recording_screen(std::string i_filename, const mws_video_params * i_params)
{
   p->start_recording_screen(i_filename, i_params);
}

void mws_mod::app_storage::stop_recording_screen()
{
   p->stop_recording_screen();
}

bool mws_mod::app_storage::is_recording_screen()
{
   return p->is_recording_screen();
}

void mws_mod::app_storage::toggle_screen_recording()
{
   p->toggle_screen_recording();
}


int mws_mod::mod_count = 0;

mws_mod::mws_mod(const char* i_include_guard)
{
   init_val = false;
   set_internal_name_from_include_guard(i_include_guard);
   mod_count++;
   prefs = std::make_shared<mws_mod_preferences>();
   game_time = 0;
}

mws_mod::~mws_mod()
{
}

mws_mod::mod_type mws_mod::get_mod_type()
{
   return e_mod_base;
}

int mws_mod::get_width()
{
   return pfm::screen::get_width();
}

int mws_mod::get_height()
{
   return pfm::screen::get_height();
}

const string& mws_mod::get_name()
{
   return name;
}

void mws_mod::set_name(string i_name)
{
   name = i_name;
}

const std::string& mws_mod::get_external_name()
{
   if (external_name.empty())
   {
      return get_name();
   }

   return external_name;
}

void mws_mod::set_external_name(std::string i_name)
{
   external_name = i_name;
}

const std::string& mws_mod::get_proj_rel_path()
{
   return proj_rel_path;
}

void mws_mod::set_proj_rel_path(std::string ipath)
{
   proj_rel_path = ipath;
}

void mws_mod::set_app_exit_on_next_run(bool iapp_exit_on_next_run)
{
   mws_mod_ctrl::inst()->set_app_exit_on_next_run(iapp_exit_on_next_run);
}

bool mws_mod::gfx_available()
{
   return pfm::screen::is_gfx_available();
}

mws_sp<mws_mod> mws_mod::get_smtp_instance()
{
   return shared_from_this();
}

void mws_mod::set_internal_name_from_include_guard(const char* i_include_guard)
{
   std::string name(i_include_guard);
   int idx = name.find('_');

   if (idx <= 0)
   {
      mws_throw mws_exception("invalid format for the include guard");
   }

   std::string internal_name = name.substr(idx + 1, std::string::npos);

   std::transform(internal_name.begin(), internal_name.end(), internal_name.begin(), ::tolower);
   // replace all '_' with '-'
   std::replace(internal_name.begin(), internal_name.end(), '_', '-');
   set_name(internal_name);
}

bool mws_mod::update()
{
   if (mod_gfx_on)
   {
      bool force_rebind = false;
#if defined PLATFORM_IOS
      force_rebind = true;
#endif
      int update_count = 1;//update_ctrl_inst->update();

      mws_report_gfx_errs();

      if (mod_input_on)
      {
         //for (int k = 0; k < updateCount; k++)
         {
            touch_ctrl_inst->update();
            key_ctrl_inst->update();
            game_time += update_ctrl_inst->getTimeStepDuration();
         }
      }

      gfx_scene_inst->update();

      if (mod_mws_on)
      {
         mws_root->update_state();
      }


      post_update();
      gfx::i()->rt.set_current_render_target(nullptr, force_rebind);
      mws_report_gfx_errs();
      gfx_scene_inst->draw();
      update_view(update_count);
      gfx_scene_inst->post_draw();
      post_update_view();
      mws_report_gfx_errs();
   }


#ifdef MWS_DEBUG_BUILD

   // update fps
   frame_count++;
   uint32 now = pfm::time::get_time_millis();
   uint32 dt = now - last_frame_time;

   if (dt >= 1000)
   {
      fps = frame_count * 1000.f / dt;
      last_frame_time = now;
      frame_count = 0;
   }

#endif

   return true;
}

void mws_mod::on_resize()
{
#if MOD_GFX

   if (is_gfx_mod() && gfx::i())
   {
      mws_sp<gfx_state> gfx_st = gfx::i()->get_gfx_state();
      int w = get_width();
      int h = get_height();

      decl_gfxpl(pl1)
      {
         {gl::VIEWPORT, 0, 0, w, h},
         {},
      };

      gfx_st->set_state(pl1);

      if (mws_root)
      {
         mws_root->on_resize();
      }
   }

#endif
}

void mws_mod::on_pause()
{
}

void mws_mod::on_resume()
{
}

void mws_mod::receive(mws_sp<mws_dp> i_dp)
{
   if (mod_input_on)
   {
      if (mod_mws_on)
      {
         send(mws_root, i_dp);
      }

      if (!i_dp->is_processed())
      {
         if (i_dp->is_type(mws_key_evt::KEYEVT_EVT_TYPE))
         {
            mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(i_dp);

            if (ke->get_type() != mws_key_evt::KE_RELEASED)
            {
               bool do_action = false;

               if (ke->get_type() != mws_key_evt::KE_REPEATED)
               {
                  do_action = true;

                  switch (ke->get_key())
                  {
                  case KEY_F1:
                     mws_mod_ctrl::inst()->pause();
                     break;

                  case KEY_F2:
                     mws_mod_ctrl::inst()->resume();
                     break;

                  case KEY_F3:
                     pfm::screen::flip_screen();
                     break;

                  case KEY_F4:
                     storage.toggle_screen_recording();
                     break;

                  case KEY_F5:
                     storage.save_screenshot();
                     break;

                  case KEY_F6:
                     mws_mod_ctrl::inst()->set_app_exit_on_next_run(true);
                     break;

                  case KEY_F11:
                     pfm::screen::set_full_screen_mode(!pfm::screen::is_full_screen_mode());
                     break;

                  default:
                     do_action = false;
                  }
               }

               if (do_action)
               {
                  process(ke);
               }
            }
         }
      }
   }
}

void mws_mod::process(mws_sp<mws_dp> i_dp)
{
   i_dp->process(get_smtp_instance());
}

void mws_mod::base_init()
{
#if MOD_GFX

   if (is_gfx_mod())
   {
      if (mod_input_on)
      {
         update_ctrl_inst = updatectrl::nwi();
         touch_ctrl_inst = touchctrl::nwi();
         key_ctrl_inst = key_ctrl::nwi();
      }

      gfx_scene_inst = mws_sp<gfx_scene>(new gfx_scene());
      gfx_scene_inst->init();
   }

#endif

   init();
   storage.p->u = get_smtp_instance();

#if MOD_GFX

   // getInst() doesn't work in the constructor
   if (is_gfx_mod())
   {
      if (mod_input_on)
      {
         touch_ctrl_inst->add_receiver(get_smtp_instance());
         key_ctrl_inst->add_receiver(get_smtp_instance());
      }

#if MOD_MWS

      {
         mws_cam = mws_camera::nwi();
         mws_cam->camera_id = "mws_cam";
         mws_cam->projection_type = gfx_camera::e_orthographic_proj;
         mws_cam->near_clip_distance = -100;
         mws_cam->far_clip_distance = 100;
         mws_cam->clear_color = false;
         mws_cam->clear_color_value = gfx_color::colors::black;
         mws_cam->clear_depth = true;
         gfx_scene_inst->attach(mws_cam);
      }

      {
         mws_root = mws_page_tab::nwi(get_smtp_instance());
         gfx_scene_inst->attach(mws_root);
         mws_root->init();
         init_mws();
         mws_root->init_subobj();
         post_init_mws();
      }

#endif
   }

#endif // MOD_GFX
}

/**
* Called on entering the mws_mod for the first time.
*/
void mws_mod::init()
{
}

/**
* Called before the mws_mod is destroyed.
*/
void mws_mod::on_destroy()
{
   if (is_init())
   {
      //mws_root->on_destroy();
   }
}

void mws_mod::init_mws() {}
void mws_mod::post_init_mws() {}

/**
* Called on entering the mws_mod.
*/
void mws_mod::load()
{
}

/**
* Called on leaving the mws_mod.
*/
void mws_mod::unload()
{
}

bool mws_mod::rsk_was_hit(int x0, int y0)
{
   int w = 32, h = 32;
   int radius = std::max(w, h) / 2;
   int cx = get_width() - w / 2 - 4;
   int cy = get_height() - h / 2 - 4;
   int dx = cx - x0;
   int dy = cy - y0;

   if ((int)sqrtf(float(dx* dx + dy * dy)) <= radius)
   {
      return true;
   }

   return false;
}

int mws_mod::schedule_operation(const std::function<void()> & ioperation)
{
   operation_mutex.lock();
   operation_list.push_back(ioperation);
   operation_mutex.unlock();

   return 0;
}

bool mws_mod::cancel_operation(int ioperation_id)
{
   return false;
}

bool mws_mod::back()
{
#ifndef SINGLE_MOD_BUILD
   mws_mod_list::up_one_level();

   return false;
#else

#if MOD_MWS
   return !mws_root->handle_back_evt();
#else
   return true;
#endif
#endif
}

bool mws_mod::is_gfx_mod()
{
   return get_preferences()->requires_gfx();
}

mws_sp<mws_mod_preferences> mws_mod::get_preferences()
{
   return prefs;
}

bool mws_mod::is_init()
{
   return init_val;
}

mws_sp<mws_sender> mws_mod::sender_inst()
{
   return get_smtp_instance();
}

void mws_mod::run_step()
{
#if MOD_VECTOR_FONTS

   font_db::inst()->on_frame_start();

#endif

   if (!operation_list.empty())
   {
      operation_mutex.lock();
      auto temp = operation_list;

      operation_list.clear();
      operation_mutex.unlock();

      for (const auto& function : temp)
      {
         function();
      }
   }

   update();

#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

   if (storage.is_recording_screen())
   {
      storage.p->update_video_encoder();
   }

#endif
}

void mws_mod::base_load()
{
   fps = 0;
   frame_count = 0;
   last_frame_time = pfm::time::get_time_millis();

   load();
   //update_ctrl_inst->started();
}

void mws_mod::base_unload()
{
   unload();
   //update_ctrl_inst->stopped();
}

void mws_mod::set_init(bool i_is_init)
{
   init_val = i_is_init;
}

void mws_mod::update_view(int update_count)
{
#if MOD_GFX && MOD_MWS

   mws_root->update_view(mws_cam);

#ifdef MWS_DEBUG_BUILD

   if (fps > 0 && !storage.is_recording_screen())
   {
      float ups = 1000.f / update_ctrl_inst->getTimeStepDuration();
      string f = mws_to_str_fmt("uc %d u %02.1f f %02.1f", update_count, ups, fps);
      glm::vec2 txt_dim = mws_cam->get_font()->get_text_dim(f);

      mws_cam->drawText(f, get_width() - txt_dim.x, 0.f);
   }

#endif // MWS_DEBUG_BUILD

#endif
}

void mws_mod::post_update_view() {}


int mws_mod_list::mod_list_count = 0;


mws_mod_list::mws_mod_list() : mws_mod(mws_to_str_fmt("mod_app_list_#%d", mws_mod_list::mod_list_count).c_str())
{
   mod_list_count++;
}

mws_sp<mws_mod_list> mws_mod_list::nwi()
{
   return mws_sp<mws_mod_list>(new mws_mod_list());
}

mws_mod::mod_type mws_mod_list::get_mod_type()
{
   return e_mod_list;
}

void mws_mod_list::add(mws_sp<mws_mod> i_mod)
{
   mws_assert(i_mod != nullptr);

   i_mod->parent = get_smtp_instance();
   ulist.push_back(i_mod);
   //ulmodel.lock()->notify_update();
}

mws_sp<mws_mod> mws_mod_list::mod_at(int i_index)
{
   return ulist[i_index];
}

mws_sp<mws_mod> mws_mod_list::mod_by_name(string iname)
{
   int size = ulist.size();

   for (int i = 0; i < size; i++)
   {
      mws_sp<mws_mod> u = ulist[i];

      if (u->get_name().compare(iname) == 0)
      {
         return u;
      }
   }

   return mws_sp<mws_mod>();
}

int mws_mod_list::get_mod_count()const
{
   return ulist.size();
}

void mws_mod_list::on_resize()
{
   if (mod_gfx_on)
   {
      if (ulmodel.lock())
      {
         auto u = ulist[ulmodel.lock()->get_selected_elem()];

         if (u && u->is_init())
         {
            u->on_resize();
         }
      }
   }
}

void mws_mod_list::receive(mws_sp<mws_dp> i_dp)
{
   if (mod_input_on)
   {
      if (!i_dp->is_processed() && i_dp->is_type(mws_ptr_evt::TOUCHSYM_EVT_TYPE))
      {
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);
      }

      if (!i_dp->is_processed())
      {
         mws_mod::receive(i_dp);
      }
   }
}

void mws_mod_list::forward()
{
   if (mod_gfx_on)
   {
      if (ulist.size() > 0)
      {
         mws_sp<mws_mod> u = ulist[ulmodel.lock()->get_selected_elem()];
         mws_mod_ctrl::inst()->set_next_mod(u);
      }
   }
}

void mws_mod_list::up_one_level()
{
#ifndef SINGLE_MOD_BUILD
   mws_sp<mws_mod> u = mws_mod_ctrl::inst()->get_current_mod();
   mws_sp<mws_mod> parent = u->parent.lock();

   if (parent != NULL)
   {
      if (parent->get_mod_type() == e_mod_list)
      {
         mws_sp<mws_mod_list> ul = static_pointer_cast<mws_mod_list>(parent);
         uint32 idx = std::find(ul->ulist.begin(), ul->ulist.end(), u) - ul->ulist.begin();

         if (idx < ul->ulist.size())
         {
            ul->ulmodel.lock()->set_selected_elem(idx);
         }
      }

      mws_mod_ctrl::inst()->set_next_mod(parent);
   }
#endif
}

void mws_mod_list::on_destroy()
{
   ulist.clear();
}

void mws_mod_list::init_mws()
{
#if MOD_MWS

   class lmodel : public mws_list_model
   {
   public:
      lmodel(mws_sp<mws_mod_list> i_ul) : ul(i_ul) {}

      int get_length()
      {
         return get_mod_list()->ulist.size();
      }

      std::string elem_at(int idx)
      {
         return get_mod_list()->ulist[idx]->get_name();
      }

      void on_elem_selected(int idx)
      {
         mws_sp<mws_mod> u = get_mod_list()->ulist[idx];

         //trx("item %1%") % elemAt(idx);
         mws_mod_ctrl::inst()->set_next_mod(u);
      }

   private:
      mws_sp<mws_mod_list> get_mod_list()
      {
         return ul.lock();
      }

      mws_wp<mws_mod_list> ul;
   };

   mws_sp<mws_mod_list> ul = static_pointer_cast<mws_mod_list>(get_smtp_instance());
   mws_sp<mws_list_model> lm((mws_list_model*)new lmodel(ul));
   mws_sp<mws_page> p = mws_page::nwi(mws_root);
   mws_sp<mws_list> l = mws_list::nwi();

   ulmodel = lm;
   l->set_model(lm);
   p->attach(l);
   mws_cam->clear_color = true;

#endif
}


mws_wp<mws_mod_list> mws_mod_setup::ul;
mws_wp<mws_mod> mws_mod_setup::next_crt_mod;


mws_sp<mws_mod_list> mws_mod_setup::get_mod_list()
{
   return ul.lock();
}

void mws_mod_setup::add_mod(mws_sp<mws_mod> i_mod, std::string i_mod_path, bool i_set_current)
{
   i_mod->set_proj_rel_path(i_mod_path);

   if (get_mod_list())
   {
      get_mod_list()->add(i_mod);

      if (i_set_current)
      {
         next_crt_mod = i_mod;
      }
   }
}
