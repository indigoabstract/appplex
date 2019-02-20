#include "stdafx.hxx"

#include "mws-vkb-visual.hxx"

void mws_vkb_gen::update_diag()
{
   voronoi_diag_impl->update_data();
}


void mws_vkb_main::set_kernel_points(std::vector<glm::vec2> i_kernel_points)
{
   std::vector<float> vx, vy;

   vx.resize(i_kernel_points.size());
   vy.resize(i_kernel_points.size());

   for (uint32 k = 0; k < i_kernel_points.size(); k++)
   {
      vx[k] = i_kernel_points[k].x;
      vy[k] = i_kernel_points[k].y;
   }

   vgen->voronoi_diag_impl->init_data(diag_data, vx, vy);
   update_diag();
}

mws_vrn_diag::idx_dist mws_vkb_main::get_kernel_idx_at(float i_x, float i_y) const
{
   return vgen->voronoi_diag_impl->get_kernel_idx_at(i_x, i_y);
}

void mws_vkb_main::move_kernel_to(uint32 i_idx, float i_x, float i_y)
{
   vgen->voronoi_diag_impl->move_kernel_to(i_idx, i_x, i_y);
   update_geometry();
}

void mws_vkb_main::insert_kernel_at(float i_x, float i_y)
{
   vgen->voronoi_diag_impl->insert_kernel_at(i_x, i_y);
   update_geometry();
}

void mws_vkb_main::remove_kernel(uint32 i_idx)
{
   vgen->voronoi_diag_impl->remove_kernel(i_idx);
   update_geometry();
}
