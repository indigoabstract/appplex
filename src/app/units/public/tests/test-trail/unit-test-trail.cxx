#include "stdafx.hxx"

#include "unit-test-trail.hxx"

#ifdef UNIT_TEST_TRAIL

#include "com/unit/input-ctrl.hxx"
#include "gfx.hxx"
#include "gfx-rt.hxx"
#include "gfx-camera.hxx"
#include "gfx-shader.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-vxo.hxx"
#include "gfx-state.hxx"
#include "ext/gfx-surface.hxx"
#include "ext/gfx-trail.hxx"
#include "tiny-obj-loader/tiny_obj_loader.hxx"
#include <glm/inc.hpp>


namespace ns_test_trail
{
   class gesture_state
   {
   public:
      /// gesture action codes
      enum e_code
      {
         none = 0,
         start = (1 << 0),
         move = (1 << 1),
         end = (1 << 2),
         action = start | end,
      };
   };


   class pinch_detector
   {
   public:
      pinch_detector()
      {
         start_event = std::make_shared<pointer_evt>();
      }

      /// feed new touch event and return detected state
      gesture_state::e_code detect(const mws_sp<pointer_evt> newEvent);
      /// reset the detector
      void reset();

      /// get position of first touch
      glm::vec2 position0;
      /// get position of second touch
      glm::vec2 position1;
      /// get start position of first touch
      glm::vec2 start_position0;
      /// get start position of second touch
      glm::vec2 start_position1;

      mws_sp<pointer_evt> start_event;
   };

   void pinch_detector::reset()
   {
      if (start_event->type != pointer_evt::touch_invalid)
      {
         start_event = std::make_shared<pointer_evt>();
      }
   }

   gesture_state::e_code pinch_detector::detect(const mws_sp<pointer_evt> new_event)
   {
      // check for cancelled event
      if (new_event->type == pointer_evt::touch_cancelled)
      {
         reset();

         return gesture_state::none;
      }

      // need 2 touches
      if ((new_event->type != pointer_evt::touch_ended) && (new_event->touch_count != 2))
      {
         reset();

         return gesture_state::none;
      }

      // check if touch identifiers are unchanged (number of touches and same touch ids)
      if ((start_event->type != pointer_evt::touch_invalid) && !start_event->same_touches(*new_event))
      {
         reset();

         return gesture_state::none;
      }

      // check for gesture start, move and end
      if (new_event->type == pointer_evt::touch_began)
      {
         if (new_event->touch_count < 2)
         {
            return gesture_state::none;
         }

         start_event = new_event;
         start_position0 = new_event->touch_pos(new_event->points[0].identifier);
         start_position1 = new_event->touch_pos(new_event->points[1].identifier);
         position0 = start_position0;
         position1 = start_position1;

         return gesture_state::start;
      }
      else if (new_event->type == pointer_evt::touch_moved)
      {
         // cancel if start event is not valid
         if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
         {
            return gesture_state::none;
         }

         position0 = new_event->touch_pos(start_event->points[0].identifier);
         position1 = new_event->touch_pos(start_event->points[1].identifier);

         return gesture_state::move;
      }
      else if (new_event->type == pointer_evt::touch_ended)
      {
         if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
         {
            return gesture_state::none;
         }

         position0 = new_event->touch_pos(start_event->points[0].identifier);
         position1 = new_event->touch_pos(start_event->points[1].identifier);
         reset();

         return gesture_state::end;
      }

      return gesture_state::none;
   }


   class axis_roll_detector
   {
   public:
      axis_roll_detector()
      {
         start_event = std::make_shared<pointer_evt>();
      }

      /// feed new touch event and return detected state
      gesture_state::e_code detect(const mws_sp<pointer_evt> new_event)
      {
         // check for cancelled event
         if (new_event->type == pointer_evt::touch_cancelled)
         {
            reset();

            return gesture_state::none;
         }

         // need 2 touches
         if ((new_event->type != pointer_evt::touch_ended) && (new_event->touch_count != 2))
         {
            reset();

            return gesture_state::none;
         }

         // check if touch identifiers are unchanged (number of touches and same touch ids)
         if ((start_event->type != pointer_evt::touch_invalid) && !start_event->same_touches(*new_event))
         {
            reset();

            return gesture_state::none;
         }

         // check for gesture start, move and end
         if (new_event->type == pointer_evt::touch_began)
         {
            if (new_event->touch_count < 2)
            {
               return gesture_state::none;
            }

            start_event = new_event;
            start_position0 = new_event->touch_pos(new_event->points[0].identifier);
            start_position1 = new_event->touch_pos(new_event->points[1].identifier);
            position0 = start_position0;
            position1 = start_position1;

            return gesture_state::start;
         }
         else if (new_event->type == pointer_evt::touch_moved)
         {
            // cancel if start event is not valid
            if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
            {
               return gesture_state::none;
            }

            position0 = new_event->touch_pos(start_event->points[0].identifier);
            position1 = new_event->touch_pos(start_event->points[1].identifier);

            float dist = glm::distance(start_position0, position0);

            if (dist > 50.f)
            {
               return gesture_state::none;
            }

            return gesture_state::move;
         }
         else if (new_event->type == pointer_evt::touch_ended)
         {
            if (start_event->type == pointer_evt::touch_invalid || new_event->touch_count < 2)
            {
               return gesture_state::none;
            }

            position0 = new_event->touch_pos(start_event->points[0].identifier);
            position1 = new_event->touch_pos(start_event->points[1].identifier);
            reset();

            return gesture_state::end;
         }

         return gesture_state::none;
      }

      /// reset the detector
      void reset()
      {
         if (start_event->type != pointer_evt::touch_invalid)
         {
            start_event = std::make_shared<pointer_evt>();
         }
      }

      /// get position of first touch
      glm::vec2 position0;
      /// get position of second touch
      glm::vec2 position1;
      /// get start position of first touch
      glm::vec2 start_position0;
      /// get start position of second touch
      glm::vec2 start_position1;

      mws_sp<pointer_evt> start_event;
   };
}
using namespace ns_test_trail;


class gfx_tube : public gfx_vxo
{
public:
   gfx_tube();
   virtual ~gfx_tube();
   void add_position(glm::vec3 ipos);
   float mTubeScale;

public:
   int GetVertexInRingIndex(int RingIdx, int PointIdx)
   {
      int pidx = PointIdx % mRingVertexCount;
      return RingIdx * mRingVertexCount + pidx;
   }

   int mRingVertexCount;
   float mTubeRadius;
   int max_positions;
   std::deque<glm::vec3> mPositions;
};


gfx_tube::gfx_tube() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
   setup_tangent_basis = false;
   max_positions = 1500;
   mRingVertexCount = 3;
   mTubeRadius = 0.2f;
   mTubeScale = 1500.f;
}

gfx_tube::~gfx_tube()
{
}

void gfx_tube::add_position(glm::vec3 ipos)
{
   // skip points too close together
   if (!mPositions.empty() && glm::distance2(mPositions.front(), ipos) < 0.01f)
   {
      return;
   }

   // push newest point in front
   mPositions.push_front(ipos);

   if (mPositions.size() > max_positions)
   {
      mPositions.pop_back();
   }

   if (mPositions.size() < 2)
   {
      return;
   }

   auto inst = std::static_pointer_cast<gfx_vxo>(get_mws_sp());
   std::vector<float> tvertices_data;

   glm::vec3 x_axis(1, 0, 0);
   glm::vec3 y_axis(0, 1, 0);

   // start from oldest to newest point
   for (int k = mPositions.size() - 1; k >= 0; k--)
   {
      glm::vec3 fwd_dir;

      if (k == 0)
      {
         fwd_dir = mPositions[k] - mPositions[k + 1];
      }
      else
      {
         fwd_dir = mPositions[k - 1] - mPositions[k];
      }

      fwd_dir = glm::normalize(fwd_dir);
      glm::vec3 first_normal = glm::cross(fwd_dir, y_axis);
      first_normal = glm::normalize(first_normal);

      // if first_normal is too close to y_axis
      if (first_normal.length() < 0.1f)
      {
         first_normal = glm::cross(fwd_dir, x_axis);
         first_normal = glm::normalize(first_normal);
      }

      for (int vx_idx = 0; vx_idx < mRingVertexCount; vx_idx++)
      {
         float angle = (2.f * glm::pi<float>() / mRingVertexCount) * vx_idx;
         glm::quat rotation = glm::angleAxis(angle, fwd_dir);
         glm::vec3 normal = rotation * first_normal;
         normal = glm::normalize(normal);
         glm::vec3 point = mPositions[k] + normal * mTubeRadius * mTubeScale;
         glm::vec2 tex_coord(0.f);

         tvertices_data.push_back(point.x);
         tvertices_data.push_back(point.y);
         tvertices_data.push_back(point.z);
         tvertices_data.push_back(normal.x);
         tvertices_data.push_back(normal.y);
         tvertices_data.push_back(normal.z);
         tvertices_data.push_back(tex_coord.x);
         tvertices_data.push_back(tex_coord.y);
      }
   }

   std::vector<uint32> tindices_data;
   int indices_size = mPositions.size() - 1;
   int quad_index_count = 6;
   tindices_data.resize(indices_size * quad_index_count * mRingVertexCount);

   // ring index, from oldest to newest ring
   for (int ring_idx = 0, index = 0; ring_idx < indices_size; ring_idx++)
   {
      // point in ring index
      for (int point_idx = 0; point_idx < mRingVertexCount; point_idx++)
      {
         tindices_data[index++] = GetVertexInRingIndex(ring_idx + 1, point_idx);// 0;
         tindices_data[index++] = GetVertexInRingIndex(ring_idx, point_idx + 1);// 2;
         tindices_data[index++] = GetVertexInRingIndex(ring_idx, point_idx);// 1;
         tindices_data[index++] = GetVertexInRingIndex(ring_idx + 1, point_idx + 1);// 3;
         tindices_data[index++] = GetVertexInRingIndex(ring_idx, point_idx + 1);// 2;
         tindices_data[index++] = GetVertexInRingIndex(ring_idx + 1, point_idx);// 0;
      }
   }

   gfx_vxo_util::set_mesh_data((const uint8*)&tvertices_data[0], tvertices_data.size() * sizeof(float), &tindices_data[0], tindices_data.size() * sizeof(uint32), inst);
}


class unit_test_trail_impl
{
public:
   unit_test_trail_impl()
   {
      t = 0;
      t2 = 0;
      phi = 0;
      theta = 0;
      look_at_dir = glm::vec3(0.f, 0.f, -1.f);
      up_dir = glm::vec3(0.f, 1.f, 0.f);
      speed = 0;
      sphere_radius = 7000;
      trail_rot_angle_0 = 0;
      trail_rot_angle_1 = 0;
      trail_dist = 3900;
      tube_rot_angle_0 = 0;
      tube_rot_angle_1 = 0;
      tube_dist = 4900;
      last_dist = 0;
   }

   void load_trail_data()
   {
      auto file = pfm_file::get_inst("trail-points-12.data");

      if (file->exists())
      {
         auto fds = std::make_shared<file_data_sequence>(file);
         auto dr = std::make_shared<data_sequence_reader>(fds);

         file->io.open();
         int size = dr->read_int32();

         tube_pos.resize(size);
         tube_ts.resize(size);

         for (int i = 0; i < tube_pos.size(); i++)
         {
            float x = dr->read_real32();
            float y = dr->read_real32();
            float z = dr->read_real32();
            tube_pos[i] = glm::vec3(x, y, z);
         }
         for (int i = 0; i < tube_ts.size(); i++)
         {
            int ts = dr->read_int32();
            tube_ts[i] = ts;
         }
      }
      else
      {
         trx("trail-points.data not found");
      }
   }

   mws_sp<gfx_trail> trail_mesh;
   mws_sp<gfx_tube> tube_mesh;
   mws_sp<gfx_camera> persp_cam;
   float t;
   float t2;
   float phi;
   float theta;
   float trail_rot_angle_0;
   float trail_rot_angle_1;
   float trail_dist;
   float tube_rot_angle_0;
   float tube_rot_angle_1;
   float tube_dist;
   glm::vec3 look_at_dir;
   glm::vec3 up_dir;
   float speed;
   float sphere_radius;
   uint32 last_update_time;
   std::vector<glm::vec3> tube_pos;
   std::vector<int> tube_ts;

   kinetic_scrolling ks;
   pinch_detector pinch_gest_det;
   axis_roll_detector axis_roll_gest_det;
   float last_dist;
};


unit_test_trail::unit_test_trail() : unit(mws_stringify(UNIT_TEST_TRAIL)) {}

mws_sp<unit_test_trail> unit_test_trail::nwi()
{
   return mws_sp<unit_test_trail>(new unit_test_trail());
}

void unit_test_trail::init()
{
   //touch_ctrl->add_receiver(get_smtp_instance());
   //key_ctrl_inst->add_receiver(get_smtp_instance());
}

void unit_test_trail::load()
{
   p = mws_sp<unit_test_trail_impl>(new unit_test_trail_impl());

   {
      p->persp_cam = gfx_camera::nwi();
      p->persp_cam->name = "defcam";
      p->persp_cam->camera_id = "default";
      p->persp_cam->rendering_priority = 0;
      p->persp_cam->near_clip_distance = 0.01f;
      p->persp_cam->far_clip_distance = 50000.f;
      p->persp_cam->fov_y_deg = 60.f;
      p->persp_cam->clear_color = true;
      p->persp_cam->clear_color_value = gfx_color::colors::black;
      p->persp_cam->clear_depth = true;
   }

   {
      p->trail_mesh = mws_sp<gfx_trail>(new gfx_trail());
      gfx_trail& r_trail_mesh = *p->trail_mesh;
      r_trail_mesh[MP_SHADER_NAME] = "trail";
      r_trail_mesh[MP_CULL_BACK] = false;
      r_trail_mesh[MP_CULL_FRONT] = false;
      r_trail_mesh[MP_BLENDING] = MV_ALPHA;
      r_trail_mesh["u_v4_color"] = glm::vec4(0, 1, 0, 1);
      //r_trail_mesh[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
   }

   {
      p->tube_mesh = mws_sp<gfx_tube>(new gfx_tube());
      gfx_tube& r_tube_mesh = *p->tube_mesh;
      r_tube_mesh[MP_SHADER_NAME] = "tube";
      r_tube_mesh[MP_CULL_BACK] = false;
      r_tube_mesh[MP_CULL_FRONT] = false;
      r_tube_mesh[MP_BLENDING] = MV_ALPHA;
      r_tube_mesh["u_v4_color"] = glm::vec4(0, 1, 0, 1);
      //r_tube_mesh[MP_WIREFRAME_MODE] = MV_WF_OVERLAY;
   }

   gfx_scene_inst->attach(p->persp_cam);
   gfx_scene_inst->attach(p->trail_mesh);
   gfx_scene_inst->attach(p->tube_mesh);

   p->persp_cam->position = glm::vec3(0.f, 0.f, p->sphere_radius + 15.f);
   p->last_update_time = pfm::time::get_time_millis();
   p->load_trail_data();

   p->tube_mesh->position = glm::vec3(0.f);
   //p->tube_mesh->add_position(glm::vec3(0.f) + p->tube_mesh->mTubeScale);
   //p->tube_mesh->add_position(glm::vec3(250.f) + p->tube_mesh->mTubeScale);
   //p->tube_mesh->add_position(glm::vec3(250.f,250,250) + p->tube_mesh->mTubeScale);
   //p->tube_mesh->add_position(glm::vec3(5750.f, 750.f, 750.f) + p->tube_mesh->mTubeScale);

   mws_report_gfx_errs();
}
bool update_started = false;
int idx = 0;

bool unit_test_trail::update()
{
   uint32 last_update_time = pfm::time::get_time_millis();

   if (!update_started)
   {
      p->last_update_time = last_update_time;
      update_started = true;
   }

   float t = p->t;
   float t2 = p->t2;
   //impl->t += 0.00005f;
   //impl->t2 += 0.005f;

   glm::vec3 trail_pos;
   p->trail_rot_angle_0 += 0.009f;
   p->trail_rot_angle_1 += 0.025f;
   trail_pos.x = glm::cos(p->trail_rot_angle_0) * p->trail_dist;
   trail_pos.y = glm::cos(p->trail_rot_angle_0) * glm::sin(p->trail_rot_angle_1) * p->trail_dist;
   trail_pos.z = glm::sin(p->trail_rot_angle_0) * p->trail_dist;
   p->trail_mesh->position = glm::vec3(0.f);
   p->trail_mesh->add_position(trail_pos);
   p->trail_mesh->visible = false;

   uint32 diff = last_update_time - p->last_update_time;

   if (idx < p->tube_ts.size() && diff >= p->tube_ts[idx])
   {
      glm::vec3 tube_pos2 = p->tube_pos[idx] * p->tube_mesh->mTubeScale;
      //if (idx == 2)
      //{
      //   tube_pos2 = (p->tube_pos[idx+10]+glm::vec3(0.f, 0.f, 0.f)) * p->tube_mesh->mTubeScale;
      //}
      p->tube_mesh->position = glm::vec3(0.f);
      p->tube_mesh->add_position(tube_pos2);
      //trx("idx {}", idx);
      idx++;
   }

   //for (size_t i = 0; i < p->tube_ts.size(); i++)
   //{
   //   glm::vec3 tube_pos2 = p->tube_pos[i] * p->tube_mesh->mTubeScale;
   //   p->persp_cam->draw_point(tube_pos2, gfx_color::colors::red.to_vec4(), 25.f);
   //}

   //for (size_t i = 0; i < p->tube_mesh->mPositions.size(); i++)
   //{
   //   p->persp_cam->draw_point(p->tube_mesh->mPositions[i], gfx_color::colors::red.to_vec4(), 25.f);
   //}

   //if(last_update_time - p->last_update_time > 500)
   //{
   //   glm::vec3 tube_pos;
   //   float tube_off = 3.f;
   //   p->tube_rot_angle_0 += 0.009f;
   //   p->tube_rot_angle_1 += 0.025f;
   //   tube_pos.x = glm::cos(p->tube_rot_angle_0 + tube_off) * p->tube_dist;
   //   tube_pos.y = glm::cos(p->tube_rot_angle_0 + tube_off) * glm::sin(p->tube_rot_angle_1 + tube_off) * p->tube_dist;
   //   tube_pos.z = glm::sin(p->tube_rot_angle_0 + tube_off) * p->tube_dist;
   //   p->tube_mesh->position = glm::vec3(0.f);
   //   p->tube_mesh->add_position(tube_pos);
   //   p->last_update_time = last_update_time;
   //}

   p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(5000, 0, 0), glm::vec4(1, 0, 0.f, 1.f), 1.f);
   p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 5000, 0), glm::vec4(0, 1, 0.f, 1.f), 1.f);
   p->persp_cam->draw_line(glm::vec3(0.f), glm::vec3(0, 0, 5000), glm::vec4(0, 0, 1.f, 1.f), 1.f);

   p->persp_cam->position += p->look_at_dir * p->speed;
   p->persp_cam->look_at(p->look_at_dir, p->up_dir);

   glm::vec3 forward_dir = p->persp_cam->get_forward_dir();

   mws_report_gfx_errs();

   return unit::update();
}

void unit_test_trail::receive(mws_sp<mws_dp> idp)
{
   if (!idp->is_processed())
   {
      if (idp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
      {
         mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(idp);

         switch (ts->get_type())
         {
         case touch_sym_evt::TS_PRESSED:
         {
            if (ts->touch_count == 2)
            {
               p->ks.grab(ts->points[0].x, ts->points[0].y);
            }
            //ts->process();
            break;
         }
         }

         {
            auto& axis_roll_gest_det = p->axis_roll_gest_det;
            auto& pinch_gest_det = p->pinch_gest_det;
            auto axis_roll_state = axis_roll_gest_det.detect(ts->crt_state.te);
            auto pinch_state = pinch_gest_det.detect(ts->crt_state.te);
            trx("geture state {0} {1}", pinch_state, axis_roll_state);

            switch (axis_roll_state)
            {
            case gesture_state::start:
            {
               break;
            }

            case gesture_state::move:
            {
               int w = get_width();
               int h = get_height();
               float dx = ts->points[1].x - ts->prev_state.te->points[1].x;
               float dy = ts->points[1].y - ts->prev_state.te->points[1].y;

               if (dx != 0 || dy != 0)
               {
                  float scale = 0.25f;
                  //float dx_rad = glm::radians(dx * scale);
                  //float dy_rad = glm::radians(-dy * scale);
                  glm::vec3 screen_center(w * 0.5f, h * 0.5f, 0.f);
                  glm::vec3 oriented_radius = glm::vec3(ts->prev_state.te->points[1].x, ts->prev_state.te->points[1].y, 0.f) - screen_center;
                  glm::vec3 neg_z_axis(0.f, 0.f, -1.f);
                  glm::vec3 circle_tangent = glm::cross(neg_z_axis, oriented_radius);
                  circle_tangent = glm::normalize(circle_tangent);
                  glm::vec3 touch_dir(dx, dy, 0.f);
                  touch_dir = glm::normalize(touch_dir);
                  float dot_prod = glm::dot(touch_dir, circle_tangent);
                  float cos_alpha = dot_prod / (glm::length(touch_dir) * glm::length(circle_tangent));

                  glm::quat rot_around_look_at_dir = glm::angleAxis(-cos_alpha * 0.1f, p->look_at_dir);
                  p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);

                  if (ts->is_finished)
                  {
                     p->ks.start_slowdown();
                  }
                  else
                  {
                     p->ks.begin(ts->points[1].x, ts->points[1].y);
                  }

                  //arcball_cam->theta_deg += glm::radians(dx * 9.f);
                  //arcball_cam->phi_deg -= glm::radians(dy * 5.f);
                  //arcball_cam->clamp_angles();
                  ts->process();
                  //arcball_cam->movement_type = arcball_cam->e_roll_view_axis;
               }
               break;
            }
            }

            //if (axis_roll_state != gesture_state::move)
            if (!ts->is_processed())
            {
               switch (pinch_state)
               {
               case gesture_state::start:
               {
                  p->last_dist = glm::distance(pinch_gest_det.position0, pinch_gest_det.position1);
                  break;
               }

               case gesture_state::move:
               {
                  float dist = glm::distance(pinch_gest_det.position0, pinch_gest_det.position1);
                  float delta = dist - p->last_dist;

                  p->last_dist = dist;
                  p->persp_cam->position += p->look_at_dir * 5.f * delta;
                  trx("geture dist {0} {1}", dist, delta);
                  ts->process();
                  break;
               }
               }
            }
         }

         if (!ts->is_processed())
         {
            //mws_print("tn %s\n", ts->get_type_name(ts->get_type()).c_str());
            if (ts->get_type() == touch_sym_evt::TS_PRESS_AND_DRAG)
            {
               float dx = ts->points[0].x - ts->prev_state.te->points[0].x;
               float dy = ts->points[0].y - ts->prev_state.te->points[0].y;
               float scale = 0.25f;
               float dx_rad = glm::radians(dx * scale);
               float dy_rad = glm::radians(dy * scale);

               //impl->theta += glm::radians(dx / 10);
               //impl->phi += glm::radians(-dy / 10);
               //impl->theta = glm::mod(impl->theta, 2 * glm::pi<float>());
               //impl->phi = glm::mod(impl->phi, 2 * glm::pi<float>());
               //glm::vec3 axis(glm::sin(impl->theta) * glm::sin(impl->phi), glm::cos(impl->theta) * glm::sin(impl->phi), -glm::cos(impl->phi));
               //axis = glm::normalize(axis);
               //mws_print("x %f, y %f\n", impl->theta, impl->phi);
               //mws_print("x %f, y %f, z %f - th %f, ph %f\n", axis.x, axis.y, axis.z, impl->theta, impl->phi);
               glm::vec3 right_dir = glm::cross(p->look_at_dir, p->up_dir);
               glm::quat rot_around_right_dir = glm::angleAxis(dy_rad, right_dir);
               p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_right_dir);
               p->up_dir = glm::normalize(glm::cross(right_dir, p->look_at_dir));

               glm::quat rot_around_up_dir = glm::angleAxis(dx_rad, p->up_dir);
               p->look_at_dir = glm::normalize(p->look_at_dir * rot_around_up_dir);
               ts->process();
            }
            else if (ts->get_type() == touch_sym_evt::TS_MOUSE_WHEEL)
            {
               mws_sp<mouse_wheel_evt> mw = static_pointer_cast<mouse_wheel_evt>(ts);

               p->persp_cam->position += p->look_at_dir * 150.f * float(mw->wheel_delta);
               ts->process();
            }
         }
      }
      else if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
      {
         mws_sp<key_evt> ke = key_evt::as_key_evt(idp);

         if (ke->get_type() != key_evt::KE_RELEASED)
         {
            bool do_action = true;

            switch (ke->get_key())
            {
            case KEY_Q:
            {
               p->persp_cam->position -= p->look_at_dir * 100.05f;
               break;
            }

            case KEY_E:
            {
               p->persp_cam->position += p->look_at_dir * 100.05f;
               break;
            }

            case KEY_A:
            {
               glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(+5.f), p->look_at_dir);
               p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
               break;
            }

            case KEY_D:
            {
               glm::quat rot_around_look_at_dir = glm::angleAxis(glm::radians(-5.f), p->look_at_dir);
               p->up_dir = glm::normalize(p->up_dir * rot_around_look_at_dir);
               break;
            }

            case KEY_Z:
            {
               p->speed -= 0.05f;
               break;
            }

            case KEY_C:
            {
               p->speed += 0.05f;
               break;
            }

            default:
               do_action = false;
            }

            if (!do_action && ke->get_type() != key_evt::KE_REPEATED)
            {
               do_action = true;

               switch (ke->get_key())
               {
               case KEY_SPACE:
               case KEY_F1:
                  //vdec->play_pause();
                  break;

               case KEY_BACKSPACE:
               case KEY_F2:
                  //vdec->stop();
                  break;

               case KEY_F6:
                  //unit_ctrl::inst()->set_app_exit_on_next_run(true);
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
               ke->process();
            }
         }
      }
   }
}

#endif
