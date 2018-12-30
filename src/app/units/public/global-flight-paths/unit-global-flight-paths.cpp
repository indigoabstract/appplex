#include "stdafx.h"

#include "unit-global-flight-paths.hpp"

#ifdef UNIT_GLOBAL_FLIGHT_PATHS

#include "com/mws/mws-com.hpp"
#include "com/mws/mws-camera.hpp"
#include "com/mws/mws-font.hpp"
#include "com/unit/input-ctrl.hpp"
#include "cpp-property.hpp"
#include "gfx-inc.hpp"
#include "ext/gfx-surface.hpp"
#include "utils/free-camera.hpp"
#include "tlib/rng/rng.hpp"
#include <glm/inc.hpp>


namespace global_flight_paths_ns
{
   float ANIM_SPEED = 80.f;
   float START_SIZE = 0.75f;
   float MIDDLE_SIZE = 2.5f;
   float END_SIZE = 0.75f;

   class hot_spot
   {
   public:
      hot_spot(float i_latitude, float i_longitude, glm::vec3 i_position, glm::vec3 i_normal)
      {
         latitude = i_latitude;
         longitude = i_longitude;
         position = i_position;
         normal = i_normal;
      }

      float latitude;
      float longitude;
      glm::vec3 position;
      glm::vec3 normal;
   };

   void get_hot_spot_data(float latitude, float longitude, float globe_radius, glm::vec3& position, glm::vec3& normal)
   {
      float sign_lat = 1.f;
      float sign_long = 1.f;

      if (latitude < 1.f)
      {
         sign_lat = -1.f;
      }

      if (longitude < 1.f)
      {
         sign_long = -1.f;
      }

      longitude = 180.f - glm::abs(longitude);

      float latitude_rad = glm::radians(latitude);
      float longitude_rad = glm::radians(longitude);

      glm::vec3 unit_sphere_position = glm::vec3(sign_long * glm::sin(longitude_rad) * glm::cos(latitude_rad), glm::cos(longitude_rad) * glm::cos(latitude_rad), glm::sin(latitude_rad));
      normal = glm::normalize(unit_sphere_position);
      position = normal * globe_radius;
   }

   std::shared_ptr<hot_spot> new_hot_spot(float latitude, float longitude, float globe_radius)
   {
      glm::vec3 position;
      glm::vec3 normal;

      get_hot_spot_data(latitude, longitude, globe_radius, position, normal);

      return std::make_shared<hot_spot>(latitude, longitude, position, normal);
   }

   std::shared_ptr<hot_spot> new_random_hot_spot(float globe_radius)
   {
      static RNG rng;
      float sign_lat = 1.f;
      float sign_long = 1.f;

      if (rng.percent(50))
      {
         sign_lat = -sign_lat;
      }

      if (rng.percent(50))
      {
         sign_long = -sign_long;
      }

      float latitude = sign_lat * rng.nextInt(90);
      float longitude = sign_long * rng.nextInt(180);

      return new_hot_spot(latitude, longitude, globe_radius);
   }

   glm::vec3 transform_point(const glm::vec3& point, const glm::mat4& transform)
   {
      glm::vec4 pos = transform * glm::vec4(point, 0.f);
      glm::vec3 pos_3(pos.x, pos.y, pos.z);

      return pos_3;
   }


   class hot_spot_link : public gfx_vxo
   {
   public:
      struct link_size
      {
         float pos;
         float size;
      };

      int step_count;
      std::shared_ptr<hot_spot> start_point;
      std::shared_ptr<hot_spot> end_point;
      glm::vec3 base;
      glm::vec3 base_ortho;
      std::vector<float> distances;
      std::vector<float> cumulated_distances;
      std::vector<glm::vec3> positions;
      std::vector<glm::vec3> vertices;
      std::vector<glm::vec2> tex_coord;
      std::vector<glm::vec2> vertex_index;
      std::vector<glm::vec3> segment_dir;
      std::vector<gfx_indices_type> indices;
      std::vector<link_size> lk_size;

      hot_spot_link() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord, a_v2_tex_coord_1"))
      {
      }

      float get_total_distance()
      {
         return cumulated_distances[step_count - 1];
      }

      void gen_link(std::shared_ptr<hot_spot> i_start_point, std::shared_ptr<hot_spot> i_end_point, float i_globe_radius)
      {
         start_point = i_start_point;
         end_point = i_end_point;
         base = end_point->position - start_point->position;
         lk_size =
         {
            { 0.f, START_SIZE },
         { 0.45f, MIDDLE_SIZE },
         { 0.55f, MIDDLE_SIZE },
         { 1.f, END_SIZE },
         };

         float dist = glm::length(base);
         float ratio = 0.5f * dist / i_globe_radius;
         float xfact = 4;

         step_count = int(xfact * glm::floor((1 - ratio) * 15.f + 50.f * ratio));

         positions.resize(step_count);
         distances.resize(step_count);
         cumulated_distances.resize(step_count);
         tex_coord.resize(step_count * 2);
         segment_dir.resize(step_count * 2);

         base = glm::normalize(base);
         base_ortho = (end_point->normal + start_point->normal) * 0.5f;
         base_ortho = glm::normalize(base_ortho);

         glm::vec3 org = (end_point->position + start_point->position) * 0.5f;
         float r = dist * 0.5f;
         float start_angle = (1.5f - ratio) * 45.f;
         float r2 = r / glm::cos(glm::radians(start_angle));
         float off = r2 * glm::sin(glm::radians(start_angle));

         for (int k = 0; k < step_count; k++)
         {
            float p = k / (step_count - 1.f);
            float angle = glm::mix(glm::radians(start_angle), glm::radians(180.f - start_angle), p);
            float x = r2 * glm::cos(angle);
            float y = r2 * glm::sin(angle) - off;

            positions[k] = org + base * x + base_ortho * y;
         }

         for (int k = 1; k < step_count; k++)
         {
            distances[k] = glm::distance(positions[k], positions[k - 1]);
            cumulated_distances[k] = cumulated_distances[k - 1] + distances[k];
         }

         float total_distance = cumulated_distances[step_count - 1];

         for (int i = 0; i < step_count; i++)
         {
            float iv = cumulated_distances[i] / total_distance;
            int idx1 = 2 * i + 0;
            tex_coord[idx1] = glm::vec2(-0.5f, iv);
            segment_dir[idx1] = glm::vec3(0.f);

            int idx2 = 2 * i + 1;
            tex_coord[idx2] = glm::vec2(0.5f, iv);
            segment_dir[idx2] = glm::vec3(0.f);
         }

         int indices_size = step_count - 1;
         int index = 0;

         indices.resize(indices_size * 6);

         for (int k = 0; k < indices_size; k++)
         {
            int i4 = 2 * k;
            indices[index] = i4 + 0;
            index = index + 1;
            indices[index] = i4 + 3;
            index = index + 1;
            indices[index] = i4 + 1;
            index = index + 1;
            indices[index] = i4 + 3;
            index = index + 1;
            indices[index] = i4 + 0;
            index = index + 1;
            indices[index] = i4 + 2;
            index = index + 1;
         }
      }

      void calc_vertex_positions()
      {
         float total_distance = cumulated_distances[step_count - 1];

         vertices.resize(step_count * 2);
         vertex_index.resize(step_count * 2);
         segment_dir.resize(step_count * 2);

         for (int i = 0; i < step_count - 1; i++)
         {
            float vsize = get_size_from_index(i);
            auto tvect = positions[i];
            auto seg = tvect - positions[i + 1];
            seg = glm::normalize(seg);

            int idx1 = 2 * i + 0;
            vertices[idx1] = tvect;
            vertex_index[idx1] = glm::vec2(i, vsize);
            segment_dir[idx1] = seg;

            int idx2 = 2 * i + 1;
            vertices[idx2] = tvect;
            vertex_index[idx2] = glm::vec2(i, vsize);
            segment_dir[idx2] = seg;
         }

         int i = step_count - 1;
         float iv = cumulated_distances[i] / total_distance;
         float vsize = get_size_from_index(i);
         auto tvect = positions[i];
         auto seg = positions[i - 1] - tvect;
         seg = glm::normalize(seg);

         int idx1 = 2 * i + 0;
         vertices[idx1] = tvect;
         vertex_index[idx1] = glm::vec2(i, vsize);
         segment_dir[idx1] = seg;

         int idx2 = 2 * i + 1;
         vertices[idx2] = tvect;
         vertex_index[idx2] = glm::vec2(i, vsize);
         segment_dir[idx2] = seg;

         auto& r = *this;

         r[MP_BLENDING] = MV_ADD;
         r[MP_DEPTH_FUNCTION] = MV_LESS;
         r[MP_DEPTH_TEST] = true;
         r[MP_DEPTH_WRITE] = false;
         r[MP_SHADER_NAME] = "hot-spot-lines";
         r["u_s2d_tex"] = "line.png";
         //r["u_s2d_tex"] = "trail.png";
         r[MP_CULL_BACK] = false;
         r[MP_CULL_FRONT] = false;

         struct vx_fmt_p3f_n3f_t2f_t2f
         {
            glm::vec3 pos;
            glm::vec3 nrm;
            glm::vec2 tex;
            glm::vec2 tex_1;
         };

         int vert_count = vertices.size();
         std::vector<vx_fmt_p3f_n3f_t2f_t2f> tvertices_data(vert_count);

         for (int k = 0; k < vert_count; k++)
         {
            auto& rf = tvertices_data[k];

            rf.pos = vertices[k];
            rf.nrm = segment_dir[k];
            rf.tex = tex_coord[k];
            rf.tex_1 = vertex_index[k];
         }

         gfx_vxo_util::set_mesh_data(
            (const uint8*)tvertices_data.data(), sizeof(vx_fmt_p3f_n3f_t2f_t2f) * tvertices_data.size(),
            indices.data(), indices.size() * sizeof(gfx_indices_type), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
      }

      void set_visible_vertices(int sv_idx, int ev_idx)
      {
         auto& sv = positions[sv_idx];
         auto& sseg_dir = segment_dir[2 * sv_idx];
         auto& ev = positions[ev_idx];
         auto& eseg_dir = segment_dir[2 * ev_idx];

         auto& r = *this;

         r["u_vertex_color"] = glm::vec4(0.5f, 0.5f, 1.f, 1.f);
         r["u_v4_start_vertex"] = glm::vec4(sv, sv_idx);
         r["u_v3_start_vertex_seg_dir"] = sseg_dir;
         r["u_v4_end_vertex"] = glm::vec4(ev, ev_idx);
         r["u_v3_end_vertex_seg_dir"] = eseg_dir;
      }

      void draw_link(std::shared_ptr<gfx_camera> camera, const glm::mat4& i_globe_tf)
      {
         float s = 0.05f;
         glm::vec3 bsize(s, s, s);
         auto src = camera->position() + glm::vec3(0.01f, 0, 0);

         for (int k = 0; k < step_count; k++)
         {
            auto pos = transform_point(positions[k], i_globe_tf);
            camera->draw_line(pos, src, gfx_color::colors::green.to_vec4(), 0.25f);
            camera->draw_box(pos, bsize, glm::quat(1.f, 0.f, 0.f, 0.f), gfx_color::colors::blue.to_vec4(), 0.25f);
         }
      }

      float get_size_from_index(int index)
      {
         float position = index / (step_count - 1.f);

         return get_size_from_position(position);
      }

      // 0 <= i_position <= 1
      float get_size_from_position(float i_position)
      {
         int size_length = lk_size.size();
         int start_idx = 0;
         int end_idx = 0;
         float size = 0;

         for (int i = 0; i < size_length; i++)
         {
            start_idx = i;
            end_idx = i + 1;

            if (end_idx > size_length)
            {
               end_idx = size_length;
            }

            auto& es = lk_size[start_idx];
            auto& ee = lk_size[end_idx];

            if (i_position <= ee.pos)
            {
               float e1 = es.size;
               float e2 = ee.size;
               float dif = ee.pos - es.pos;
               float iv = i_position - es.pos;

               if (dif > 0)
               {
                  iv = iv / dif;
               }

               size = e1 * (1.f - iv) + e2 * iv;
               break;
            }
         }

         return size;
      }
   };


   class hot_spot_chain
   {
   public:
      hot_spot_chain()
      {
         loop_count = 1;
         //loop_index = -1;
         loop_index = 1;
         l_show = false;
         l_makevisible = false;
      }

      uint32 last_update_time;
      float chain_distance;
      int last_link_idx;
      float current_position;
      std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;
      std::vector<float> cumulated_distances;
      bool l_makevisible;
      int loop_count;
      int loop_index;
      bool l_show;

      void add_link(std::shared_ptr<hot_spot_link> _link)
      {
         int sv_idx = 0;
         int ev_idx = _link->step_count - 1;

         _link->set_visible_vertices(sv_idx, ev_idx);
         _link->visible = false;

         hot_spot_link_list.resize(hot_spot_link_list.size() + 1);
         cumulated_distances.resize(hot_spot_link_list.size());

         hot_spot_link_list[hot_spot_link_list.size()] = _link;
         cumulated_distances[hot_spot_link_list.size() - 1] = cumulated_distances[hot_spot_link_list.size() - 2] + _link->get_total_distance();
         chain_distance = cumulated_distances[hot_spot_link_list.size() - 1];
      }

      void set_hot_spot_link_list(std::vector<std::shared_ptr<hot_spot_link> > i_hot_spot_link_list)
      {
         int hot_spot_link_list_length = i_hot_spot_link_list.size();

         last_update_time = pfm::time::get_time_millis();
         hot_spot_link_list = i_hot_spot_link_list;
         chain_distance = 0;
         last_link_idx = 0;
         current_position = 0;

         cumulated_distances.resize(hot_spot_link_list_length);
         cumulated_distances[0] = hot_spot_link_list[0]->get_total_distance();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto link = hot_spot_link_list[k];
            int sv_idx = 0;
            int ev_idx = link->step_count - 1;

            link->calc_vertex_positions();
            link->set_visible_vertices(sv_idx, ev_idx);
            link->visible = false;
         }

         for (int k = 1; k < hot_spot_link_list_length; k++)
         {
            cumulated_distances[k] = cumulated_distances[k - 1] + hot_spot_link_list[k]->get_total_distance();
         }

         chain_distance = cumulated_distances[hot_spot_link_list_length - 1];
         l_makevisible = true;
      }

      void show()
      {
         l_show = true;
         set_visible(true);
      }

      void hide()
      {
         l_show = false;
         set_visible(false);
      }

      void force_visible()
      {
         int hot_spot_link_list_length = hot_spot_link_list.size();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto lk = hot_spot_link_list[k];
            float delta_d;

            if (k > 0)
            {
               delta_d = cumulated_distances[k] - cumulated_distances[k - 1];
            }
            else
            {
               delta_d = cumulated_distances[k];
            }

            int ev_idx = get_end_vertex_idx(delta_d, k);
            hot_spot_link_list[k]->set_visible_vertices(0, ev_idx);
         }
      }

      void set_visible(bool visible)
      {
         int hot_spot_link_list_length = hot_spot_link_list.size();

         for (int k = 0; k < hot_spot_link_list_length; k++)
         {
            auto lk = hot_spot_link_list[k];

            lk->visible = visible;
         }

         l_makevisible = true;
      }

      void restart_animation_for_last(int _nb)
      {
         loop_index = 1;
         _nb = glm::clamp<int>(_nb, 0, hot_spot_link_list.size());
         last_link_idx = hot_spot_link_list.size() - _nb;
         current_position = cumulated_distances[hot_spot_link_list.size() - _nb];
         last_update_time = pfm::time::get_time_millis();
         l_makevisible = true;
      }

      void restart_animation()
      {
         loop_index = 1;
         rewind_animation();
      }

      void stop_animation()
      {
         loop_index = -1;
      }

      void rewind_animation()
      {
         int hot_spot_chain_list_length = hot_spot_link_list.size();
         auto link = hot_spot_link_list[0];

         current_position = 0;
         link->set_visible_vertices(0, 0);
         last_link_idx = 0;
         l_makevisible = true;
         last_update_time = pfm::time::get_time_millis();

         set_visible(false);
      }

      std::shared_ptr<hot_spot> get_last_hot_spot()
      {
         int idx = hot_spot_link_list.size() - 1;

         return hot_spot_link_list[idx]->start_point;
      }

      void update()
      {
         if (!l_show)
         {
            return;
         }

         if ((loop_index > 0) && (loop_count == -1 || loop_index <= loop_count))
         {
            int hot_spot_chain_list_length = hot_spot_link_list.size();
            auto link = hot_spot_link_list[last_link_idx];
            float delta_t = (pfm::time::get_time_millis() - last_update_time) / 1000.f;

            last_update_time = pfm::time::get_time_millis();
            current_position = current_position + ANIM_SPEED * delta_t;

            if (current_position > cumulated_distances[last_link_idx])
            {
               while (last_link_idx < hot_spot_chain_list_length && current_position > cumulated_distances[last_link_idx])
               {
                  last_link_idx = last_link_idx + 1;
               }

               if (last_link_idx >= hot_spot_chain_list_length)
               {
                  loop_index = loop_index + 1;

                  if (loop_count == -1 || loop_index <= loop_count)
                  {
                     // repeat animation
                     rewind_animation();
                  }
                  else
                  {
                     // end animation
                     for (int k = 0; k < hot_spot_chain_list_length; k++)
                     {
                        auto link = hot_spot_link_list[k];

                        link->set_visible_vertices(0, link->step_count - 1);
                     }
                  }
               }
               else
               {
                  float delta_d = cumulated_distances[last_link_idx] - current_position;
                  int ev_idx = get_end_vertex_idx(delta_d, last_link_idx);

                  link->set_visible_vertices(0, link->step_count - 1);
                  link = hot_spot_link_list[last_link_idx];
                  link->set_visible_vertices(0, ev_idx);
                  l_makevisible = true;
               }
            }
            else
            {
               float delta_d = current_position;

               if (last_link_idx > 0)
               {
                  delta_d = current_position - cumulated_distances[last_link_idx - 1];
               }

               int ev_idx = get_end_vertex_idx(delta_d, last_link_idx);

               link->set_visible_vertices(0, ev_idx);

               if (l_makevisible)
               {
                  l_makevisible = false;
                  link->visible = true;
               }
            }
         }
      }

      int get_end_vertex_idx(float position_on_link, int link_idx)
      {
         auto link = hot_spot_link_list[link_idx];
         float dist = link->get_total_distance();
         float pos = position_on_link / dist;
         int idx = int(glm::floor((link->step_count - 1) * pos));

         return idx;
      }

      void destroy()
      {
         for (size_t k = 0; k < hot_spot_link_list.size(); k++)
         {
            hot_spot_link_list[k]->detach();
         }

         hot_spot_link_list.clear();
      }
   };


   class hot_spot_connector
   {
   public:
      float globe_radius;
      std::vector<std::shared_ptr<hot_spot_chain> > hot_spot_chain_list;
      bool is_init;
      bool visible;
      std::shared_ptr<gfx_vxo> globe;

      hot_spot_connector(std::shared_ptr<gfx_vxo> i_globe, float i_globe_radius)
      {
         globe = i_globe;
         globe_radius = i_globe_radius;
         is_init = false;
         visible = true;
      }

      void destroy()
      {
         destroy_old_chain();
      }

      void update()
      {
         if (globe)
         {
            if (!is_init)
            {
               is_init = true;

               int hot_spot_chain_list_length = hot_spot_chain_list.size();

               for (int i = 0; i < hot_spot_chain_list_length; i++)
               {
                  auto chain = hot_spot_chain_list[i];
                  int hot_spot_chain_list_length = chain->hot_spot_link_list.size();

                  for (int j = 0; j < hot_spot_chain_list_length; j++)
                  {
                     auto link = chain->hot_spot_link_list[j];

                     link->calc_vertex_positions();
                     link->position = globe->position;
                  }
               }
            }

            int hot_spot_chain_list_length = hot_spot_chain_list.size();

            for (int i = 0; i < hot_spot_chain_list_length; i++)
            {
               auto chain = hot_spot_chain_list[i];
               int hot_spot_chain_list_length = chain->hot_spot_link_list.size();

               for (int j = 0; j < hot_spot_chain_list_length; j++)
               {
                  chain->hot_spot_link_list[j]->orientation = globe->orientation;
                  //chain->hot_spot_link_list[j]->draw_link(nullptr, globe->get_global_tf_mx());
               }

               chain->update();
            }
         }
      }

      void set_visible(bool isvisible)
      {
         visible = isvisible;

         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->set_visible(visible);
         }
      }

      bool is_visible()
      {
         return visible;
      }

      void show()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->show();
         }
      }

      void hide()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            auto chain = hot_spot_chain_list[k];
            chain->hide();
         }
      }

      void force_visible()
      {
         hot_spot_chain_list[0]->force_visible();
         set_visible(true);
      }

      void restart_animation_for_last(int _nb)
      {
         hot_spot_chain_list[0]->restart_animation_for_last(_nb);
         // show();
      }

      void restart_animation()
      {
         int hot_spot_chain_list_length = hot_spot_chain_list.size();

         for (int k = 0; k < hot_spot_chain_list_length; k++)
         {
            hot_spot_chain_list[k]->restart_animation();
         }

         //show();
      }

      void stop_animation()
      {
         int hot_spot_chain_list_length = hot_spot_chain_list.size();

         for (int k = 0; k < hot_spot_chain_list_length; k++)
         {
            hot_spot_chain_list[k]->stop_animation();
         }
      }

      void destroy_old_chain()
      {
         for (size_t k = 0; k < hot_spot_chain_list.size(); k++)
         {
            hot_spot_chain_list[k]->destroy();
         }

         hot_spot_chain_list.clear();
      }

      void set_hot_spots(std::vector<glm::vec2> globe_hot_spot_list)
      {
         std::vector<std::shared_ptr<hot_spot> > globe_hot_spot_data_list;

         for (size_t k = 0; k < globe_hot_spot_list.size(); k++)
         {
            glm::vec3 position;
            glm::vec3 normal;
            auto hs = globe_hot_spot_list[k];

            get_hot_spot_data(hs.x, hs.y, globe_radius, position, normal);

            auto hs_2 = std::make_shared<hot_spot>(hs.x, hs.y, position, normal);

            globe_hot_spot_data_list.push_back(hs_2);
         }

         is_init = false;
         destroy_old_chain();

         if (globe_hot_spot_list.empty())
         {
            RNG rng;
            int hot_spot_chain_list_length = 4 + rng.nextInt(4);
            bool default_hot_spot = false;
            auto scene = globe->get_scene();

            mws_print("hot_spot_chain_list_length: [%d]\n", hot_spot_chain_list_length);

            for (int i = 0; i < hot_spot_chain_list_length; i++)
            {
               std::vector<std::shared_ptr<hot_spot> > hot_spot_list;
               int hot_spot_list_length = 6 + rng.nextInt(5);
               std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;

               hot_spot_chain_list.push_back(std::make_shared<hot_spot_chain>());
               hot_spot_chain_list[i]->loop_count = -1;

               for (int j = 0; j < hot_spot_list_length; j++)
               {
                  auto hs = new_random_hot_spot(globe_radius);

                  hot_spot_list.push_back(hs);
               }

               if (!default_hot_spot)
               {
                  default_hot_spot = true;
                  hot_spot_list_length = hot_spot_list_length + 1;
                  hot_spot_list.push_back(new_hot_spot(44.435278f, 26.102778f, globe_radius));
               }

               for (int j = 1; j < hot_spot_list_length; j++)
               {
                  auto start_point = hot_spot_list[j - 1];
                  auto end_point = hot_spot_list[j];
                  auto link = std::make_shared<hot_spot_link>();

                  scene->attach(link);
                  link->gen_link(end_point, start_point, globe_radius);
                  hot_spot_link_list.push_back(link);
               }

               mws_print("hs idx[%d] hot_spot_list_length: [%d]\n", i, hot_spot_list_length);
               hot_spot_chain_list[i]->set_hot_spot_link_list(hot_spot_link_list);
            }
         }
         else
         {
            std::vector<std::shared_ptr<hot_spot> > hot_spot_list;
            int hot_spot_list_length = globe_hot_spot_list.size();
            std::vector<std::shared_ptr<hot_spot_link> > hot_spot_link_list;
            auto scene = globe->get_scene();
            auto hot_spot_chain_inst = std::make_shared<hot_spot_chain>();

            hot_spot_chain_inst->loop_count = -1;
            hot_spot_chain_list.push_back(hot_spot_chain_inst);

            for (int j = 0; j < hot_spot_list_length; j++)
            {
               auto point = globe_hot_spot_list[j];
               auto hs = new_hot_spot(point.x, point.y, globe_radius);

               hot_spot_list.push_back(hs);
            }

            int index = 0;

            for (int j = 1; j < hot_spot_list_length; j++)
            {
               auto start_point = hot_spot_list[index];
               auto end_point = hot_spot_list[j];
               auto link = std::make_shared<hot_spot_link>();

               scene->attach(link);
               link->gen_link(end_point, start_point, globe_radius);
               hot_spot_link_list.push_back(link);
               index = index + 1;
            }

            hot_spot_chain_list[0]->set_hot_spot_link_list(hot_spot_link_list);
         }
      }

      void add_hot_spot(glm::vec2 _hot_spot)
      {
         auto start_point = hot_spot_chain_list[0]->get_last_hot_spot();
         auto end_point = new_hot_spot(_hot_spot.x, _hot_spot.y, globe_radius);
         auto link = std::make_shared<hot_spot_link>();

         link->gen_link(end_point, start_point, globe_radius);
         hot_spot_chain_list[0]->add_link(link);
         is_init = false;
      }
   };


   class main_page : public mws_page
   {
   public:
      virtual void init()
      {
         mws_page::init();

         t = 0;
         free_cam = std::make_shared<free_camera>(get_unit());
         persp_cam = gfx_camera::nwi();
         persp_cam->camera_id = "default";
         persp_cam->rendering_priority = 0;
         persp_cam->near_clip_distance = 0.1f;
         persp_cam->far_clip_distance = 50000.f;
         persp_cam->fov_y_deg = 60.f;
         persp_cam->clear_color = true;
         persp_cam->clear_color_value = gfx_color::colors::black;
         persp_cam->clear_depth = true;
         persp_cam->position = glm::vec3(0.f, 0.f, 250.f);
         free_cam->persp_cam = persp_cam;
         free_cam->mw_speed_factor = 1.f;

         u_v3_light_dir = -glm::vec3(-1.f, 0.f, 0.5f);

         skybox = shared_ptr<gfx_box>(new gfx_box());
         gfx_box& r_cube_mesh = *skybox;
         float s = persp_cam->far_clip_distance * 0.5f;
         r_cube_mesh.set_dimensions(s, s, s);
         r_cube_mesh[MP_SHADER_NAME] = "sky-box";
         r_cube_mesh["u_scm_skybox"] = "skybx";
         r_cube_mesh[MP_CULL_BACK] = false;
         r_cube_mesh[MP_CULL_FRONT] = true;

         globe_radius = 100.f;
         vpc_rs_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
         gfx_vpc_ring_sphere& r_vpc_rs_mesh = *vpc_rs_mesh;
         r_vpc_rs_mesh.get_vx_info().uses_tangent_basis = true;
         //r_vpc_rs_mesh.render_method = GLPT_LINES;
         r_vpc_rs_mesh.set_dimensions(globe_radius, 75);
         r_vpc_rs_mesh.position = glm::vec3(0.f, 0.f, 0.f);
         r_vpc_rs_mesh[MP_SHADER_NAME] = "3d-globe";
         r_vpc_rs_mesh["u_s2d_day"] = "earth-flat-map.png";
         r_vpc_rs_mesh["u_s2d_night"] = "earth-night-flat-map.png";
         r_vpc_rs_mesh["u_s2d_clouds"] = "clouds-26-aug-2011.png";
         r_vpc_rs_mesh["u_s2d_normal_specular_map"] = "earth-normal-spec-map.png";
         r_vpc_rs_mesh["u_v3_light_dir"] = u_v3_light_dir;

         //s_mesh = shared_ptr<gfx_vpc_ring_sphere>(new gfx_vpc_ring_sphere());
         //gfx_vpc_ring_sphere& rs_mesh = *s_mesh;
         //rs_mesh.set_dimensions(100, 75);
         //rs_mesh.position = glm::vec3(0.f, 0.f, 0.f);
         //rs_mesh[MP_SHADER_NAME] = "ldo-test";
         ////rs_mesh["u_s2d_tex"] = "earth-flat-map.png";
         //rs_mesh["u_scm_skybox"] = "ocean";
         //rs_mesh["u_v3_light_dir"] = u_v3_light_dir;

         auto scene = get_unit()->gfx_scene_inst;

         scene->attach(persp_cam);
         //scene->attach(s_mesh);
         scene->attach(vpc_rs_mesh);
         scene->attach(skybox);

         persp_cam->position = glm::vec3(-100.f, 0.f, -200.f);
         free_cam->look_at_dir = -persp_cam->position();// glm::vec3(0.f, 0.25f, -1.f);
         free_cam->up_dir = glm::vec3(0.0f, 1.0f, 0.0f);
         free_cam->target_ref_point = r_vpc_rs_mesh.position;

         hot_spot_connex = std::make_shared<hot_spot_connector>(vpc_rs_mesh, globe_radius);
         std::vector<glm::vec2> hs_list;
         hot_spot_connex->set_hot_spots(hs_list);
         hot_spot_connex->show();

         mws_report_gfx_errs();
      }

      virtual void receive(shared_ptr<iadp> idp)
      {
         free_cam->update_input(idp);

         if (!idp->is_processed())
         {
            if (idp->is_type(key_evt::KEYEVT_EVT_TYPE))
            {
               shared_ptr<key_evt> ke = key_evt::as_key_evt(idp);

               if (ke->is_pressed())
               {
                  bool do_action = true;

                  switch (ke->get_key())
                  {
                  case KEY_R:
                  {
                     hot_spot_connex->restart_animation();
                     break;
                  }

                  case KEY_N:
                  {
                     std::vector<glm::vec2> hs_list;
                     hot_spot_connex->set_hot_spots(hs_list);
                     hot_spot_connex->show();
                     break;
                  }

                  default:
                     do_action = false;
                  }

                  if (do_action)
                  {
                     ke->process();
                  }
               }
            }
         }

         mws_page::receive(idp);
      }

      virtual void update_state()
      {
         mws_page::update_state();

         vpc_rs_mesh->orientation = glm::quat(glm::vec3(0, t, 0));
         t += 0.001f;

         free_cam->update();
         skybox->position = persp_cam->position;
         hot_spot_connex->update();

         //persp_cam->draw_axes(vpc_rs_mesh->position, 5 * globe_radius, 1);

         mws_report_gfx_errs();
      }

      virtual void update_view(shared_ptr<mws_camera> g)
      {
         mws_page::update_view(g);
      }

      shared_ptr<gfx_box> skybox;
      shared_ptr<gfx_vpc_ring_sphere> vpc_rs_mesh;
      float globe_radius;
      shared_ptr<gfx_vpc_ring_sphere> s_mesh;
      shared_ptr<gfx_camera> persp_cam;
      glm::vec3 u_v3_light_dir;
      float t;
      shared_ptr<free_camera> free_cam;
      std::shared_ptr<hot_spot_connector> hot_spot_connex;
   };
}
using namespace global_flight_paths_ns;


unit_global_flight_paths::unit_global_flight_paths()
{
   set_name("global-flight-paths");
}

shared_ptr<unit_global_flight_paths> unit_global_flight_paths::nwi()
{
   return shared_ptr<unit_global_flight_paths>(new unit_global_flight_paths());
}

void unit_global_flight_paths::init_mws()
{
   auto page = mws_root->new_page<main_page>();
}

#endif
