#include "stdafx.h"

#include "appplex-conf.hpp"

#ifdef MOD_JCV

#include "vrn-diag-data.hpp"


// return if a kernel or a nexus point based on i_id value
// return invalid value if i_id is outside of range
mws_vrn_diag_pt_type mws_vrn_data::get_diagram_point_type_by_id(uint32 i_id)
{
   if (i_id >= geom.kernel_points.first_idx() && i_id <= geom.kernel_points.last_idx())
   {
      return mws_vrn_diag_pt_type::kernel_point;
   }
   else if (i_id >= geom.nexus_points.first_idx() && i_id <= geom.nexus_points.last_idx())
   {
      return mws_vrn_diag_pt_type::nexus_point;
   }

   return mws_vrn_diag_pt_type::invalid_point;
}

// return a kernel or a nexus point based on id value
// return null if id is outside of range
mws_vrn_diag_pt* mws_vrn_data::get_diagram_point_by_id(uint32 i_id)
{
   if (i_id >= geom.kernel_points.first_idx() && i_id <= geom.kernel_points.last_idx())
   {
      uint32 idx = i_id - geom.kernel_points.first_idx();

      return &geom.kernel_points[idx];
   }
   else if (i_id >= geom.nexus_points.first_idx() && i_id <= geom.nexus_points.last_idx())
   {
      uint32 idx = i_id - geom.nexus_points.first_idx();

      return &geom.nexus_points[idx];
   }

   return nullptr;
}

mws_vrn_kernel_pt* mws_vrn_data::get_kernel_point_by_id(uint32 i_id)
{
   return (mws_vrn_kernel_pt*)get_diagram_point_by_id(i_id);
}

mws_vrn_nexus_pt* mws_vrn_data::get_nexus_point_by_id(uint32 i_id)
{
   return (mws_vrn_nexus_pt*)get_diagram_point_by_id(i_id);
}

#endif
