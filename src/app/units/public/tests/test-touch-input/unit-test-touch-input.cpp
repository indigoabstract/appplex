#include "stdafx.h"
#include "appplex-conf.hpp"

#ifdef UNIT_TEST_TOUCH_INPUT

#include "unit-test-touch-input.hpp"
#include "com/unit/gesture-detectors.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-com.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "gfx-vxo.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx-material.hpp"
#include "gfx-rt.hpp"
#include "pfm-gl.h"


namespace unit_test_touch_input_ns
{
   class unit_preferences_detail : public unit_preferences
   {
   public:
      virtual int get_preferred_screen_width() override { return 586; }//640
      virtual int get_preferred_screen_height() override { return 1040; }//1136
      virtual double get_preferred_aspect_ratio() override { return 640. / 1136.; }
   };
}


unit_test_touch_input::unit_test_touch_input()
{
   set_name("test-touch-input");
   prefs = mws_sp<unit_preferences>(new unit_test_touch_input_ns::unit_preferences_detail());
}

mws_sp<unit_test_touch_input> unit_test_touch_input::new_instance()
{
   return mws_sp<unit_test_touch_input>(new unit_test_touch_input());
}


namespace unit_test_touch_input_ns
{
   class main_page : public mws_page
   {
   public:
      virtual void init()
      {
         auto scene = get_unit()->gfx_scene_inst;

         frame_idx = 0;
         gpu_readback_init = false;
         gpu_readback_enabled = true;
         last_click = glm::vec2(-1.f);
         map_click_x = map_click_y = 0;
         obj_index = -1;
         pbo_index = 0;
         pbo_data_size = 0;
         obj_scaling = 1.f;

         add_objects();
      }

      void add_objects()
      {
         auto scene = get_unit()->gfx_scene_inst;
         glm::vec2 cn(400, 800);

         // triangle
         {
            obj_vect.push_back(std::make_shared<gfx_quad_2d>());
            gfx_quad_2d& mq = *obj_vect.back();
            mq.set_dimensions(1, 1);
            mq.set_translation(cn + glm::vec2(-50, -50));
            mq[MP_DEPTH_TEST] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq["u_s2d_tex"] = "logo-0.png";
         }

         // square quad
         {
            obj_vect.push_back(std::make_shared<gfx_quad_2d>());
            gfx_quad_2d& mq = *obj_vect.back();
            mq.set_dimensions(1, 1);
            mq.set_translation(cn);
            mq[MP_DEPTH_TEST] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq["u_s2d_tex"] = "logo-1.png";
         }

         // pentagon
         {
            obj_vect.push_back(std::make_shared<gfx_quad_2d>());
            gfx_quad_2d& mq = *obj_vect.back();
            mq.set_dimensions(1, 1);
            mq.set_translation(cn + glm::vec2(50, 50));
            mq[MP_DEPTH_TEST] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq["u_s2d_tex"] = "logo-2.png";
         }

         // hexagon
         {
            obj_vect.push_back(std::make_shared<gfx_quad_2d>());
            gfx_quad_2d& mq = *obj_vect.back();
            mq.set_dimensions(1, 1);
            mq.set_translation(cn + glm::vec2(-50, 50));
            mq[MP_DEPTH_TEST] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq["u_s2d_tex"] = "logo-3.png";
         }

         // octagon
         {
            obj_vect.push_back(std::make_shared<gfx_quad_2d>());
            gfx_quad_2d& mq = *obj_vect.back();
            mq.set_dimensions(1, 1);
            mq.set_translation(cn + glm::vec2(50, -50));
            mq[MP_DEPTH_TEST] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq["u_s2d_tex"] = "logo-4.png";
         }

         // picking detector debug
         {
            gfx_quad_2d& mq = *(picking_dgb_q2d = std::make_shared<gfx_quad_2d>());
            //picking_tex_dim = glm::vec2(256, 512);
            picking_tex_dim = glm::vec2(128, 256);
            //picking_tex_dim = glm::vec2(gfx::i()->rt.get_screen_width(), gfx::i()->rt.get_screen_height());

            if (gfx::i()->rt.get_screen_width() > gfx::i()->rt.get_screen_height())
            {
               picking_tex_dim = glm::vec2(picking_tex_dim.y, picking_tex_dim.x);
            }

            mq.set_dimensions(1, 1);
            mq.set_scale(picking_tex_dim.x, picking_tex_dim.y);
            mq.set_translation(50, 50);
            mq[MP_DEPTH_TEST] = true;
            mq[MP_DEPTH_WRITE] = true;
            mq[MP_SHADER_NAME] = "basic-tex-shader";
            mq.camera_id_list.clear();
            mq.camera_id_list.push_back(get_unit()->mws_cam->camera_id());
            mq.set_v_flip(true);
            mq.set_z(obj_vect.size() + 2.f);
            picking_dgb_q2d->visible = true;
         }

         int z_pos = 1;

         for (auto m : obj_vect)
         {
            m->set_anchor(m->e_center);
            (*m)[MP_BLENDING] = MV_ALPHA;
            m->set_z(float(z_pos++));
            m->camera_id_list.clear();
            m->camera_id_list.push_back(get_unit()->mws_cam->camera_id());
            scene->attach(m);
         }

         set_obj_scaling(1.5f);
         scene->attach(picking_dgb_q2d);
         get_unit()->mws_cam->sort_function = get_unit()->mws_cam->z_order_sort_function;
      }

      virtual void receive(mws_sp<iadp> idp)
      {
         if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
         {
            shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);
            glm::vec2 dragging_delta;
            float zoom_factor;
            bool dragging_detected = dragging_dt.detect_helper(ts->crt_state.te, dragging_delta);
            bool pinch_zoom_detected = pinch_zoom_dt.detect_helper(ts->crt_state.te, zoom_factor);
            auto double_tap_gs = double_tap_dt.detect(ts->crt_state.te);

            if (double_tap_gs == GS_ACTION)
            {
               if (picking_dgb_q2d)
               {
                  picking_dgb_q2d->visible = !picking_dgb_q2d->visible;
               }
            }

            if (dragging_detected)
            {
               if (obj_index >= 0)
               {
                  auto& obj = obj_vect[obj_index];

                  obj->set_translation(obj->get_translation() + dragging_delta);
               }
            }

            if (pinch_zoom_detected)
            {
               float scale = obj_scaling + zoom_factor * 0.005f;
               float inf_lim = 0.25f;
               float sup_lim = 5.f;

               if (scale < inf_lim)
               {
                  scale = inf_lim;
               }
               else if (scale > sup_lim)
               {
                  scale = sup_lim;
               }

               set_obj_scaling(scale);
               mws_print("pinch zoom [%f]\n", zoom_factor);
            }

            switch (ts->get_type())
            {
            case touch_sym_evt::TS_PRESSED:
               last_click = glm::vec2(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y);
               break;

            case touch_sym_evt::TS_MOUSE_WHEEL:
            {
               shared_ptr<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);
               float scale = obj_scaling + mw->wheel_delta * 0.1f;
               float inf_lim = 0.25f;
               float sup_lim = 5.f;

               if (scale < inf_lim)
               {
                  scale = inf_lim;
               }
               else if (scale > sup_lim)
               {
                  scale = sup_lim;
               }

               set_obj_scaling(scale);
               ts->process();
               break;
            }
            }
         }
         else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
         {
            shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

            if (ke->is_pressed())
            {
               bool isAction = true;

               switch (ke->get_key())
               {
               case KEY_H:
               {
                  if (picking_dgb_q2d)
                  {
                     picking_dgb_q2d->visible = !picking_dgb_q2d->visible;
                  }

                  break;
               }

               default:
                  isAction = false;
               }

               if (isAction)
               {
                  ke->process();
               }
            }
         }

         mws_page::receive(idp);
      }

      virtual void update_state()
      {
         mws_page::update_state();
      }

      virtual void update_view(mws_sp<mws_camera> g)
      {
         //mws_page::update_view(g);

         //g->drawLine(pos_pt.x, pos_pt.y, pointer_pt.x, pointer_pt.y);
         //g->drawLine(pos_pt.x, pos_pt.y, side_pt.x, side_pt.y);

         if (gpu_readback_enabled)
         {
            mws_assert(PBO_COUNT >= 2);

            if (!gpu_readback_init)
            {
               pbo_data_size = int(picking_tex_dim.x) * int(picking_tex_dim.y);
               gfx_tex_params prm;

               prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
               prm.max_anisotropy = 0.f;
               prm.min_filter = gfx_tex_params::e_tf_nearest;
               prm.mag_filter = gfx_tex_params::e_tf_nearest;
               prm.gen_mipmaps = false;
               prm.max_anisotropy = 0.f;

               picking_tex = gfx::i()->tex.new_tex_2d(gfx_tex::gen_id(), int(picking_tex_dim.x), int(picking_tex_dim.y), "R8", &prm);
               picking_rt = gfx::i()->rt.new_rt();
               picking_rt->set_color_attachment(picking_tex);

               pbo_id_vect.resize(PBO_COUNT);
               glGenBuffers(PBO_COUNT, pbo_id_vect.data());
               //pbo_data.resize(pbo_data_size);

               for (uint32 k = 0; k < PBO_COUNT; k++)
               {
                  glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[k]);
                  glBufferData(GL_PIXEL_PACK_BUFFER, pbo_data_size, 0, GL_STREAM_READ);
               }

               picking_shader = gfx::i()->shader.new_program("tex-picking", "tex-picking");
               (*picking_dgb_q2d)["u_s2d_tex"] = picking_tex->get_name();
               gpu_readback_init = true;
            }

            gfx::i()->rt.set_current_render_target(picking_rt);

            mws_report_gfx_errs();
            shared_ptr<gfx_state> gl_st = gfx::i()->get_gfx_state();
            decl_scgfxpl(plist)
            {
               { gl::COLOR_CLEAR_VALUE, 0.f, 0.f, 0.f, 1.f }, { gl::CLEAR_MASK, gl::COLOR_BUFFER_BIT_GL | gl::DEPTH_BUFFER_BIT_GL }, {},
            };
            gl_st->set_state(plist);

            auto z_sort = [](mws_sp<gfx_quad_2d> a, mws_sp<gfx_quad_2d> b)
            {
               return (a->position().z < b->position().z);
            };

            std::sort(obj_vect.begin(), obj_vect.end(), z_sort);
            g->update_rt_cam_state = false;

            for (auto m : obj_vect)
            {
               float z_pos = m->position().z / obj_vect.size();
               (*m)[MP_SHADER_NAME] = "tex-picking";
               (*m)[MP_DEPTH_TEST] = false;
               (*m)[MP_BLENDING] = MV_NONE;
               (*m)["u_v1_z_pos"] = z_pos;
               m->render_mesh(g);
               (*m)[MP_SHADER_NAME] = "basic-tex-shader";
               (*m)[MP_DEPTH_TEST] = true;
               (*m)[MP_BLENDING] = MV_ALPHA;
            }

            g->update_rt_cam_state = true;
            bool pbo_supported = true;

            // with PBO
            if (pbo_supported)
            {
               read_pixels_pbo(g);
            }

            mws_report_gfx_errs();
            gfx::i()->rt.set_current_render_target(nullptr);

            if (picking_dgb_q2d && picking_dgb_q2d->visible)
            {
               //picking_dgb_q2d->update_recursive(glm::mat4(), true);
               //picking_dgb_q2d->render_mesh(g);
               auto tr = picking_dgb_q2d->get_translation();
               g->draw_point(glm::vec3(tr.x + map_click_x, tr.y + map_click_y, picking_dgb_q2d->get_z() + 1.f), glm::vec4(1., 1, 1, 1.), 5.f);
            }

            // increment current index first then get the next index
            // pbo_index is used to read pixels from a framebuffer to a PBO
            pbo_index = (pbo_index + 1) % PBO_COUNT;
         }

         frame_idx++;
      }

      void read_pixels_pbo(mws_sp<mws_camera> g)
      {
         auto& tex_prm = picking_tex->get_params();

         mws_report_gfx_errs();
         glReadBuffer(GL_COLOR_ATTACHMENT0);

         // copy pixels from framebuffer to PBO and use offset instead of pointer.
         // OpenGL should perform async DMA transfer, so glReadPixels() will return immediately.
         glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[pbo_index]);
         mws_report_gfx_errs();
         glPixelStorei(GL_PACK_ALIGNMENT, 1);
         mws_report_gfx_errs();
         glReadPixels(0, 0, picking_tex->get_width(), picking_tex->get_height(), tex_prm.format, tex_prm.type, 0);

         mws_report_gfx_errs();
         if ((frame_idx >= PBO_COUNT - 1) && last_click.x >= 0.f)
         {
            // pbo_next_index is used to process pixels in the other PBO
            int pbo_next_index = (pbo_index + 1) % PBO_COUNT;

            // map the PBO containing the framebuffer pixels before processing it
            glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo_id_vect[pbo_next_index]);
            // glGetBufferSubData(GL_PIXEL_PACK_BUFFER, 0, pbo_data.size(), pbo_data.data());
            GLubyte* src = (GLubyte*)glMapBufferRange(GL_PIXEL_PACK_BUFFER, 0, pbo_data_size, GL_MAP_READ_BIT);

            mws_report_gfx_errs();
            if (src)
            {
               if (g->projection_type == g->e_perspective_proj)
               {
                  float screen_aspect_ratio = gfx::i()->rt.get_screen_width() / float(gfx::i()->rt.get_screen_height());
                  float new_hs_tex_width = picking_tex->get_height() * screen_aspect_ratio;
                  float hs_tex_width_delta = picking_tex->get_width() - new_hs_tex_width;
                  float tf_x = new_hs_tex_width / float(gfx::i()->rt.get_screen_width());
                  float tf_y = picking_tex->get_height() / float(gfx::i()->rt.get_screen_height());
                  map_click_x = int(hs_tex_width_delta / 2.f + last_click.x * tf_x);
                  map_click_y = int(last_click.y * tf_y);
               }
               else if (g->projection_type == g->e_orthographic_proj)
               {
                  float tf_x = picking_tex->get_width() / float(gfx::i()->rt.get_screen_width());
                  float tf_y = picking_tex->get_height() / float(gfx::i()->rt.get_screen_height());
                  map_click_x = int(last_click.x * tf_x);
                  map_click_y = int(last_click.y * tf_y);
               }

               int idx = (picking_tex->get_height() - map_click_y - 1) * picking_tex->get_width() + map_click_x;
               //uint8 px = pbo_data[idx];
               uint8 px = src[idx];

               // release pointer to the mapped buffer
               glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
               mws_report_gfx_errs();
               glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
               mws_report_gfx_errs();

               last_click = glm::vec2(-1.f);
               obj_index = -1;

               if (px > 0)
               {
                  float vx_id = px / 255.f;
                  float f_idx = vx_id * obj_vect.size();

                  obj_index = int(roundf(f_idx));

                  if (obj_index > 0)
                  {
                     obj_index -= 1;
                     mws_print("obj_idx [%d] vx_id [%f]\n", obj_index, f_idx);
                  }
                  else
                  {
                     mws_print("error. obj_idx [%d] must be greater than 0. vx_id [%f]\n", obj_index, f_idx);
                  }

                  // put this object on top
                  if (obj_index < int(obj_vect.size() - 1))
                  {
                     float top_z_pos = obj_vect.back()->get_z();
                     auto obj = obj_vect[obj_index];

                     obj_vect.erase(obj_vect.begin() + obj_index);
                     obj_vect.push_back(obj);

                     for (uint32 k = obj_index; k < obj_vect.size(); k++)
                     {
                        obj_vect[k]->set_z(obj_vect[k]->get_z() - 1.f);
                     }

                     obj_vect.back()->set_z(top_z_pos);
                     obj_index = obj_vect.size() - 1;
                  }
               }
            }
            else
            {
               // release pointer to the mapped buffer
               glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
               mws_report_gfx_errs();
               glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
               mws_report_gfx_errs();
            }
         }
      }

      void set_obj_scaling(float i_scaling)
      {
         float tex_width = 256;
         float tex_height = 256;

         obj_scaling = i_scaling;

         for (auto m : obj_vect)
         {
            m->set_scale(tex_width * obj_scaling, tex_height * obj_scaling);
         }
      }

      uint32 frame_idx;
      std::vector<std::shared_ptr<gfx_quad_2d> > obj_vect;
      float obj_scaling;
      glm::vec2 last_click;
      int map_click_x;
      int map_click_y;
      int obj_index;

      // picking
      bool gpu_readback_init;
      bool gpu_readback_enabled;
      glm::vec2 picking_tex_dim;
      std::shared_ptr<gfx_quad_2d> picking_dgb_q2d;
      std::shared_ptr<gfx_tex> picking_tex;
      std::shared_ptr<gfx_rt> picking_rt;
      std::shared_ptr<gfx_shader> picking_shader;
      const uint32 PBO_COUNT = 2;
      std::vector<gfx_uint> pbo_id_vect;
      int pbo_index;
      std::vector<uint8> pbo_data;
      int pbo_data_size;

      // gesture
      pinch_zoom_detector pinch_zoom_dt;
      double_tap_detector double_tap_dt;
      dragging_detector dragging_dt;
   };
}


void unit_test_touch_input::init_mws()
{
   auto page = mws_page::new_shared_instance(mws_root, new unit_test_touch_input_ns::main_page());
   mws_cam->clear_color = true;
   mws_cam->clear_depth = true;
   mws_cam->clear_color_value = gfx_color::colors::indigo;
}

#endif
