#pragma once

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "pfm.hpp"
#include <glm/inc.hpp>


class voronoi_data;


class position_vector
{
public:
   position_vector()
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


class diagram_point
{
public:
   diagram_point()
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
class kernel_point : public diagram_point
{
public:
   kernel_point()
   {
   }
};


// a corner of a voronoi cell
class nexus_point : public diagram_point
{
public:
   nexus_point()
   {
   }
};


template <class T> class map_point_vect : public position_vector
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
      if (iidx >= vect.size())
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

   std::vector < T > vect;
};


class kernel_point_vect : public map_point_vect < kernel_point >
{
public:
};


class nexus_point_vect : public map_point_vect < nexus_point >
{
public:
};


// a pair of nexuses
class nexus_pair
{
public:
   nexus_pair()
   {
      nexus0_id = nexus1_id = -1;
   }

   uint32 nexus0_id;
   uint32 nexus1_id;
};


class cell_point_id
{
public:
   cell_point_id()
   {
      point_id = id = -1;
   }

   uint32 point_id;
   uint32 id;
};


class cell_point_id_vector : public position_vector
{
public:
   cell_point_id_vector() {}

   virtual cell_point_id& operator[](int iidx)
   {
      return vect[iidx];
   }

   virtual int get_id_at(int iidx)
   {
      return vect[iidx].id;
   }

   glm::vec3& get_position_at(int iidx) override;
   void set_position_at(const glm::vec3& iposition, int iidx) override;

   virtual void set_cpi_at(const cell_point_id& icpi, int iidx)
   {
      if (iidx >= vect.size())
      {
         resize(iidx + 1);
      }

      mws_assert(icpi.id > 0);
      mws_assert(icpi.point_id > 0);
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

   std::vector<cell_point_id> vect;
   weak_ptr<voronoi_data> vdata;
};


class nexus_pair_vect
{
public:
   nexus_pair_vect()
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

   std::vector<nexus_pair> vect;
};


class point_3d
{
public:
   point_3d()
   {
      id = 0;
      position = glm::vec3(0.f);
   }

   glm::vec3 position;
   int id;
};


enum diagram_point_type
{
   e_invalid_point,
   e_kernel_point,
   e_nexus_point,
   e_triangle_point,
};


class voronoi_data
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

   struct voronoi_geometry_data
   {
      // centers of all the cells in the map
      kernel_point_vect kernel_points;
      // corners for all the cells in the map
      nexus_point_vect nexus_points;

      // corners for each cell (for building triangles)
      //picking_vec3 cell_points;
      cell_point_id_vector cell_points_ids;
      std::vector<int> cell_point_count;

      // lines delimiting the cells
      nexus_pair_vect nexus_pair_points;

      // value of the last id (out of all ids. the first value is always 1 (value of picking_start_idx))
      int last_geom_id;
   };

   // return if a kernel or a nexus point based on id value
   // return invalid value if id is outside of range
   diagram_point_type get_diagram_point_type_by_id(int id)
   {
      if (id >= geom.kernel_points.first_idx() && id <= geom.kernel_points.last_idx())
      {
         return e_kernel_point;
      }
      else if (id >= geom.nexus_points.first_idx() && id <= geom.nexus_points.last_idx())
      {
         return e_nexus_point;
      }

      return e_invalid_point;
   }

   // return a kernel or a nexus point based on id value
   // return null if id is outside of range
   diagram_point* get_diagram_point_by_id(int id)
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

   kernel_point* get_kernel_point_by_id(int id)
   {
      return (kernel_point*)get_diagram_point_by_id(id);
   }

   nexus_point* get_nexus_point_by_id(int id)
   {
      return (nexus_point*)get_diagram_point_by_id(id);
   }

   settings info;
   voronoi_geometry_data geom;
};


inline glm::vec3& cell_point_id_vector::get_position_at(int iidx)
{
   return vdata.lock()->get_diagram_point_by_id(vect[iidx].point_id)->position;
}

inline void cell_point_id_vector::set_position_at(const glm::vec3& iposition, int iidx)
{
   if (iidx >= vect.size())
   {
      resize(iidx + 1);
   }

   vdata.lock()->get_diagram_point_by_id(vect[iidx].point_id)->position = iposition;
}

#endif
