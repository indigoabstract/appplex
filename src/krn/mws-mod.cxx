#include "stdafx.hxx"

#include "appplex-conf.hxx"
#include "mws-mod.hxx"
#include "mws-impl.hxx"
#include "mws-mod-ctrl.hxx"
#include "min.hxx"
#include "res-ld/res-ld.hxx"
#include "util/util.hxx"
#include "mws/mws.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "fonts/mws-font-db.hxx"
#include "fonts/mws-font.hxx"
#include "input/input-ctrl.hxx"
#include "input/update-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "gfx-scene.hxx"
#include "gfx-state.hxx"
#include "mod-list.hxx"
#include <algorithm>
#include <cstdio>

#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

#include "gfx-quad-2d.hxx"
#include "gfx-rt.hxx"
#include "tst/test-ffmpeg/ffmpeg/venc-ffmpeg.hxx"
#include "pfm-gl.h"

#endif


mws_sp<mws_app> mws_app_inst();


bool mws_mod_preferences::requires_gfx()
{
   return mod_gfx_on;
}

class mws_app_storage_impl
{
public:
   mws_app_storage_impl()
   {
   }

   const mws_file_map& get_res_file_list() const
   {
      return res_files_map;
   }

   std::vector<uint8_t> load_as_byte_vect(mws_sp<mws_file> i_file) const
   {
      std::vector<uint8_t> res;

      if (i_file->io.open())
      {
         uint32_t size = static_cast<uint32_t>(i_file->length());

         res.resize(size);
         i_file->io.read(reinterpret_cast<std::byte*>(res.data()), size);
         i_file->io.close();
      }

      // copy elision
      return res;
   }

   std::vector<uint8_t> load_as_byte_vect(const mws_path& i_file_path) const
   {
      mws_sp<mws_file> fs = mws_file::get_inst(i_file_path);

      return load_as_byte_vect(fs);
   }

   mws_sp<std::vector<uint8_t>> load_as_sp_byte_vect(const mws_path& i_file_path) const
   {
      mws_sp<std::vector<uint8_t>> res;
      mws_sp<mws_file> fs = mws_file::get_inst(i_file_path);

      if (fs->io.open())
      {
         uint32_t size = static_cast<uint32_t>(fs->length());

         res = mws_sp<std::vector<uint8_t>>(new std::vector<uint8_t>(size));
         fs->io.read(reinterpret_cast<std::byte*>(res->data()), size);
         fs->io.close();
      }

      return res;
   }

   std::string load_as_string(mws_sp<mws_file> i_file) const
   {
      std::string text;

      if (i_file->io.open("rt"))
      {
         uint32_t size = static_cast<uint32_t>(i_file->length());
         std::vector<std::byte> res(size);
         int text_size = i_file->io.read(res.data(), size);

         i_file->io.close();
         text = std::string(reinterpret_cast<const char*>(res.data()), text_size);
      }

      // copy elision
      return text;
   }

   std::string load_as_string(const mws_path& i_file_path) const
   {
      mws_sp<mws_file> fs = mws_file::get_inst(i_file_path);

      return load_as_string(fs);
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
         auto crt_date_str = mws::time::get_current_date();
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
         static void read_pixels_helper(bool pbo_supported, mws_sp<gfx_tex> i_tex, gfx_uint pbo_id, gfx_uint pbo_next_id, std::vector<uint8_t>& i_data_dst)
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
      //gfx::i()->rt.get_render_target_pixels<uint8_t>(rt_y, pixels_y_tex);
      mws_report_gfx_errs();
      helper::read_pixels_helper(pbo_supported, rt_y_tex, y_pbo_ids[pbo_index], y_pbo_ids[pbo_next_index], pixels_y_tex);
      mws_report_gfx_errs();

      gfx::i()->rt.set_current_render_target(rt_u);
      rt_u_quad->draw_out_of_sync(mws_cam);
      //gfx::i()->rt.get_render_target_pixels<uint8_t>(rt_u, pixels_u_tex);
      helper::read_pixels_helper(pbo_supported, rt_u_tex, u_pbo_ids[pbo_index], u_pbo_ids[pbo_next_index], pixels_u_tex);
      gfx::i()->rt.set_current_render_target();
      mws_report_gfx_errs();

      gfx::i()->rt.set_current_render_target(rt_v);
      rt_v_quad->draw_out_of_sync(mws_cam);
      //gfx::i()->rt.get_render_target_pixels<uint8_t>(rt_v, pixels_v_tex);
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
         c2.a = uint8_t(v * 255);
         recording_fnt->set_color(c2);
         mws_cam->drawText(recording_txt, px, py, recording_fnt);
         //mws_print("slider: %f %f\n", sv, v);
      }

      rec_txt_slider.update();
      frame_index++;

#endif
   }

   void start_recording_screen(const mws_path& i_file_path = "", const mws_video_params* i_params = nullptr)
   {
#if MOD_FFMPEG && MOD_TEST_FFMPEG && MOD_GFX

      mws_path file_path = i_file_path;
      int video_width = gfx::i()->rt.get_screen_width();
      int video_height = gfx::i()->rt.get_screen_height();

      if (venc && venc->is_encoding())
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


      if (file_path.is_empty())
      {
         file_path = mws_to_str_fmt("app-%s-screen-capture.mp4", u.lock()->name().c_str());
      }

      const mws_video_params* video_params = (i_params) ? i_params : &default_video_params;

      venc->set_video_path(file_path);
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
   std::vector<uint8_t> pixels_y_tex;
   std::vector<uint8_t> pixels_u_tex;
   std::vector<uint8_t> pixels_v_tex;
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

   mws_file_map res_files_map;
   mws_wp<mws_mod> u;
};


void mws_mod_ctrl::set_current_mod(mws_sp<mws_mod> i_mod)
{
   if (i_mod)
   {
      if (i_mod != crt_mod.lock())
      {
         auto mod_pref = i_mod->get_preferences();

         if (!crt_mod.expired())
         {
            crt_mod.lock()->base_unload();
         }

         crt_mod = i_mod;
         mws_app_inst()->reconfigure_directories(i_mod);
         mws_log::set_enabled(mod_pref->log_enabled());

         // reload resources
         i_mod->storage.p->res_files_map = mws_app_inst()->list_internal_directory();
      }
   }
   else
   {
      mws_signal_error("warning: tried to make current a null mws_mod");
   }
}


mws_app_storage::mws_app_storage()
{
   p = std::make_unique<mws_app_storage_impl>();
}

const mws_file_map& mws_app_storage::get_res_file_list() const
{
   return p->get_res_file_list();
}

std::vector<uint8_t> mws_app_storage::load_as_byte_vect(mws_sp<mws_file> i_file) const
{
   return p->load_as_byte_vect(i_file);
}

std::vector<uint8_t> mws_app_storage::load_as_byte_vect(const mws_path& i_file_path) const
{
   return p->load_as_byte_vect(i_file_path);
}

mws_sp<std::vector<uint8_t>> mws_app_storage::load_as_sp_byte_vect(const mws_path& i_file_path) const
{
   return p->load_as_sp_byte_vect(i_file_path);
}

std::string mws_app_storage::load_as_string(mws_sp<mws_file> i_file) const
{
   return p->load_as_string(i_file);
}

std::string mws_app_storage::load_as_string(const mws_path& i_file_path) const
{
   return p->load_as_string(i_file_path);
}

// writable/private/persistent files directory
const mws_path& mws_app_storage::prv_dir() const
{
   return mws_app_inst()->prv_dir();
}

// read-only/resource files directory
const mws_path& mws_app_storage::res_dir() const
{
   return mws_app_inst()->res_dir();
}

// temporary files directory
const mws_path& mws_app_storage::tmp_dir() const
{
   return mws_app_inst()->tmp_dir();
}

void mws_app_storage::save_screenshot(const mws_path& i_file_path) const
{
#if MOD_GFX && MOD_PNG
   if (!p->u.lock()->is_gfx_mod())
   {
      return;
   }

   mws_sp<mws_file> screenshot_file;

   if (i_file_path.is_empty())
   {
      std::string file_root = mws_to_str_fmt("%s-", p->u.lock()->name().c_str());
      std::string img_ext = ".png";
      std::string zeroes[] =
      {
         "00", "0"
      };
      mws_path dir = tmp_dir() / "screens";
      int screenshot_idx = 0;

      // if dir doesn't exist, create it
      if (!dir.exists())
      {
         bool success = dir.make_dir();

         if (!success)
         {
            mws_println("cannot create dir [ %s ]", dir.string().c_str());
            return;
         }
      }

      // find the first available file name.
      do
      {
         std::string idx_nr = "";
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
         screenshot_file = mws_file::get_inst(dir / file_name);
         screenshot_idx++;
      } while (screenshot_file->exists());
   }
   else
   {
      screenshot_file = mws_file::get_inst(i_file_path);
   }

   {
      mws_print("saving screenshot to [ %s ] ... ", screenshot_file->string_path().c_str());
      int w = gfx::i()->rt.get_screen_width();
      int h = gfx::i()->rt.get_screen_height();
      mws_sp<std::vector<uint32_t>> pixels = gfx::i()->rt.get_render_target_pixels<uint32_t>();
      res_ld::inst()->save_image(screenshot_file, w, h, (uint8_t*)pixels->data(), res_ld::e_vertical_flip);
      mws_println("done.");
   }
#endif
}


void mws_app_storage::start_recording_screen(const mws_path& i_file_path, const mws_video_params* i_params)
{
   p->start_recording_screen(i_file_path, i_params);
}

void mws_app_storage::stop_recording_screen() { p->stop_recording_screen(); }
bool mws_app_storage::is_recording_screen() { return p->is_recording_screen(); }
void mws_app_storage::toggle_screen_recording() { p->toggle_screen_recording(); }


int mws_mod::mod_count = 0;

mws_mod::mws_mod(const char* i_include_guard)
{
   init_val = false;
   set_internal_name_from_include_guard(i_include_guard);
   mod_count++;
   prefs = std::make_shared<mws_mod_preferences>();
   game_time = 0;
}

mws_mod::~mws_mod() {}
mws_mod::mod_type mws_mod::get_mod_type() { return e_mod_base; }
int mws_mod::get_width() { return mws::screen::get_width(); }
int mws_mod::get_height() { return mws::screen::get_height(); }
const std::string& mws_mod::name() const { return name_v; }
void mws_mod::name(const std::string& i_name) { name_v = i_name; }
const std::string& mws_mod::external_name() const { return (external_name_v.empty()) ? name() : external_name_v; }
void mws_mod::external_name(const std::string& i_name) { external_name_v = i_name; }
const std::string& mws_mod::description() const { return (description_v.empty()) ? external_name() : description_v; }
void mws_mod::description(const std::string& i_description) { description_v = i_description; }
const mws_path& mws_mod::get_proj_rel_path() { return proj_rel_path; }
void mws_mod::set_proj_rel_path(const mws_path& i_path) { proj_rel_path = i_path; }
void mws_mod::set_app_exit_on_next_run(bool i_app_exit_on_next_run) { mws_mod_ctrl::inst()->set_app_exit_on_next_run(i_app_exit_on_next_run); }
bool mws_mod::gfx_available() { return mws::screen::is_gfx_available(); }
mws_sp<mws_mod> mws_mod::get_smtp_instance() { return shared_from_this(); }

void mws_mod::set_internal_name_from_include_guard(const char* i_include_guard)
{
   std::string name_t(i_include_guard);
   int idx = name_t.find('_');

   if (idx <= 0)
   {
      mws_throw mws_exception("invalid format for the include guard");
   }

   std::string internal_name = name_t.substr(idx + 1, std::string::npos);

   std::transform(internal_name.begin(), internal_name.end(), internal_name.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });
   // replace all '_' with '-'
   std::replace(internal_name.begin(), internal_name.end(), '_', '-');
   name(internal_name);
}

bool mws_mod::update()
{
   if (mod_gfx_on)
   {
      bool force_rebind = false;
#if defined MWS_PFM_IOS
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
   uint32_t now = mws::time::get_time_millis();
   uint32_t dt = now - last_frame_time;

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
         if (i_dp->is_type(mws_key_evt::key_evt_type))
         {
            mws_sp<mws_key_evt> ke = mws_key_evt::as_key_evt(i_dp);

            if (ke->get_type() != mws_key_evt::ke_released)
            {
               if (ke->get_type() != mws_key_evt::ke_repeated)
               {
                  bool key_handled = handle_function_key(ke->get_key());

                  if (key_handled)
                  {
                     process(ke);
                  }
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

bool mws_mod::handle_function_key(mws_key_types i_key)
{
   bool key_handled = true;

   switch (i_key)
   {
   case mws_key_f1:
      mws_mod_ctrl::inst()->pause();
      break;

   case mws_key_f2:
      mws_mod_ctrl::inst()->resume();
      break;

   case mws_key_f3:
      mws::screen::flip_screen();
      break;

   case mws_key_f4:
      storage.toggle_screen_recording();
      break;

   case mws_key_f5:
      storage.save_screenshot();
      break;

   case mws_key_f6:
      mws_mod_ctrl::inst()->set_app_exit_on_next_run(true);
      break;

   case mws_key_f11:
      mws::screen::set_full_screen_mode(!mws::screen::is_full_screen_mode());
      break;

   default:
      key_handled = false;
   }

   return key_handled;
}

void mws_mod::config_font_db_size()
{
#if MOD_VECTOR_FONTS

   uint32_t pow_of_two = get_preferences()->get_font_db_pow_of_two_size();
   mws_font_db::nwi_inex(pow_of_two);

#endif
}

void mws_mod::base_init()
{
#if MOD_GFX

   if (is_gfx_mod())
   {
      if (mod_input_on)
      {
         update_ctrl_inst = updatectrl::nwi();
         touch_ctrl_inst = mws_touch_ctrl::nwi();
         key_ctrl_inst = mws_key_ctrl::nwi();
      }

      gfx_scene_inst = mws_sp<gfx_scene>(new gfx_scene(get_smtp_instance()));
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

void mws_mod::enq_op_on_next_frame_start(const std::function<void()>& i_op)
{
   operation_mutex.lock();
   operation_list.push_back(i_op);
   operation_mutex.unlock();
}

void mws_mod::enq_op_on_crt_frame_end(const std::function<void()>& i_op)
{
   operation_mutex.lock();
   end_of_frame_op_list.push_back(i_op);
   operation_mutex.unlock();
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

   mws_font_db::inst()->on_frame_start();

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

   if (!end_of_frame_op_list.empty())
   {
      for (const auto& function : end_of_frame_op_list)
      {
         function();
      }

      end_of_frame_op_list.clear();
   }

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
   last_frame_time = mws::time::get_time_millis();

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

   if (prefs->show_fps() && fps > 0 && !storage.is_recording_screen())
   {
      mws_sp<mws_font> font = mws_cam->get_font();

      if (font)
      {
         float ups = 1000.f / update_ctrl_inst->getTimeStepDuration();
         std::string f = mws_to_str_fmt("uc %d u %02.1f f %02.1f", update_count, ups, fps);
         glm::vec2 txt_dim = font->get_text_dim(f);

         mws_cam->drawText(f, get_width() - txt_dim.x, 0.f);
      }
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

mws_sp<mws_mod> mws_mod_list::mod_by_name(const std::string& i_name)
{
   int size = ulist.size();

   for (int i = 0; i < size; i++)
   {
      mws_sp<mws_mod> u = ulist[i];

      if (u->name().compare(i_name) == 0)
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
      if (!i_dp->is_processed() && i_dp->is_type(mws_ptr_evt::ptr_evt_type))
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
         uint32_t idx = std::find(ul->ulist.begin(), ul->ulist.end(), u) - ul->ulist.begin();

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
         return get_mod_list()->ulist[idx]->name();
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
   mws_sp<mws_page> pg = mws_page::nwi(mws_root);
   mws_sp<mws_list> l = mws_list::nwi();

   ulmodel = lm;
   l->set_model(lm);
   pg->attach(l);
   mws_cam->clear_color = true;

#endif
}


mws_sp<mws_mod_list> mws_mod_setup::get_mod_list()
{
   return ul.lock();
}

void mws_mod_setup::add_mod(mws_sp<mws_mod> i_mod, const mws_path& i_mod_path, bool i_set_current)
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
