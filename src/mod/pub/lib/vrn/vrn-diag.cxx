#include "stdafx.hxx"

#include "vrn-diag.hxx"
#include "vrn-diag-data.hxx"
#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

const uint32_t picking_start_idx = 0;


class voronoi_2d_diag_impl : public mws_vrn_diag
{
public:
   virtual void init_data(mws_sp<mws_vrn_data> i_vdata, std::vector<float>& vx, std::vector<float>& vy) override
   {
      jcv_clipper* clipper = nullptr;
      vrn_data = i_vdata;
      clip_dim = { jcv_point {0.f, 0.f}, jcv_point{ (float)i_vdata->info.diag_width, (float)i_vdata->info.diag_height } };
      full_clear();

      for (uint32_t k = 0; k < vx.size(); ++k)
      {
         jcv_point p = { vx[k], vy[k] };
         kernels_list.push_back(p);
      }

      memset(&diagram, 0, sizeof(jcv_diagram));
      jcv_diagram_generate(kernels_list.size(), kernels_list.data(), &clip_dim, clipper, &diagram);
   }

   virtual const mws_sp<mws_vrn_data> get_data() const override
   {
      return vrn_data.lock();
   }

   virtual idx_dist get_kernel_idx_at(float i_x, float i_y) const override
   {
      float min_dist = JCV_FLT_MAX;
      idx_dist ret = { -1, min_dist };
      glm::vec2 ref_pt(i_x, i_y);
      uint32_t size = kernels_list.size();

      for (uint32_t k = 0; k < size; k++)
      {
         auto& kp = kernels_list[k];
         glm::vec2 kp_pt(kp.x, kp.y);
         float dist = glm::distance2(ref_pt, kp_pt);

         if (dist < min_dist)
         {
            min_dist = dist;
            ret.idx = k;
         }
      }

      if (ret.idx >= 0)
      {
         ret.dist = glm::sqrt(min_dist);
      }

      return ret;
   }

   virtual glm::vec2 get_kernel_at(uint32_t i_idx) const override
   {
      mws_assert(i_idx < kernels_list.size());
      const jcv_point& ker = kernels_list[i_idx];
      return glm::vec2(ker.x, ker.y);
   }

   virtual void update_data() override
   {
      mws_sp<mws_vrn_data> vdata = vrn_data.lock();
      mws_vrn_data::settings& s = vdata->info;
      mws_vrn_data::mws_vrn_geom_data& g = vdata->geom;
      uint32_t current_picking_start_idx = picking_start_idx;

      {
         mws_vrn_kernel_pt_vect& p = g.kernel_points;

         p._first_idx = current_picking_start_idx;
         update_global_kernel_points(vdata);
         p._last_idx = p._first_idx + p.size() - 1;
         current_picking_start_idx = p._last_idx + 1;
      }

      {
         mws_vrn_nexus_pt_vect& p = g.nexus_points;

         p._first_idx = current_picking_start_idx;
         update_global_nexus_points(vdata);
         p._last_idx = p._first_idx + p.size() - 1;
         current_picking_start_idx = p._last_idx + 1;
      }

      {
         mws_vrn_cell_pt_id_vect& p = g.cell_points_ids;

         p._first_idx = current_picking_start_idx;
         update_per_cell_corner_points(vdata);
         p._last_idx = p._first_idx + g.cell_point_count.size() - 1;
         current_picking_start_idx = p._last_idx + 1;
         g.last_geom_id = p._last_idx;
      }
   }

   virtual void move_kernel_to(uint32_t i_idx, float i_x, float i_y) override
   {
      jcv_clipper* clipper = nullptr;
      auto& kp = kernels_list[i_idx];

      partial_clear();
      kp.x = i_x;
      kp.y = i_y;
      memset(&diagram, 0, sizeof(jcv_diagram));
      jcv_diagram_generate(kernels_list.size(), kernels_list.data(), &clip_dim, clipper, &diagram);
      update_data();
   }

   virtual void insert_kernel_at(float i_x, float i_y) override
   {
      jcv_clipper* clipper = nullptr;
      jcv_point np{ i_x, i_y };

      partial_clear();
      kernels_list.push_back(np);
      memset(&diagram, 0, sizeof(jcv_diagram));
      jcv_diagram_generate(kernels_list.size(), kernels_list.data(), &clip_dim, clipper, &diagram);
      update_data();
   }

   virtual void remove_kernel(uint32_t i_idx) override
   {
      jcv_clipper* clipper = nullptr;

      partial_clear();
      kernels_list.erase(kernels_list.begin() + i_idx);
      memset(&diagram, 0, sizeof(jcv_diagram));
      jcv_diagram_generate(kernels_list.size(), kernels_list.data(), &clip_dim, clipper, &diagram);
      update_data();
   }

   void partial_clear()
   {
      nexus_id_map.clear();
      kernel_id_map.clear();

      if (diagram.internal)
      {
         jcv_diagram_free(&diagram);
      }
   }

   void full_clear()
   {
      partial_clear();
      kernels_list.clear();
   }

   // update a list of original input points
   void update_global_kernel_points(mws_sp<mws_vrn_data> i_vdata)
   {
      i_vdata->geom.kernel_points.resize(kernels_list.size());

      for (uint32_t k = 0; k < kernels_list.size(); k++)
      {
         auto& p = kernels_list[k];
         glm::vec3 pos(p.x, p.y, 0.f);
         //i_vdata->geom.kernel_points.set_cell_at(&cell, idx);

         i_vdata->geom.kernel_points.set_position_at(pos, k);
         i_vdata->geom.kernel_points.vect[k].id = i_vdata->geom.kernel_points.first_idx() + k;
      }

      int size = i_vdata->geom.kernel_points.size();

      for (int k = 0; k < size; k++)
      {
         mws_vrn_kernel_pt& p = i_vdata->geom.kernel_points.vect[k];
         uint64_t key = get_key(p.position);

         mws_assert(p.id >= 0);
         kernel_id_map[key] = p.id;
      }
   }

   // update a list of corner/wall intersection points for the entire diagram
   void update_global_nexus_points(mws_sp<mws_vrn_data> i_vdata)
   {
      int edge_count = 0;

      i_vdata->geom.nexus_points.clear();

      const jcv_edge* edge = jcv_diagram_get_edges(&diagram);

      while (edge)
      {
         glm::vec3 pos(edge->pos[0].x, edge->pos[0].y, 0.f);

         i_vdata->geom.nexus_points.set_position_at(pos, edge_count);
         i_vdata->geom.nexus_points.set_edge_at((jcv_edge*)edge, edge_count);
         i_vdata->geom.nexus_points.vect[edge_count].id = i_vdata->geom.nexus_points.first_idx() + edge_count;
         edge_count++;

         //draw_line(edge->pos[0], edge->pos[1]);
         edge = jcv_diagram_get_next_edge(edge);
      }

      i_vdata->geom.nexus_points.resize(edge_count);

      int size = i_vdata->geom.nexus_points.size();

      for (int k = 0; k < size; k++)
      {
         mws_vrn_nexus_pt& p = i_vdata->geom.nexus_points.vect[k];
         uint64_t key = get_key(p.position);

         mws_assert(p.id > 0);
         nexus_id_map[key] = p.id;
      }

      update_global_nexus_pair_points(i_vdata, edge_count);
   }

   // update as a list of pair points the list of segments linking the corner/wall intersection points for the entire diagram
   // used for drawing the lines between nexuses
   void update_global_nexus_pair_points(mws_sp<mws_vrn_data> i_vdata, int i_edge_count)
   {
      int idx = 0;
      const jcv_edge* edge = jcv_diagram_get_edges(&diagram);

      i_vdata->geom.nexus_pairs.resize(i_edge_count);

      while (edge)
      {
         i_vdata->geom.nexus_pairs.set_min_size(idx + 1);

         mws_vrn_nexus_pair & pair = i_vdata->geom.nexus_pairs.vect[idx];

         glm::vec3 position0(edge->pos[0].x, edge->pos[0].y, 0.f);
         glm::vec3 position1(edge->pos[1].x, edge->pos[1].y, 0.f);

         uint64_t key0 = get_key(position0);
         uint64_t key1 = get_key(position1);
         bool p0_found = (nexus_id_map.find(key0) != nexus_id_map.end());
         bool p1_found = (nexus_id_map.find(key1) != nexus_id_map.end());
         uint32_t p0_id = -1;
         uint32_t p1_id = -1;

         if (!p0_found)
         {
            int k = i_vdata->geom.nexus_points.size();
            //edge_type& edge = (edge_type&)*it;

            i_vdata->geom.nexus_points.resize(k + 1);
            i_vdata->geom.nexus_points.set_position_at(position0, k);
            //i_vdata->geom.nexus_points.set_edge_at(&edge, k);
            p0_id = i_vdata->geom.nexus_points.vect[k].id = i_vdata->geom.nexus_points.first_idx() + k;
            nexus_id_map[key0] = p0_id;
         }
         else
         {
            p0_id = nexus_id_map[key0];
         }

         if (!p1_found)
         {
            int k = i_vdata->geom.nexus_points.size();
            //edge_type& edge = (edge_type&)*it;

            i_vdata->geom.nexus_points.resize(k + 1);
            i_vdata->geom.nexus_points.set_position_at(position1, k);
            //i_vdata->geom.nexus_points.set_edge_at(&edge, k);
            p1_id = i_vdata->geom.nexus_points.vect[k].id = i_vdata->geom.nexus_points.first_idx() + k;
            nexus_id_map[key1] = p1_id;
         }
         else
         {
            p1_id = nexus_id_map[key1];
         }

         mws_assert(p0_id > 0);
         mws_assert(p1_id > 0);
         pair.nexus0_id = p0_id;
         pair.nexus1_id = p1_id;
         idx++;
         edge = jcv_diagram_get_next_edge(edge);
      }

      i_vdata->geom.nexus_pairs.resize(idx);
   }

   // update a list of corner/wall intersection points for each cell in the diagram and a list counting the number of corner points for each cell
   void update_per_cell_corner_points(mws_sp<mws_vrn_data> i_vdata)
   {
      i_vdata->geom.cell_point_count.clear();
      int k = 0;
      const jcv_site* sites = jcv_diagram_get_sites(&diagram);

      for (int i = 0; i < diagram.numsites; ++i)
      {
         const jcv_site* site = &sites[i];
         int idx = site->index;
         i_vdata->geom.kernel_points.set_cell_at((jcv_site*)site, idx);
         jcv_point& vertex = kernels_list[idx];
         //edge_type* first_edge = (edge_type*)it->incident_edge();
         //edge_type* edge = first_edge;
         int point_count = 1;

         glm::vec3 kp_pos = glm::vec3(vertex.x, vertex.y, 0.f);

         uint64_t kp_key = get_key(kp_pos);
         bool kp_key_found = (kernel_id_map.find(kp_key) != kernel_id_map.end());
         mws_assert(kp_key_found);

         {
            mws_vrn_cell_pt_id cpi;

            cpi.point_id = kernel_id_map[kp_key];
            mws_assert(cpi.point_id >= 0);
            cpi.id = i_vdata->geom.cell_points_ids.first_idx() + k;
            i_vdata->geom.cell_points_ids.set_cpi_at(cpi, k);
         }

         k++;
         const jcv_graphedge* e = site->edges;

         while (e)
         {
            //vertex_type* start_vertex = edge->vertex0();

            //if (start_vertex)
            //{
            glm::vec3 np_pos = glm::vec3(e->pos[0].x, e->pos[0].y, 0.f);

            uint64_t np_key = get_key(np_pos);
            bool np_key_found = (nexus_id_map.find(np_key) != nexus_id_map.end());
            mws_assert(np_key_found);
            mws_vrn_cell_pt_id cpi;

            cpi.point_id = nexus_id_map[np_key];
            cpi.id = i_vdata->geom.cell_points_ids.first_idx() + k;
            i_vdata->geom.cell_points_ids.set_cpi_at(cpi, k);
            k++;
            //}
            //else
            //{
            //   cell_point_id cpi;

            //   cpi.point_id = kernel_id_map[kp_key];
            //   cpi.id = i_vdata->geom.cell_points_ids.first_idx() + k;
            //   i_vdata->geom.cell_points_ids.set_cpi_at(cpi, k);
            //   k++;
            //}

            //edge = edge->next();
            point_count++;
            e = e->next;
         }// while (edge != first_edge);

         i_vdata->geom.cell_point_count.push_back(point_count);
      }

      i_vdata->geom.cell_points_ids.resize(k);
   }

   static uint64_t get_key(glm::vec3 & iposition)
   {
      int* x = (int*)& iposition.x;
      int* y = (int*)& iposition.y;
      uint64_t x64 = *x & 0xffffffff;
      uint64_t y64 = *y & 0xffffffff;
      uint64_t r = (x64 << 32) | y64;

      return r;
   }

   std::vector<jcv_point> kernels_list;
   std::unordered_map<uint64_t, uint32_t> nexus_id_map;
   std::unordered_map<uint64_t, uint32_t> kernel_id_map;
   jcv_diagram diagram;
   jcv_rect clip_dim;
   mws_wp<mws_vrn_data> vrn_data;
};


mws_sp<mws_vrn_diag> mws_vrn_diag::nwi()
{
   return mws_sp<mws_vrn_diag>(new voronoi_2d_diag_impl());
}
