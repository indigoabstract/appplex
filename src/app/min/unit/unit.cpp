#include "stdafx.h"

#include "unit.hpp"
#include "appplex-conf.hpp"
#include "com/ux/ux-camera.hpp"
#include "com/ux/ux.hpp"
#include "com/ux/ux-com.hpp"
#include "com/ux/ux-font.hpp"
#include "unit-ctrl.hpp"
#include "min.hpp"
#include "com/unit/input-ctrl.hpp"
#include "com/unit/update-ctrl.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "gfx-scene.hpp"
#include "gfx-state.hpp"
#include "media/res-ld/res-ld.hpp"
#include "com/ux/font-db.hpp"
#include <cstdio>

#ifdef MOD_BOOST

#include <boost/filesystem.hpp>
namespace bfs = ::boost::filesystem;

#endif // MOD_BOOST

#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

#include "gfx-quad-2d.hpp"
#include "gfx-rt.hpp"
#include "public/tests/test-ffmpeg/ffmpeg/venc-ffmpeg.hpp"
#include "pfmgl.h"

#endif

using std::string;
using std::vector;


class unit::app_storage_impl
{
public:
   app_storage_impl()
   {
   }

   void setup_video_encoding(int video_width, int video_height)
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

      if (!venc)
      {
         venc = std::make_shared<venc_ffmpeg>();
         default_video_params.bit_rate = 2200 * 1000;
         default_video_params.width = 0;
         default_video_params.height = 0;
         // frames per second
         default_video_params.time_base = { 2, 50 };
         default_video_params.ticks_per_frame = 2;
         // emit one intra frame every ten frames
         default_video_params.gop_size = 10;
         default_video_params.max_b_frames = 1;
         default_video_params.pix_fmt = AV_PIX_FMT_YUV420P;
         default_video_params.codec_id = AV_CODEC_ID_H264;

         recording_fnt = ux_font::new_inst(20.f);
         recording_fnt->set_color(gfx_color::colors::red);
         recording_txt = "[ recording ]";
         recording_txt_dim = recording_fnt->get_text_dim(recording_txt);
         date_fnt = ux_font::new_inst(20.f);
         date_fnt->set_color(gfx_color::colors::cyan);
         pbo_supported = is_gl_extension_supported("GL_ARB_pixel_buffer_object") != 0;
      }

      default_video_params.width = gfx::rt::get_screen_width();
      default_video_params.height = gfx::rt::get_screen_height();
      rec_txt_slider.start(0.95f);

      if (!scr_mirror_tex || scr_mirror_tex->get_width() != video_width || scr_mirror_tex->get_height() != video_height)
      {
         gfx_tex_params prm;

         prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
         prm.max_anisotropy = 0.f;
         prm.min_filter = gfx_tex_params::e_tf_linear;
         prm.mag_filter = gfx_tex_params::e_tf_linear;
         prm.gen_mipmaps = false;
         scr_mirror_tex = gfx::tex::new_tex_2d(gfx_tex::gen_id(), gfx::rt::get_screen_width(), gfx::rt::get_screen_height(), &prm);

         prm.min_filter = gfx_tex_params::e_tf_nearest;
         prm.mag_filter = gfx_tex_params::e_tf_nearest;
         pixels_y_tex.resize(video_width * video_height);
         pixels_uv_tex.resize(video_width * video_height);

         // y rt
         {
            int rt_y_width = video_width;
            int rt_y_height = video_height;

            rt_y_tex = gfx::tex::new_tex_2d("u_s2d_y_tex", rt_y_width, rt_y_height, "R8", &prm);
            rt_y = gfx::rt::new_rt();
            rt_y->set_color_attachment(rt_y_tex);
            rt_y_quad = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            {
               auto& msh = *rt_y_quad;

               msh.set_dimensions(1, 1);
               msh.set_scale((float)rt_y_width, (float)rt_y_height);
               msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
               msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-y-420.fsh";
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
            }
         }

         // uv rt
         {
            int rt_uv_width = video_width / 2;
            int rt_uv_height = video_height / 2;

            rt_uv_tex = gfx::tex::new_tex_2d("u_s2d_uv_tex", rt_uv_width, rt_uv_height, "RGBA8", &prm);
            rt_uv = gfx::rt::new_rt();
            rt_uv->set_color_attachment(rt_uv_tex);
            rt_uv_quad = shared_ptr<gfx_quad_2d>(new gfx_quad_2d());

            {
               auto& msh = *rt_uv_quad;

               msh.set_dimensions(1, 1);
               msh.set_scale((float)rt_uv_width, (float)rt_uv_height);
               msh[MP_SHADER_NAME][MP_VSH_NAME] = "conv-rgb-2-yuv-420.vsh";
               msh[MP_SHADER_NAME][MP_FSH_NAME] = "conv-rgb-2-uv-420.fsh";
               msh["u_s2d_tex"] = scr_mirror_tex->get_name();
            }
         }
      }

#endif
   }

   void update_video_encoder()
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

      gfx_util::check_gfx_error();

      auto unit = u.lock();
      auto ux_cam = u.lock()->ux_cam;
      int width = unit->get_width();
      int height = unit->get_height();

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
         ux_cam->drawText(recording_txt, px, py, recording_fnt);
         //vprint("slider: %f %f\n", sv, v);
      }

      // show date of recording
      {
         auto crt_date_str = mws_util::time::get_current_date();
         auto txt_dim = date_fnt->get_text_dim(crt_date_str);
         float px = width - txt_dim.x;
         float py = height - txt_dim.y;

         ux_cam->drawText(crt_date_str, px, py, date_fnt);
      }

      // blit screen to a texture
      scr_mirror_tex->set_active(0);
      glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);

      gfx::rt::set_current_render_target(rt_y);
      rt_y_quad->render_mesh(ux_cam);
      gfx::rt::get_render_target_pixels<uint8>(rt_y, pixels_y_tex);
      gfx_util::check_gfx_error();

      gfx::rt::set_current_render_target(rt_uv);
      rt_uv_quad->render_mesh(ux_cam);
      gfx::rt::get_render_target_pixels<uint32>(rt_uv, pixels_uv_tex);
      gfx::rt::set_current_render_target();
      gfx_util::check_gfx_error();

      venc->encode_yuv420_frame(pixels_y_tex.data(), pixels_uv_tex.data());
      rec_txt_slider.update();

#endif
   }

   void start_recording_screen(std::string i_filename = "", const video_params_ffmpeg* i_params = nullptr)
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

      int video_width = gfx::rt::get_screen_width();
      int video_height = gfx::rt::get_screen_height();

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


      if (i_filename.empty())
      {
         i_filename = trs("app-{}-screen-capture.mp4", u.lock()->get_name());
      }

      const video_params_ffmpeg* video_params = (i_params) ? i_params : &default_video_params;

      venc->start_encoding(i_filename.c_str(), *video_params);

#else

      vprint("you need to enable MOD_FFMPEG and UNIT_TEST_FFMPEG to record screen video\n");

#endif
   }

   void stop_recording_screen()
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

      if (venc && venc->is_encoding())
      {
         venc->stop_encoding();
      }
      else
      {
         vprint("not recording screen video\n");
      }

#else

      vprint("you need to enable MOD_FFMPEG and UNIT_TEST_FFMPEG to record screen video\n");

#endif
      }

   bool is_recording_screen()
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

      return venc && venc->is_encoding();

#endif

      return false;
   }

   void toggle_screen_recording()
   {
#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

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

      vprint("you need to enable MOD_FFMPEG and UNIT_TEST_FFMPEG to record screen video\n");

#endif
      }

#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

   // data for converting rgb to yuv420
   shared_ptr<gfx_tex> scr_mirror_tex;
   shared_ptr<gfx_rt> rt_y;
   shared_ptr<gfx_tex> rt_y_tex;
   shared_ptr<gfx_quad_2d> rt_y_quad;
   shared_ptr<gfx_rt> rt_uv;
   shared_ptr<gfx_tex> rt_uv_tex;
   shared_ptr<gfx_quad_2d> rt_uv_quad;
   bool pbo_supported;
   shared_ptr<venc_ffmpeg> venc;
   video_params_ffmpeg default_video_params;
   std::vector<uint8> pixels_y_tex;
   std::vector<uint32> pixels_uv_tex;
   shared_ptr<ux_font> date_fnt;
   shared_ptr<ux_font> recording_fnt;
   std::string recording_txt;
   glm::vec2 recording_txt_dim;
   ping_pong_time_slider rec_txt_slider;

#endif

   std::weak_ptr<unit> u;
   };


unit::app_storage::app_storage()
{
   p = std::make_unique<app_storage_impl>();
}

shared_ptr<std::vector<uint8> > unit::app_storage::load_unit_byte_vect(string name)
{
   return pfm::filesystem::load_unit_byte_vect(p->u.lock(), name);
}

//shared_array<uint8> unit::app_storage::load_unit_byte_array(string name, int& size)
//{
//	return pfm::storage::load_unit_byte_array(u.lock(), name, size);
//}

bool unit::app_storage::store_unit_byte_array(string name, const uint8* resPtr, int size)
{
   return pfm::filesystem::store_unit_byte_array(p->u.lock(), name, resPtr, size);
}

bool unit::app_storage::store_unit_byte_vect(string name, const std::vector<uint8>& resPtr)
{
   return pfm::filesystem::store_unit_byte_vect(p->u.lock(), name, resPtr);
}

shared_ptr<pfm_file> unit::app_storage::random_access(std::string name)
{
   return pfm::filesystem::random_access(p->u.lock(), name);
}

void unit::app_storage::save_screenshot(std::string ifilename)
{
#ifdef MOD_BOOST
   if (!p->u.lock()->is_gfx_unit())
   {
      return;
   }

   shared_ptr<std::vector<uint32> > pixels = gfx::rt::get_render_target_pixels<uint32>();
   shared_ptr<pfm_file> screenshot_file;

   if (ifilename.empty())
   {
      string file_root = trs("{}-", p->u.lock()->get_name());
      string img_ext = ".png";
      string zeroes[] =
      {
         "00", "0"
      };
      std::string dir_name = trs("f:\\data\\media\\work\\screens\\{}", p->u.lock()->get_name());
      //shared_ptr<pfm_file> dir = pfm_file::get_inst(dir_name);
      bfs::path dst_dir(dir_name);
      //bfs::path screenshot_file;
      int screenshot_idx = 0;

      if (!bfs::exists(dst_dir))
         // if dir doesn't exist, create it
      {
         bfs::create_directory(dst_dir);
      }

      do
         // find the first available file name.
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
            idx_nr = trs("{0}{1}{2}", file_root, zeroes[digits], screenshot_idx);
         }
         else
         {
            idx_nr = trs("{0}{1}", file_root, screenshot_idx);
         }

         //screenshot_file = dst_dir / bfs::path(trs("%1%%2%") % idx_nr % img_ext);
         std::string file_name = trs("{0}{1}", idx_nr, img_ext);
         screenshot_file = pfm_file::get_inst(dir_name + "\\" + file_name);
         screenshot_idx++;
      }
      //while (bfs::exists(screenshot_file));
      while (screenshot_file->exists());
   }
   else
   {
      screenshot_file = pfm_file::get_inst(ifilename);
   }

   res_ld::inst()->save_image(screenshot_file, gfx::rt::get_screen_width(), gfx::rt::get_screen_height(), (uint8*)begin_ptr(pixels), res_ld::e_vertical_flip);
#endif
}


void unit::app_storage::start_recording_screen(std::string i_filename, const video_params_ffmpeg* i_params)
{
   p->start_recording_screen(i_filename, i_params);
}

void unit::app_storage::stop_recording_screen()
{
   p->stop_recording_screen();
}

bool unit::app_storage::is_recording_screen()
{
   return p->is_recording_screen();
}

void unit::app_storage::toggle_screen_recording()
{
   p->toggle_screen_recording();
}


int unit::unit_count = 0;

unit::unit()
{
   char unit_name[256];

   sprintf(unit_name, "unit#%d", unit_count);
   unit_count++;
   initVal = false;
   name = string(unit_name);
   prefs = shared_ptr<unit_preferences>(new unit_preferences());
   game_time = 0;

   if (is_gfx_unit())
   {
      update_ctrl = updatectrl::new_instance();
      touch_ctrl = touchctrl::new_instance();
      key_ctrl = keyctrl::new_instance();
      gfx_scene_inst = shared_ptr<gfx_scene>(new gfx_scene());
      gfx_scene_inst->init();
   }
}

unit::~unit()
{
}

unit::unit_type unit::get_unit_type()
{
   return e_unit_base;
}

int unit::get_width()
{
   return pfm::screen::get_width();
}

int unit::get_height()
{
   return pfm::screen::get_height();
}

const string& unit::get_name()
{
   return name;
}

void unit::set_name(string i_name)
{
   name = i_name;
}

const std::string& unit::get_external_name()
{
   if (external_name.empty())
   {
      return get_name();
   }

   return external_name;
}

void unit::set_external_name(std::string i_name)
{
   external_name = i_name;
}

const std::string& unit::get_proj_rel_path()
{
   return proj_rel_path;
}

void unit::set_proj_rel_path(std::string ipath)
{
   proj_rel_path = ipath;
}

void unit::set_app_exit_on_next_run(bool iapp_exit_on_next_run)
{
   unit_ctrl::inst()->set_app_exit_on_next_run(iapp_exit_on_next_run);
}

bool unit::gfx_available()
{
   return pfm::screen::is_gfx_available();
}

shared_ptr<unit> unit::get_smtp_instance()
{
   return shared_from_this();
}

bool unit::update()
{
   int updateCount = 1;//update_ctrl->update();

   for (int k = 0; k < updateCount; k++)
   {
      touch_ctrl->update();
      key_ctrl->update();
      game_time += update_ctrl->getTimeStepDuration();
   }

   gfx_scene_inst->update();
   uxroot->update_state();

   gfx_scene_inst->draw();
   update_view(updateCount);

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

   return true;
}

void unit::on_resize()
{
   if (is_gfx_unit() && gfx::is_init())
   {
      shared_ptr<gfx_state> gfx_st = gfx::get_gfx_state();
      int w = get_width();
      int h = get_height();

      decl_gfxpl(pl1)
      {
         {gl::VIEWPORT, 0, 0, w, h},
         {},
      };

      gfx_st->set_state(pl1);

      if (uxroot)
      {
         uxroot->on_resize();
      }
   }
}

void unit::on_pause()
{
}

void unit::on_resume()
{
}

void unit::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);
      auto pa = ts->crt_state.te;
      //trx("_mt2 %1% tt %2%") % pa->is_multitouch() % pa->type;
   }
   send(uxroot, idp);

   if (!idp->is_processed())
   {
      if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
      {
         shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

         //if(ts->get_type() == touch_sym_evt::TS_BACKWARD_SWIPE)
         //{
         //	back();
         //	ts->process();
         //}
      }
      else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
      {
         shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

         if (ke->get_type() != key_evt::KE_RELEASED)
         {
            bool isAction = false;

            if (ke->get_type() != key_evt::KE_REPEATED)
            {
               isAction = true;

               switch (ke->get_key())
               {
               case KEY_F1:
                  unit_ctrl::inst()->pause();
                  break;

               case KEY_F2:
                  unit_ctrl::inst()->resume();
                  break;

               case KEY_F4:
                  storage.toggle_screen_recording();
                  break;

               case KEY_F5:
                  storage.save_screenshot();
                  break;

               case KEY_F6:
                  unit_ctrl::inst()->set_app_exit_on_next_run(true);
                  break;

               case KEY_F11:
                  pfm::screen::set_full_screen_mode(!pfm::screen::is_full_screen_mode());
                  break;

               default:
                  isAction = false;
               }
            }

            if (isAction)
            {
               ke->process();
            }
         }
      }
   }
}

void unit::iInit()
{
   init();
   storage.p->u = get_smtp_instance();

#ifdef MOD_GFX
   // getInst() doesn't work in the constructor
   if (is_gfx_unit())
   {
      touch_ctrl->add_receiver(get_smtp_instance());
      key_ctrl->add_receiver(get_smtp_instance());
      uxroot = ux_page_tab::new_instance(get_smtp_instance());
      ux_cam = ux_camera::new_inst();
      ux_cam->camera_id = "ux_cam";
      ux_cam->projection_type = "orthographic";
      ux_cam->near_clip_distance = -100;
      ux_cam->far_clip_distance = 100;
      ux_cam->clear_color = false;
      ux_cam->clear_color_value = gfx_color::colors::black;
      ux_cam->clear_depth = true;
      gfx_scene_inst->attach(ux_cam);

      init_ux();
      uxroot->init();
   }
#endif // MOD_GFX
}

/**
* Called on entering the unit for the first time.
*/
void unit::init()
{
}

/**
* Called before the unit is destroyed.
*/
void unit::on_destroy()
{
   if (isInit())
   {
      //uxroot->on_destroy();
   }
}

void unit::init_ux()
{
}

/**
* Called on entering the unit.
*/
void unit::load()
{
}

/**
* Called on leaving the unit.
*/
void unit::unload()
{
}

bool unit::rsk_was_hit(int x0, int y0)
{
   int w = 32, h = 32;
   int radius = std::max(w, h) / 2;
   int cx = get_width() - w / 2 - 4;
   int cy = get_height() - h / 2 - 4;
   int dx = cx - x0;
   int dy = cy - y0;

   if ((int)sqrtf(dx * dx + dy * dy) <= radius)
   {
      return true;
   }

   return false;
}

int unit::schedule_operation(const std::function<void()> &ioperation)
{
   operation_mutex.lock();
   operation_list.push_back(ioperation);
   operation_mutex.unlock();

   return 0;
}

bool unit::cancel_operation(int ioperation_id)
{
   return false;
}

bool unit::back()
{
#ifndef SINGLE_UNIT_BUILD
   unit_list::up_one_level();

   return false;
#else
   return true;
#endif
}

bool unit::is_gfx_unit()
{
   return get_preferences()->requires_gfx();
}

shared_ptr<unit_preferences> unit::get_preferences()
{
   return prefs;
}

bool unit::isInit()
{
   return initVal;
}

shared_ptr<ia_sender> unit::sender_inst()
{
   return get_smtp_instance();
}

bool unit::iRunFrame()
{
   font_db::inst()->on_frame_start();

   if (!operation_list.empty())
   {
      operation_mutex.lock();
      auto temp = operation_list;

      operation_list.clear();
      operation_mutex.unlock();

      for (const auto &function : temp)
      {
         function();
      }
   }

   bool drawFrame = update();

#if defined MOD_FFMPEG && defined UNIT_TEST_FFMPEG && defined MOD_GFX

   if (storage.is_recording_screen())
   {
      storage.p->update_video_encoder();
   }

#endif

   return drawFrame;
}

void unit::iLoad()
{
   fps = 0;
   frame_count = 0;
   last_frame_time = pfm::time::get_time_millis();

   load();
   //update_ctrl->started();
}

void unit::iUnload()
{
   unload();
   //update_ctrl->stopped();
}

void unit::setInit(bool isInit0)
{
   initVal = isInit0;
}

void unit::update_view(int update_count)
{
   //shared_ptr<ux_camera> gfx = gfx_openvg::get_instance();
   //sprenderer r = renderer::get_instance();
   //glm::mat4 cam, tm;
   //cam = glm::ortho(0.f, (float)get_width(), (float)get_height(), 0.f, -1.f, 1000.f);
   //r->mx.set_projection_matrix(cam);
   //r->mx.set_view_matrix(tm);

   //decl_scglpl(pl1)
   //{
   //	{gl::CULL_FACE, gl::FALSE_GL}, {gl::DEPTH_TEST, gl::FALSE_GL},
   //	{gl::DEPTH_WRITEMASK, gl::FALSE_GL}, {gl::VERTEX_ARRAY, gl::TRUE_GL},
   //	{gl::COLOR_ARRAY, gl::FALSE_GL}, {gl::NORMAL_ARRAY, gl::FALSE_GL}, 
   //	{gl::TEXTURE_COORD_ARRAY, gl::FALSE_GL}, {gl::TEXTURE_2D, gl::FALSE_GL},
   //	{},
   //};
   //r->st.set_state(pl1);

   shared_ptr<ux_camera> gfx = ux_cam;
   //gfx->sync_with_openvg();
   uxroot->update_view(gfx);

   //r->mx.set_projection_matrix(cam);
   //r->mx.set_view_matrix(tm);

   //if(prefs->show_onscreen_console())
   //{
   //	pfm::get_console()->draw(gfx);
   //}

   if (prefs->draw_touch_symbols_trail() && !touch_ctrl->is_pointer_released())
   {
      const vector<pointer_sample>& ps = touch_ctrl->get_pointer_samples();
      int size = ps.size() - 1;

      gfx->setColor(0xff0000);

      for (int k = 0; k < size; k++)
      {
         const pointer_sample& p1 = ps[k];
         const pointer_sample& p2 = ps[k + 1];

         gfx->drawLine(p1.te->points[0].x, p1.te->points[0].y, p2.te->points[0].x, p2.te->points[0].y);
      }
   }

   if (fps > 0 && !storage.is_recording_screen())
   {
      float ups = 1000.f / update_ctrl->getTimeStepDuration();
      string f = trs("uc {} u {:02.1f} f {:02.1f}", update_count, ups, fps);

      gfx->drawText(f, get_width() - 220.f, 0.f);
   }

   //signal_opengl_error();
}


int unit_list::unit_list_count = 0;


unit_list::unit_list()
{
   char unit_name[256];

   sprintf(unit_name, "unit-list#%d", unit_list_count);
   set_name(string(unit_name));

   unit_list_count++;
}

shared_ptr<unit_list> unit_list::new_instance()
{
   return shared_ptr<unit_list>(new unit_list());
}

unit::unit_type unit_list::get_unit_type()
{
   return e_unit_list;
}

void unit_list::add(shared_ptr<unit> iunit)
{
   ia_assert(iunit != shared_ptr<unit>(), "IllegalArgumentException");

   iunit->parent = get_smtp_instance();
   ulist.push_back(iunit);
   //ulmodel.lock()->notify_update();
}

shared_ptr<unit> unit_list::unit_at(int iindex)
{
   return ulist[iindex];
}

shared_ptr<unit> unit_list::unit_by_name(string iname)
{
   int size = ulist.size();

   for (int i = 0; i < size; i++)
   {
      shared_ptr<unit> u = ulist[i];

      if (u->get_name().compare(iname) == 0)
      {
         return u;
      }
   }

   return shared_ptr<unit>();
}

int unit_list::get_unit_count()const
{
   return ulist.size();
}

void unit_list::on_resize()
{
   if (ulmodel.lock())
   {
      auto u = ulist[ulmodel.lock()->get_selected_elem()];

      if (u)
      {
         u->on_resize();
      }
   }
}

void unit_list::receive(shared_ptr<iadp> idp)
{
   if (!idp->is_processed() && idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      //if (ts->get_type() == touch_sym_evt::TS_FORWARD_SWIPE)
      //{
      //	forward();
      //	ts->process();
      //}
   }

   if (!idp->is_processed())
   {
      unit::receive(idp);
   }
}

void unit_list::forward()
{
   if (ulist.size() > 0)
   {
      shared_ptr<unit> u = ulist[ulmodel.lock()->get_selected_elem()];
      unit_ctrl::inst()->set_next_unit(u);// , get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
   }
}

void unit_list::up_one_level()
{
#ifndef SINGLE_UNIT_BUILD
   shared_ptr<unit> u = unit_ctrl::inst()->get_current_unit();
   shared_ptr<unit> parent = u->parent.lock();

   if (parent != NULL)
   {
      if (parent->get_unit_type() == e_unit_list)
      {
         shared_ptr<unit_list> ul = static_pointer_cast<unit_list>(parent);
         int idx = std::find(ul->ulist.begin(), ul->ulist.end(), u) - ul->ulist.begin();

         if (idx < ul->ulist.size())
         {
            ul->ulmodel.lock()->set_selected_elem(idx);
         }
}

      unit_ctrl::inst()->set_next_unit(parent);// , get_scroll_dir(touch_sym_evt::TS_BACKWARD_SWIPE));
   }
#endif
}

void unit_list::on_destroy()
{
   ulist.clear();
}

void unit_list::init_ux()
{
   class lmodel : public ux_list_model
   {
   public:
      lmodel(shared_ptr<unit_list> iul) : ul(iul) {}

      int get_length()
      {
         return get_unit_list()->ulist.size();
      }

      std::string elem_at(int idx)
      {
         return get_unit_list()->ulist[idx]->get_name();
      }

      void on_elem_selected(int idx)
      {
         shared_ptr<unit> u = get_unit_list()->ulist[idx];

         //trx("item %1%") % elemAt(idx);
         unit_ctrl::inst()->set_next_unit(u);// , get_scroll_dir(touch_sym_evt::TS_FORWARD_SWIPE));
      }

   private:
      shared_ptr<unit_list> get_unit_list()
      {
         return ul.lock();
      }

      weak_ptr<unit_list> ul;
   };

   shared_ptr<unit_list> ul = static_pointer_cast<unit_list>(get_smtp_instance());

   shared_ptr<ux_list_model> lm((ux_list_model*)new lmodel(ul));
   shared_ptr<ux_page> p = ux_page::new_instance(uxroot);
   shared_ptr<ux_list> l = ux_list::new_instance(p);

   ulmodel = lm;
   l->set_model(lm);

   ux_cam->clear_color = true;
}


weak_ptr<unit_list> app_units_setup::ul;
weak_ptr<unit> app_units_setup::next_crt_unit;


shared_ptr<unit_list> app_units_setup::get_unit_list()
{
   return ul.lock();
}

void app_units_setup::add_unit(std::shared_ptr<unit> iu, std::string iunit_path, bool iset_current)
{
   iu->set_proj_rel_path(iunit_path);

   if (get_unit_list())
   {
      get_unit_list()->add(iu);

      if (iset_current)
      {
         next_crt_unit = iu;
      }
   }
}
