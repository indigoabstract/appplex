#pragma once

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "pfm.hpp"
#include <glm/inc.hpp>


class vkb_voronoi_data;


class vkb_position_vector
{
public:
   vkb_position_vector()
   {
      _first_idx = _last_idx = 0;
   }

   virtual int get_id_at(int iidx) { return 0; }
   virtual glm::vec3& get_position_at(int iidx) = 0;
   virtual void set_position_at(const glm::vec3& iposition, int iidx) = 0;
   virtual int size() const = 0;
   virtual void resize(int inew_size) = 0;
   virtual int first_idx() const { return _first_idx; }
   virtual int last_idx() const { return _last_idx; }

   int _first_idx;
   int _last_idx;
};


class vkb_diagram_point
{
public:
   void set_position(const glm::vec3& iposition)
   {
      position = iposition;
   }

   glm::vec3 position = glm::vec3(0.f);
   int id = -1;
};


template <class T> class vkb_map_point_vect : public vkb_position_vector
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

   virtual int size() const
   {
      return vect.size();
   }

   std::vector < T > vect;
};


class vkb_point_vect : public vkb_map_point_vect < vkb_diagram_point >
{
public:
};


// a pair of nexuses
class vkb_nexus_pair
{
public:
   uint32 nexus0_id = -1;
   uint32 nexus1_id = -1;
};


class vkb_cell_point_id
{
public:
   uint32 point_id = -1;
};


class vkb_cell_point_id_vector
{
public:
   vkb_cell_point_id_vector() {}

   virtual vkb_cell_point_id& operator[](int iidx)
   {
      return vect[iidx];
   }

   glm::vec3& get_position_at(int iidx);
   void set_position_at(const glm::vec3& iposition, int iidx);

   virtual void set_cpi_at(const vkb_cell_point_id& icpi, int iidx)
   {
      if (iidx >= (int)vect.size())
      {
         resize(iidx + 1);
      }

      mws_assert(icpi.point_id >= 0);
      vect[iidx] = icpi;
   }

   virtual void resize(int inew_size)
   {
      vect.resize(inew_size);
   }

   virtual int size() const
   {
      return vect.size();
   }

   std::vector<vkb_cell_point_id> vect;
   mws_wp<vkb_voronoi_data> vdata;
};


class vkb_point_3d
{
public:
   glm::vec3 position = glm::vec3(0.f);
   int id = -1;
};


enum vkb_diagram_point_type
{
   e_invalid_point,
   e_kernel_point,
   e_nexus_point,
   e_triangle_point,
};


class vkb_voronoi_data
{
public:
   struct vkb_settings
   {
      int diag_width = 0;
      int diag_height = 0;
      bool nexus_points_visible = true;
      bool nexus_pairs_visible = true;
      bool kernel_points_visible = true;
      bool cell_triangles_visible = true;
   };

   struct vkb_voronoi_geometry_data
   {
      // centers of all the cells in the map
      vkb_point_vect kernel_points;
      // corners for all the cells in the map
      vkb_point_vect nexus_points;

      // corners for each cell (for building triangles)
      //picking_vec3 cell_points;
      vkb_cell_point_id_vector cell_points_ids;
      std::vector<int> cell_point_count;

      // lines delimiting the cells
      std::vector<vkb_nexus_pair> nexus_pairs;

      // value of the last id (out of all ids. the first value is always 1 (value of picking_start_idx))
      int last_geom_id = -1;
   };

   // return if a kernel or a nexus point based on id value
   // return invalid value if id is outside of range
   vkb_diagram_point_type get_vkb_diagram_point_type_by_id(int id)
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
   vkb_diagram_point* get_vkb_diagram_point_by_id(int id)
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

   vkb_diagram_point* get_kernel_point_by_id(int id)
   {
      return (vkb_diagram_point*)get_vkb_diagram_point_by_id(id);
   }

   vkb_diagram_point* get_nexus_point_by_id(int id)
   {
      return get_vkb_diagram_point_by_id(id);
   }

   vkb_settings info;
   vkb_voronoi_geometry_data geom;
};


inline glm::vec3& vkb_cell_point_id_vector::get_position_at(int iidx)
{
   return vdata.lock()->get_vkb_diagram_point_by_id(vect[iidx].point_id)->position;
}

inline void vkb_cell_point_id_vector::set_position_at(const glm::vec3& iposition, int iidx)
{
   if (iidx >= (int)vect.size())
   {
      resize(iidx + 1);
   }

   vdata.lock()->get_vkb_diagram_point_by_id(vect[iidx].point_id)->position = iposition;
}

#endif
