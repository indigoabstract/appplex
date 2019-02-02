#pragma once

#include "appplex-conf.hxx"

#ifdef MOD_JCV

#include "pfm.hxx"
#include "jc_voronoi.h"
#include <glm/inc.hpp>


class mws_vrn_data;


class mws_vrn_pos_vect
{
public:
   mws_vrn_pos_vect()
   {
      _first_idx = _last_idx = 0;
   }

   virtual uint32 get_id_at(uint32 i_idx) { return 0; }
   virtual glm::vec3& get_position_at(uint32 i_idx) = 0;
   virtual void set_position_at(const glm::vec3& iposition, uint32 i_idx) = 0;
   virtual uint32 size() const = 0;
   virtual void resize(uint32 i_new_size) = 0;
   virtual uint32 first_idx() const { return _first_idx; }
   virtual uint32 last_idx() const { return _last_idx; }

   uint32 _first_idx;
   uint32 _last_idx;
};


class mws_vrn_diag_pt
{
public:
   void set_position(const glm::vec3& iposition)
   {
      position = iposition;
   }

   glm::vec3 position = glm::vec3(0.f);
   uint32 id = -1;
};


// a site in a voronoi diagram
class mws_vrn_kernel_pt : public mws_vrn_diag_pt
{
public:
   jcv_site* cell = nullptr;
};


// a corner of a voronoi cell
class mws_vrn_nexus_pt : public mws_vrn_diag_pt
{
public:
   jcv_edge* starting_edge = nullptr;
};


template <class T> class mws_vrn_pt_vect : public mws_vrn_pos_vect
{
public:
   virtual uint32 get_id_at(uint32 i_idx)
   {
      return vect[i_idx].id;
   }

   virtual T& operator[](uint32 i_idx)
   {
      return vect[i_idx];
   }

   virtual glm::vec3& get_position_at(uint32 i_idx)
   {
      return vect[i_idx].position;
   }

   virtual void set_position_at(const glm::vec3& iposition, uint32 i_idx)
   {
      if (i_idx >= vect.size())
      {
         resize(i_idx + 1);
      }

      vect[i_idx].set_position(iposition);
   }

   virtual void resize(uint32 i_new_size)
   {
      vect.resize(i_new_size);
   }

   virtual void clear()
   {
      vect.clear();
   }

   virtual uint32 size() const
   {
      return vect.size();
   }

   std::vector<T> vect;
};


class mws_vrn_kernel_pt_vect : public mws_vrn_pt_vect < mws_vrn_kernel_pt >
{
public:
   virtual jcv_site* get_cell_at(uint32 i_idx)
   {
      return vect[i_idx].cell;
   }

   virtual void set_cell_at(jcv_site* icell, uint32 i_idx)
   {
      if (i_idx >= vect.size())
      {
         resize(i_idx + 1);
      }

      vect[i_idx].cell = icell;
   }
};


class mws_vrn_nexus_pt_vect : public mws_vrn_pt_vect < mws_vrn_nexus_pt >
{
public:
   virtual jcv_edge* get_edge_at(uint32 i_idx)
   {
      return vect[i_idx].starting_edge;
   }

   virtual void set_edge_at(jcv_edge* iincident_edge, uint32 i_idx)
   {
      if (i_idx >= vect.size())
      {
         resize(i_idx + 1);
      }

      vect[i_idx].starting_edge = iincident_edge;
   }
};


// a pair of nexuses
class mws_vrn_nexus_pair
{
public:
   uint32 nexus0_id = -1;
   uint32 nexus1_id = -1;
};


class mws_vrn_cell_pt_id
{
public:
   uint32 point_id = -1;
   uint32 id = -1;
};


class mws_vrn_cell_pt_id_vect : public mws_vrn_pos_vect
{
public:
   mws_vrn_cell_pt_id_vect() {}

   virtual mws_vrn_cell_pt_id& operator[](uint32 i_idx)
   {
      return vect[i_idx];
   }

   virtual uint32 get_id_at(uint32 i_idx)
   {
      return vect[i_idx].id;
   }

   glm::vec3& get_position_at(uint32 i_idx) override;
   void set_position_at(const glm::vec3& iposition, uint32 i_idx) override;

   virtual void set_cpi_at(const mws_vrn_cell_pt_id& icpi, uint32 i_idx)
   {
      if (i_idx >= vect.size())
      {
         resize(i_idx + 1);
      }

      mws_assert(icpi.id >= 0);
      mws_assert(icpi.point_id >= 0);
      vect[i_idx] = icpi;
   }

   virtual void resize(uint32 i_new_size) override
   {
      vect.resize(i_new_size);
   }

   virtual uint32 size() const
   {
      return vect.size();
   }

   std::vector<mws_vrn_cell_pt_id> vect;
   mws_wp<mws_vrn_data> vdata;
};


class mws_vrn_nexus_pair_vect
{
public:
   mws_vrn_nexus_pair_vect()
   {
   }

   virtual void set_min_size(uint32 imin_size)
   {
      if (imin_size >= vect.size())
      {
         resize(imin_size);
      }
   }

   virtual void resize(uint32 i_new_size)
   {
      vect.resize(i_new_size);
   }

   virtual uint32 size() const
   {
      return vect.size();
   }

   std::vector<mws_vrn_nexus_pair> vect;
};


class mws_vrn_point_3d
{
public:
   glm::vec3 position = glm::vec3(0.f);
   uint32 id = 0;
};


class mws_vrn_picking_vec3 : public mws_vrn_pos_vect
{
public:
   virtual mws_vrn_point_3d& operator[](uint32 i_idx)
   {
      return vect[i_idx];
   }

   virtual uint32 get_id_at(uint32 i_idx)
   {
      return vect[i_idx].id;
   }

   virtual glm::vec3& get_position_at(uint32 i_idx)
   {
      return vect[i_idx].position;
   }

   virtual void set_position_at(const glm::vec3& iposition, uint32 i_idx)
   {
      if (i_idx >= vect.size())
      {
         resize(i_idx + 1);
      }

      vect[i_idx].position = iposition;
   }

   virtual void resize(uint32 i_new_size)
   {
      vect.resize(i_new_size);
   }

   virtual uint32 size() const
   {
      return vect.size();
   }

   std::vector<mws_vrn_point_3d> vect;
};


enum class mws_vrn_diag_pt_type
{
   invalid_point,
   kernel_point,
   nexus_point,
   triangle_point,
};


class mws_vrn_data
{
public:
   struct settings
   {
      uint32 diag_width = 0;
      uint32 diag_height = 0;
      bool kernel_points_visible = false;
      bool nexus_points_visible = false;
      bool nexus_pairs_visible = false;
      bool cell_triangles_visible = false;
      bool convex_hull_visible = false;
      bool delaunay_diag_visible = false;
   };

   struct mws_vrn_geom_data
   {
      // centers of all the cells in the map
      mws_vrn_kernel_pt_vect kernel_points;
      // corners for all the cells in the map
      mws_vrn_nexus_pt_vect nexus_points;

      // corners for each cell (for building triangles)
      //picking_vec3 cell_points;
      mws_vrn_cell_pt_id_vect cell_points_ids;
      std::vector<uint32> cell_point_count;

      // lines delimiting the cells
      mws_vrn_nexus_pair_vect nexus_pairs;

      mws_vrn_picking_vec3 delaunay_diag_points;
      mws_vrn_picking_vec3 convex_hull_points;
      mws_vrn_picking_vec3 grid_points;
      // value of the last id (out of all ids. the first value is always 1 (value of picking_start_idx))
      uint32 last_geom_id;
   };

   // return if a kernel or a nexus point based on i_id value
   // return invalid value if i_id is outside of range
   mws_vrn_diag_pt_type get_diagram_point_type_by_id(uint32 i_id);
   // return a kernel or a nexus point based on id value
   // return null if id is outside of range
   mws_vrn_diag_pt* get_diagram_point_by_id(uint32 i_id);
   mws_vrn_kernel_pt* get_kernel_point_by_id(uint32 i_id);
   mws_vrn_nexus_pt* get_nexus_point_by_id(uint32 i_id);

   settings info;
   mws_vrn_geom_data geom;
};


inline glm::vec3& mws_vrn_cell_pt_id_vect::get_position_at(uint32 i_idx)
{
   return vdata.lock()->get_diagram_point_by_id(vect[i_idx].point_id)->position;
}

inline void mws_vrn_cell_pt_id_vect::set_position_at(const glm::vec3& iposition, uint32 i_idx)
{
   if (i_idx >= vect.size())
   {
      resize(i_idx + 1);
   }

   vdata.lock()->get_diagram_point_by_id(vect[i_idx].point_id)->position = iposition;
}

#endif
