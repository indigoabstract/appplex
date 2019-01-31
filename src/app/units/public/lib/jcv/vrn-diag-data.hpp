#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_JCV

#include "pfm.hpp"
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

   virtual int get_id_at(int iidx) { return 0; }
   virtual glm::vec3& get_position_at(int iidx) = 0;
   virtual void set_position_at(const glm::vec3& iposition, int iidx) = 0;
   virtual int get_size() const = 0;
   virtual void resize(int inew_size) = 0;
   virtual int first_idx() const { return _first_idx; }
   virtual int last_idx() const { return _last_idx; }

   int _first_idx;
   int _last_idx;
};


class mws_vrn_diag_pt
{
public:
   mws_vrn_diag_pt()
   {
      id = -1;
      position = glm::vec3(0.f);
   }

   void set_position(const glm::vec3& iposition)
   {
      position = iposition;
   }

   glm::vec3 position;
   int id;
};


// a site in a voronoi diagram
class mws_vrn_kernel_pt : public mws_vrn_diag_pt
{
public:
   mws_vrn_kernel_pt()
   {
      cell = nullptr;
   }

   jcv_site* cell;
};


// a corner of a voronoi cell
class mws_vrn_nexus_pt : public mws_vrn_diag_pt
{
public:
   mws_vrn_nexus_pt()
   {
      starting_edge = nullptr;
   }

   jcv_edge* starting_edge;
};


template <class T> class mws_vrn_pt_vect : public mws_vrn_pos_vect
{
public:
   virtual int get_id_at(int iidx)
   {
      return vect[iidx].id;
   }

   virtual T& operator[](int iidx)
   {
      return vect[iidx];
   }

   virtual glm::vec3& get_position_at(int iidx)
   {
      return vect[iidx].position;
   }

   virtual void set_position_at(const glm::vec3& iposition, int iidx)
   {
      if (iidx >= (int)vect.size())
      {
         resize(iidx + 1);
      }

      vect[iidx].set_position(iposition);
   }

   virtual void resize(int inew_size)
   {
      vect.resize(inew_size);
   }

   virtual void clear()
   {
      vect.clear();
   }

   virtual int get_size() const
   {
      return vect.size();
   }

   std::vector<T> vect;
};


class mws_vrn_kernel_pt_vect : public mws_vrn_pt_vect < mws_vrn_kernel_pt >
{
public:
   virtual jcv_site* get_cell_at(int iidx)
   {
      return vect[iidx].cell;
   }

   virtual void set_cell_at(jcv_site* icell, int iidx)
   {
      if (iidx >= vect.size())
      {
         resize(iidx + 1);
      }

      vect[iidx].cell = icell;
   }
};


class mws_vrn_nexus_pt_vect : public mws_vrn_pt_vect < mws_vrn_nexus_pt >
{
public:
   virtual jcv_edge* get_edge_at(int iidx)
   {
      return vect[iidx].starting_edge;
   }

   virtual void set_edge_at(jcv_edge* iincident_edge, int iidx)
   {
      if (iidx >= vect.size())
      {
         resize(iidx + 1);
      }

      vect[iidx].starting_edge = iincident_edge;
   }
};


// a pair of nexuses
class mws_vrn_nexus_pair
{
public:
   mws_vrn_nexus_pair()
   {
      nexus0_id = nexus1_id = -1;
   }

   uint32 nexus0_id;
   uint32 nexus1_id;
};


class mws_vrn_cell_pt_id
{
public:
   mws_vrn_cell_pt_id()
   {
      point_id = id = -1;
   }

   uint32 point_id;
   uint32 id;
};


class mws_vrn_cell_pt_id_vect : public mws_vrn_pos_vect
{
public:
   mws_vrn_cell_pt_id_vect() {}

   virtual mws_vrn_cell_pt_id& operator[](int iidx)
   {
      return vect[iidx];
   }

   virtual int get_id_at(int iidx)
   {
      return vect[iidx].id;
   }

   glm::vec3& get_position_at(int iidx) override;
   void set_position_at(const glm::vec3& iposition, int iidx) override;

   virtual void set_cpi_at(const mws_vrn_cell_pt_id& icpi, int iidx)
   {
      if (iidx >= vect.size())
      {
         resize(iidx + 1);
      }

      mws_assert(icpi.id >= 0);
      mws_assert(icpi.point_id >= 0);
      vect[iidx] = icpi;
   }

   virtual void resize(int inew_size)
   {
      vect.resize(inew_size);
   }

   virtual int get_size() const
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

   virtual void set_min_size(int imin_size)
   {
      if (imin_size >= vect.size())
      {
         resize(imin_size);
      }
   }

   virtual void resize(int inew_size)
   {
      vect.resize(inew_size);
   }

   virtual int get_size() const
   {
      return vect.size();
   }

   std::vector<mws_vrn_nexus_pair> vect;
};


class mws_vrn_point_3d
{
public:
   mws_vrn_point_3d()
   {
      id = 0;
      position = glm::vec3(0.f);
   }

   glm::vec3 position;
   int id;
};


class mws_vrn_picking_vec3 : public mws_vrn_pos_vect
{
public:
   virtual mws_vrn_point_3d& operator[](int iidx)
   {
      return vect[iidx];
   }

   virtual int get_id_at(int iidx)
   {
      return vect[iidx].id;
   }

   virtual glm::vec3& get_position_at(int iidx)
   {
      return vect[iidx].position;
   }

   virtual void set_position_at(const glm::vec3& iposition, int iidx)
   {
      if (iidx >= vect.size())
      {
         resize(iidx + 1);
      }

      vect[iidx].position = iposition;
   }

   virtual void resize(int inew_size)
   {
      vect.resize(inew_size);
   }

   virtual int get_size() const
   {
      return vect.size();
   }

   std::vector<mws_vrn_point_3d> vect;
};


enum class mws_vrn_diag_pt_type
{
   e_invalid_point,
   e_kernel_point,
   e_nexus_point,
   e_triangle_point,
};


class mws_vrn_data
{
public:
   struct settings
   {
      int diag_width;
      int diag_height;
      bool delaunay_diag_visible;
      bool nexus_points_visible;
      bool voronoi_diag_visible;
      bool convex_hull_visible;
      bool kernel_points_visible;
      bool cell_triangles_visible;
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
      std::vector<int> cell_point_count;

      // lines delimiting the cells
      mws_vrn_nexus_pair_vect nexus_pairs;

      mws_vrn_picking_vec3 delaunay_diag_points;
      mws_vrn_picking_vec3 convex_hull_points;
      mws_vrn_picking_vec3 grid_points;
      // value of the last id (out of all ids. the first value is always 1 (value of picking_start_idx))
      int last_geom_id;
   };

   // return if a kernel or a nexus point based on id value
   // return invalid value if id is outside of range
   mws_vrn_diag_pt_type get_diagram_point_type_by_id(int id)
   {
      if (id >= geom.kernel_points.first_idx() && id <= geom.kernel_points.last_idx())
      {
         return mws_vrn_diag_pt_type::e_kernel_point;
      }
      else if (id >= geom.nexus_points.first_idx() && id <= geom.nexus_points.last_idx())
      {
         return mws_vrn_diag_pt_type::e_nexus_point;
      }

      return mws_vrn_diag_pt_type::e_invalid_point;
   }

   // return a kernel or a nexus point based on id value
   // return null if id is outside of range
   mws_vrn_diag_pt* get_diagram_point_by_id(int id)
   {
      if (id >= geom.kernel_points.first_idx() && id <= geom.kernel_points.last_idx())
      {
         int idx = id - geom.kernel_points.first_idx();

         return &geom.kernel_points[idx];
      }
      else if (id >= geom.nexus_points.first_idx() && id <= geom.nexus_points.last_idx())
      {
         int idx = id - geom.nexus_points.first_idx();

         return &geom.nexus_points[idx];
      }

      return nullptr;
   }

   mws_vrn_kernel_pt* get_kernel_point_by_id(int id)
   {
      return (mws_vrn_kernel_pt*)get_diagram_point_by_id(id);
   }

   mws_vrn_nexus_pt* get_nexus_point_by_id(int id)
   {
      return (mws_vrn_nexus_pt*)get_diagram_point_by_id(id);
   }

   settings info;
   mws_vrn_geom_data geom;
};


inline glm::vec3& mws_vrn_cell_pt_id_vect::get_position_at(int iidx)
{
   return vdata.lock()->get_diagram_point_by_id(vect[iidx].point_id)->position;
}

inline void mws_vrn_cell_pt_id_vect::set_position_at(const glm::vec3& iposition, int iidx)
{
   if (iidx >= vect.size())
   {
      resize(iidx + 1);
   }

   vdata.lock()->get_diagram_point_by_id(vect[iidx].point_id)->position = iposition;
}

#endif
