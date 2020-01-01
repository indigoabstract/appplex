#include "stdafx.hxx"

#include "vrn-visual.hxx"


// mws_vrn_gen
mws_sp<mws_vrn_gen> mws_vrn_gen::nwi(mws_sp<mws_vrn_data> i_diag_data)
{
   mws_sp<mws_vrn_gen> inst(new mws_vrn_gen());

   inst->init(i_diag_data);

   return inst;
}

void mws_vrn_gen::update_diag()
{
   voronoi_diag_impl->update_data();
}

void mws_vrn_gen::clear_points()
{
   vx.clear();
   vy.clear();
   diag_data_state = mws_vrn_diag_data_change_type::diag_no_change;
}

void mws_vrn_gen::random_points()
{
   RNG rng;
   //rng.setSeed(12345);
   int max_extra_groups = 2;
   int max_extra_group_points = 3;
   int max_random_points = 5;
   int group_count = 2 + rng.nextInt(max_extra_groups);
   int random_point_count = 5 + rng.nextInt(max_random_points);
   int hw = diag_data->info.diag_width;
   int hh = diag_data->info.diag_height;
   int x_inf = 50;
   int x_sup = hw - 50;
   int y_inf = 50;
   int y_sup = hh - 50;

   clear_points();

   for (int k = 0; k < group_count; k++)
   {
      int group_point_count = 3 + rng.nextInt(max_extra_group_points);

      for (int l = 0; l < group_point_count; l++)
      {
         int x = rng.range(x_inf, x_sup);
         int y = rng.range(y_inf, y_sup);

         vx.push_back((float)x);
         vy.push_back((float)y);
      }
   }

   for (int k = 0; k < random_point_count; k++)
   {
      int x = rng.range(x_inf, x_sup);
      int y = rng.range(y_inf, y_sup);

      vx.push_back((float)x);
      vy.push_back((float)y);
   }

   voronoi_diag_impl->init_data(diag_data, vx, vy);
}

mws_vrn_gen::mws_vrn_gen() {}

void mws_vrn_gen::init(mws_sp<mws_vrn_data> i_diag_data)
{
   diag_data = i_diag_data;
   voronoi_diag_impl = mws_vrn_diag::nwi();
   diag_data_state = mws_vrn_diag_data_change_type::diag_no_change;

   //random_points();
}


// mws_vrn_cell_vxo
mws_sp<mws_vrn_cell_vxo> mws_vrn_cell_vxo::nwi()
{
   return mws_sp<mws_vrn_cell_vxo>(new mws_vrn_cell_vxo());
}

mws_vrn_cell_vxo::mws_vrn_cell_vxo() : gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord")) {}


// mws_vrn_cell_borders
mws_sp<mws_vrn_cell_borders> mws_vrn_cell_borders::nwi() { return mws_sp<mws_vrn_cell_borders>(new mws_vrn_cell_borders()); }

uint32 mws_vrn_cell_borders::get_cell_borders_mesh_size() const { return cell_borders_mesh_vect.size(); }

mws_sp<mws_vrn_cell_vxo> mws_vrn_cell_borders::get_cell_borders_mesh_at(uint32 i_idx) const { return cell_borders_mesh_vect[i_idx]; }

void mws_vrn_cell_borders::set_cell_borders_tex(mws_sp<gfx_tex> i_tex) { tex = i_tex; }

glm::vec4 mws_vrn_cell_borders::calc_2d_bounding_box(const std::vector<glm::vec3>& i_point_list)
{
   auto x_min_max = std::minmax_element(i_point_list.begin(), i_point_list.end(),
      [](const glm::vec3& i_p0, const glm::vec3& i_p1) {return i_p0.x < i_p1.x; });
   auto y_min_max = std::minmax_element(i_point_list.begin(), i_point_list.end(),
      [](const glm::vec3& i_p0, const glm::vec3& i_p1) { return i_p0.y < i_p1.y; });

   glm::vec2 upper_left(x_min_max.first->x, y_min_max.first->y);
   glm::vec2 lower_right(x_min_max.second->x, y_min_max.second->y);

   return glm::vec4(upper_left, lower_right);
}

void mws_vrn_cell_borders::set_geometry(mws_sp<mws_vrn_data> i_diag_data, mws_vrn_cell_pt_id_vect& i_point_list, const std::vector<uint32>& i_point_count_list)
{
   const float line_half_thickness = std::max(pfm::screen::get_width(), pfm::screen::get_height()) * .035f;
   const float z_pos = 0.f;
   std::vector<glm::vec3> cell_nexus_list;
   uint32 cell_count = i_point_count_list.size();
   std::vector<mws_sp<mws_vrn_cell_vxo>> borders_mesh_vect(cell_count);

   for (mws_sp<mws_vrn_cell_vxo> border : cell_borders_mesh_vect)
   {
      border->detach();
   }

   cell_borders_mesh_vect.clear();

   for (uint32 k = 0; k < cell_count; k++)
   {
      uint32 cell_nexus_count = i_point_count_list[k] - 1;
      mws_sp<mws_vrn_cell_vxo> border = mws_vrn_cell_vxo::nwi();
      uint32 vx_count = cell_nexus_count * 4;
      uint32 ix_count = cell_nexus_count * 6;
      mws_vrn_cell_vxo& rvxo = *border;

      rvxo.set_keep_geometry_data(true);
      rvxo.set_size(vx_count, ix_count);
      rvxo.camera_id_list = { "mws_cam" };
      rvxo.name = "voronoi_cell_borders";
      rvxo.visible = false;
      rvxo[MP_SHADER_NAME] = vkb_cell_borders_sh;
      rvxo["u_v1_transparency"] = 1.f;
      rvxo["u_s2d_tex"][MP_TEXTURE_INST] = tex;
      rvxo[MP_DEPTH_TEST] = false;
      rvxo[MP_DEPTH_WRITE] = false;
      rvxo[MP_CULL_FRONT] = false;
      rvxo[MP_CULL_BACK] = false;
      rvxo[MP_BLENDING] = MV_ADD_COLOR;
      rvxo.position = glm::vec3(0, 0, 0.1f);
      borders_mesh_vect[k] = border;
      attach(border);
   }

   // loop through each cell and build its contour
   for (uint32 k = 0, idx = 0; k < cell_count; k++)
   {
      mws_sp<mws_vrn_cell_vxo> mesh = borders_mesh_vect[k];
      std::vector<gfx_indices_type>& ks_indices_data = mesh->get_ix_buffer();
      vx_fmt_3f_2f* ks_vertices_data = (vx_fmt_3f_2f*)begin_ptr(mesh->get_vx_buffer());
      uint32 cell_vx_count = i_point_count_list[k];

      // build a temp list with the cell's nexus points and with the same first and last point
      glm::vec3 kernel_pos = i_point_list.get_position_at(idx);
      glm::vec2 kernel_pos_2d = glm::vec2(kernel_pos.x, kernel_pos.y);
      mesh->kernel_pos = kernel_pos_2d;
      mesh->nexus_pos_vect.reserve(cell_vx_count - 1);
      cell_nexus_list.clear();

      // we only need nexus points to build the contour and since the first vertex is always the kernel, skip it
      idx++;
      for (uint32 l = 1; l < cell_vx_count; l++)
      {
         const glm::vec3& p = i_point_list.get_position_at(idx);
         mesh->nexus_pos_vect.push_back(p);
         cell_nexus_list.push_back(p);
         idx++;
      }

      mesh->bounding_box = calc_2d_bounding_box(cell_nexus_list);
      // push first nexus at the back of the list, so we can easily wrap around the direction calculations
      cell_nexus_list.push_back(cell_nexus_list[0]);

      uint32 cell_nexus_list_count = cell_nexus_list.size();
      std::vector<glm::vec3> inside_border_points;
      std::vector<glm::vec3> outside_border_points;

      for (glm::vec3& nexus : cell_nexus_list)
      {
         glm::vec2 nexus_2d(nexus.x, nexus.y);
         glm::vec2 kernel_nexus_dir = nexus_2d - kernel_pos_2d;
         kernel_nexus_dir = glm::normalize(kernel_nexus_dir);
         float segment_length = glm::distance(nexus_2d, kernel_pos_2d);
         glm::vec2 inside_point(nexus_2d - kernel_nexus_dir * segment_length * 4.f / 5.f);
         glm::vec2 outside_point(nexus_2d + kernel_nexus_dir * segment_length * 4.f / 5.f);

         inside_border_points.push_back(glm::vec3(inside_point, 0.f));
         outside_border_points.push_back(glm::vec3(outside_point, 0.f));
      }

      // build the contour as a list of rectangles
      for (uint32 i = 1; i < cell_nexus_list_count; i++)
      {
         uint32 ix_idx = 6 * (i - 1);
         uint32 vx_idx = 4 * (i - 1);

         vx_fmt_3f_2f vx0 = { inside_border_points[i - 1], glm::vec2(1.f, 0.f) };
         vx0.pos.z = z_pos;
         ks_vertices_data[vx_idx + 0] = vx0;

         vx_fmt_3f_2f vx1 = { outside_border_points[i - 1], glm::vec2(0.f, 0.f) };
         vx1.pos.z = z_pos;
         ks_vertices_data[vx_idx + 1] = vx1;

         vx_fmt_3f_2f vx2 = { outside_border_points[i], glm::vec2(0.f, 1.f) };
         vx2.pos.z = z_pos;
         ks_vertices_data[vx_idx + 2] = vx2;

         vx_fmt_3f_2f vx3 = { inside_border_points[i], glm::vec2(1.f, 1.f) };
         vx3.pos.z = z_pos;
         ks_vertices_data[vx_idx + 3] = vx3;

         ks_indices_data[ix_idx + 0] = vx_idx + 0;
         ks_indices_data[ix_idx + 1] = vx_idx + 2;
         ks_indices_data[ix_idx + 2] = vx_idx + 1;
         ks_indices_data[ix_idx + 3] = vx_idx + 2;
         ks_indices_data[ix_idx + 4] = vx_idx + 0;
         ks_indices_data[ix_idx + 5] = vx_idx + 3;
      }

      mesh->update_data();
   }

   // rearrange the order
   cell_borders_mesh_vect.resize(cell_count);

   for (uint32 k = 0, kernel_list_idx = 0; k < cell_count; k++)
   {
      uint32 kernel_idx = i_point_list[kernel_list_idx].point_id;
      cell_borders_mesh_vect[kernel_idx] = borders_mesh_vect[k];
      kernel_list_idx += i_point_count_list[k];
   }
}

mws_vrn_cell_borders::mws_vrn_cell_borders() : gfx_node(gfx::i()) {}


// mws_vrn_geom
mws_sp<mws_vrn_geom> mws_vrn_geom::nwi(mws_sp<mws_vrn_data> i_diag_data, mws_sp<gfx_camera> i_cam)
{
   mws_sp<mws_vrn_geom> inst(new mws_vrn_geom(i_diag_data));

   inst->init(i_cam);

   return inst;
}

void mws_vrn_geom::update_geometry(mws_sp<mws_vrn_data> i_diag_data)
{
   mws_sp<mws_vrn_data> dd = i_diag_data;

   if (dd->info.kernel_points_visible)
   {
      set_points_geometry(dd->geom.kernel_points, voronoi_kernels_mesh);
   }

   if (dd->info.nexus_points_visible)
   {
      set_points_geometry(dd->geom.nexus_points, voronoi_nexus_mesh);
   }

   if (dd->info.delaunay_diag_visible)
   {
      //set_line_geometry(dd->geom.delaunay_diag_points, delaunay_diag_mesh);
   }

   if (dd->info.nexus_pairs_visible)
   {
      set_line_geometry(dd->geom.nexus_pairs, nexus_pairs_mesh);
   }

   if (dd->info.convex_hull_visible)
   {
      //set_line_geometry(dd->geom.convex_hull_points, convex_hull_mesh);
   }

   if (dd->info.cell_triangles_visible)
   {
      set_triangle_geometry(dd->geom.cell_points_ids, dd->geom.cell_point_count, voronoi_cell_faces_mesh);
   }

   if (dd->info.cell_borders_visible)
   {
      set_cell_borders_geom(dd, dd->geom.cell_points_ids, dd->geom.cell_point_count, cell_borders);
   }
}

void mws_vrn_geom::set_line_geometry(mws_vrn_nexus_pair_vect& i_line_points, mws_sp<gfx_vxo> i_mesh)
{
   uint32 list_size = i_line_points.size();
   i_mesh->set_size(list_size * 4, list_size * 6);
   std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
   std::vector<vx_fmt_3f_4f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4f_4b_2f_1i>*) & i_mesh->get_vx_buffer());

   glm::vec3 pos;
   glm::vec4 nrm;
   gfx_color color0;
   gfx_color color1;
   gfx_uint id = 0;

   struct line_setup
   {
      static void setup(mws_sp<mws_vrn_data> diag_data, mws_vrn_nexus_pair& inp, mws_vrn_nexus_pt& ip, float& i_size, gfx_color& ic)
      {
         mws_vrn_nexus_pt* p0 = diag_data->get_nexus_point_by_id(inp.nexus0_id);
         mws_vrn_nexus_pt* p1 = diag_data->get_nexus_point_by_id(inp.nexus1_id);
         ic.r = 0; ic.g = 0; ic.b = 255; ic.a = 255;
         i_size = 1.f;
      }
   };

   glm::vec2 tex0 = { -0.5f, -0.5f };
   glm::vec2 tex1 = { +0.5f, -0.5f };
   glm::vec2 tex2 = { -0.5f, +0.5f };
   glm::vec2 tex3 = { +0.5f, +0.5f };

   for (uint32 k = 0; k < list_size; k++)
   {
      mws_vrn_nexus_pair& p = i_line_points.vect[k];
      mws_vrn_nexus_pt* np0 = diag_data->get_nexus_point_by_id(p.nexus0_id);
      mws_vrn_nexus_pt* np1 = diag_data->get_nexus_point_by_id(p.nexus1_id);
      glm::vec3& p0 = np0->position;
      glm::vec3& p1 = np1->position;
      glm::vec3 seg_dir = glm::normalize(p1 - p0);
      float line_thickness = 1.f;
      float line_thickness2 = 1.f;

      line_setup::setup(diag_data, p, *np0, line_thickness, color0);
      line_setup::setup(diag_data, p, *np1, line_thickness2, color1);

      pos = p0;
      nrm = glm::vec4(seg_dir, -0.5f * line_thickness);
      vx_fmt_3f_4f_4b_2f_1i vx0 = { pos, nrm, color0, tex0, id };
      ks_vertices_data[4 * k] = vx0;

      pos = p0;
      nrm = glm::vec4(seg_dir, +0.5f * line_thickness);
      vx_fmt_3f_4f_4b_2f_1i vx1 = { pos, nrm, color0, tex1, id };
      ks_vertices_data[4 * k + 1] = vx1;

      pos = p1;
      nrm = glm::vec4(seg_dir, -0.5f * line_thickness2);
      vx_fmt_3f_4f_4b_2f_1i vx2 = { pos, nrm, color1, tex2, id };
      ks_vertices_data[4 * k + 2] = vx2;

      pos = p1;
      nrm = glm::vec4(seg_dir, +0.5f * line_thickness2);
      vx_fmt_3f_4f_4b_2f_1i vx3 = { pos, nrm, color1, tex3, id };
      ks_vertices_data[4 * k + 3] = vx3;
   }

   uint32 ind_size = ks_vertices_data.size() / 4;

   for (uint32 k = 0; k < ind_size; k++)
   {
      uint32 i_off = 4 * k;
      ks_indices_data[6 * k + 0] = i_off + 1;
      ks_indices_data[6 * k + 1] = i_off + 2;
      ks_indices_data[6 * k + 2] = i_off + 0;
      ks_indices_data[6 * k + 3] = i_off + 2;
      ks_indices_data[6 * k + 4] = i_off + 1;
      ks_indices_data[6 * k + 5] = i_off + 3;
   }

   i_mesh->update_data();
}

void mws_vrn_geom::set_points_geometry(mws_vrn_pos_vect& i_point_list, mws_sp<gfx_vxo> i_mesh)
{
   struct vx_fmt_p3f_t2f_1i
   {
      glm::vec3 pos;
      glm::vec2 tex;
      gfx_uint id;
   };

   glm::vec3 pos;
   glm::vec2 tex;
   uint32 list_size = i_point_list.size();
   i_mesh->set_size(list_size * 4, list_size * 6);
   std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
   std::vector<vx_fmt_p3f_t2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_p3f_t2f_1i>*) & i_mesh->get_vx_buffer());

   for (uint32 k = 0; k < list_size; k++)
   {
      const glm::vec3& p = i_point_list.get_position_at(k);
      gfx_uint id = i_point_list.get_id_at(k);

      pos = p;
      tex = glm::vec2(-0.5f, -0.5f);
      vx_fmt_p3f_t2f_1i vx0 = { pos, tex, id };
      ks_vertices_data[4 * k + 0] = vx0;

      pos = p;
      tex = glm::vec2(-0.5f, +0.5f);
      vx_fmt_p3f_t2f_1i vx1 = { pos, tex, id };
      ks_vertices_data[4 * k + 1] = vx1;

      pos = p;
      tex = glm::vec2(+0.5f, -0.5f);
      vx_fmt_p3f_t2f_1i vx2 = { pos, tex, id };
      ks_vertices_data[4 * k + 2] = vx2;

      pos = p;
      tex = glm::vec2(+0.5f, +0.5f);
      vx_fmt_p3f_t2f_1i vx3 = { pos, tex, id };
      ks_vertices_data[4 * k + 3] = vx3;
   }

   uint32 ind_size = ks_vertices_data.size() / 4;

   for (uint32 k = 0; k < ind_size; k++)
   {
      uint32 i_off = 4 * k;
      ks_indices_data[6 * k + 0] = i_off + 1;
      ks_indices_data[6 * k + 1] = i_off + 2;
      ks_indices_data[6 * k + 2] = i_off + 0;
      ks_indices_data[6 * k + 3] = i_off + 2;
      ks_indices_data[6 * k + 4] = i_off + 1;
      ks_indices_data[6 * k + 5] = i_off + 3;
   }

   i_mesh->update_data();
}

void mws_vrn_geom::set_triangle_geometry(mws_vrn_pos_vect& i_point_list, const std::vector<uint32>& i_point_count_list, mws_sp<gfx_vxo> i_mesh)
{
   struct vx_fmt_3f_4b_2f_1i
   {
      glm::vec3 pos;
      gfx_color clr;
      glm::vec2 tex;
      gfx_uint id;
   };

   glm::vec3 pos;
   gfx_color clr;
   glm::vec2 tex;
   uint32 list_size = i_point_count_list.size();
   uint32 vx_count = 0;
   uint32 ix_count = 0;

   for (uint32 k = 0; k < list_size; k++)
   {
      uint32 l2_size = i_point_count_list[k];
      uint32 ind_size = l2_size - 1;

      vx_count += l2_size;
      ix_count += ind_size * 3;
   }

   i_mesh->set_size(vx_count, ix_count);
   std::vector<gfx_indices_type>& ks_indices_data = i_mesh->get_ix_buffer();
   //std::vector<vx_fmt_3f_4b_2f_1i>& ks_vertices_data = *((std::vector<vx_fmt_3f_4b_2f_1i>*)&i_mesh->get_vx_buffer());
   vx_fmt_3f_4b_2f_1i* ks_vertices_data = (vx_fmt_3f_4b_2f_1i*)begin_ptr(i_mesh->get_vx_buffer());
   int idx = 0;
   int vx_idx = 0;
   int ix_idx = 0;

   for (uint32 k = 0; k < list_size; k++)
   {
      gfx_uint id = i_point_list.get_id_at(k);
      uint32 l2_size = i_point_count_list[k];
      int i_idx = idx;

      clr.r = 0;
      clr.g = 0;
      clr.b = 0;
      clr.a = 255;

      for (uint32 l = 0; l < l2_size; l++)
      {
         const glm::vec3& p = i_point_list.get_position_at(idx);

         pos = p;
         tex = glm::vec2(-0.5f, -0.5f);
         vx_fmt_3f_4b_2f_1i vx0 = { pos, clr, tex, id };
         ks_vertices_data[vx_idx++] = vx0;

         idx++;
      }

      uint32 ind_size = l2_size - 1;

      for (uint32 i = 0; i < ind_size; i++)
      {
         int i1 = i_idx + i + 1;
         int i2 = i + 2;

         if (i2 % l2_size == 0)
         {
            i2 = 1;
         }

         i2 += i_idx;
         ks_indices_data[ix_idx++] = i_idx;
         ks_indices_data[ix_idx++] = i1;
         ks_indices_data[ix_idx++] = i2;
      }
   }

   i_mesh->update_data();
}

void mws_vrn_geom::set_cell_borders_geom(mws_sp<mws_vrn_data> i_diag_data, mws_vrn_cell_pt_id_vect& i_point_list, const std::vector<uint32>& i_point_count_list, mws_sp<mws_vrn_cell_borders> i_mesh)
{
   i_mesh->set_geometry(i_diag_data, i_point_list, i_point_count_list);
}

mws_sp<mws_vrn_cell_borders> mws_vrn_geom::get_cell_borders()
{
   return cell_borders;
}

void mws_vrn_geom::set_cell_borders(mws_sp<mws_vrn_cell_borders> i_cell_borders)
{
   cell_borders = i_cell_borders;
}

void mws_vrn_geom::make_obj_visible(uint32 i_obj_type)
{
   mws_vrn_data::settings& s = diag_data->info;
   bool is_visible = false;

   switch (i_obj_type)
   {
   case mws_vrn_obj_types::kernel_points:
      break;

   case mws_vrn_obj_types::nexus_points:
      break;

   case mws_vrn_obj_types::nexus_pairs:
      break;

   case mws_vrn_obj_types::cells:
      break;

   case mws_vrn_obj_types::convex_hull:
      break;

   case mws_vrn_obj_types::delaunay_diag:
      break;

   case mws_vrn_obj_types::cell_borders:
   {
      if (cell_borders)
      {
         cell_borders->visible = true;
      }
      break;
   }

   default:
      mws_assert(false);
   }
}

mws_vrn_geom::mws_vrn_geom(mws_sp<mws_vrn_data> i_diag_data) : gfx_node(gfx::i()) { diag_data = i_diag_data; }

void mws_vrn_geom::init(mws_sp<gfx_camera> i_cam)
{
   float alpha_val = 1.f;
   float line_thickness = std::max(pfm::screen::get_width(), pfm::screen::get_height()) * 0.009f;

   // cache the shaders, to prevent recompiling
   init_shaders();

   // kernel points
   {
      voronoi_kernels_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_kernels_mesh->name = "voronoi_kernels_mesh";
      voronoi_kernels_mesh->visible = diag_data->info.kernel_points_visible;
      (*voronoi_kernels_mesh)[MP_SHADER_NAME] = vkb_point_sh;
      (*voronoi_kernels_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_kernels_mesh)[MP_CULL_BACK] = false;
      (*voronoi_kernels_mesh)["u_v1_point_size"] = 5.f;
      (*voronoi_kernels_mesh)["u_v4_color"] = glm::vec4(0.f, 1.f, 1.f, alpha_val);
      (*voronoi_kernels_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_kernels_mesh->camera_id_list = { i_cam->camera_id() };
   }
   // nexus points
   {
      voronoi_nexus_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord, a_iv1_id")));
      voronoi_nexus_mesh->name = "voronoi_nexus_mesh";
      voronoi_nexus_mesh->visible = diag_data->info.nexus_points_visible;
      (*voronoi_nexus_mesh)[MP_SHADER_NAME] = vkb_point_sh;
      (*voronoi_nexus_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*voronoi_nexus_mesh)[MP_CULL_BACK] = false;
      (*voronoi_nexus_mesh)["u_v1_point_size"] = 5.5f;
      (*voronoi_nexus_mesh)["u_v4_color"] = glm::vec4(1.f, 0.f, 0.f, alpha_val);
      (*voronoi_nexus_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_nexus_mesh->camera_id_list = { i_cam->camera_id() };
   }
   // nexus pairs
   {
      nexus_pairs_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      nexus_pairs_mesh->name = "nexus_pairs_mesh";
      nexus_pairs_mesh->visible = diag_data->info.nexus_pairs_visible;
      (*nexus_pairs_mesh)[MP_SHADER_NAME] = vkb_line_sh;
      (*nexus_pairs_mesh)["u_v1_line_thickness"] = line_thickness;
      (*nexus_pairs_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 1.f, alpha_val);
      (*nexus_pairs_mesh)[MP_CULL_FRONT] = false;
      (*nexus_pairs_mesh)[MP_CULL_BACK] = false;
      (*nexus_pairs_mesh)[MP_DEPTH_TEST] = false;
      (*nexus_pairs_mesh)[MP_BLENDING] = MV_ALPHA;
      //nexus_pairs_mesh->render_method = GLPT_POINTS;
      nexus_pairs_mesh->camera_id_list = { i_cam->camera_id() };
      nexus_pairs_mesh->position = glm::vec3(0, 0, -0.1f);
   }
   // cell faces
   {
      voronoi_cell_faces_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      voronoi_cell_faces_mesh->name = "voronoi_cell_faces_mesh";
      voronoi_cell_faces_mesh->visible = diag_data->info.cell_triangles_visible;
      (*voronoi_cell_faces_mesh)[MP_SHADER_NAME] = vkb_triangle_sh;
      (*voronoi_cell_faces_mesh)["u_v1_point_size"] = 6.f;
      (*voronoi_cell_faces_mesh)["u_v4_color"] = glm::vec4(0.f, 0.f, 0.f, alpha_val);
      (*voronoi_cell_faces_mesh)[MP_CULL_FRONT] = false;
      (*voronoi_cell_faces_mesh)[MP_CULL_BACK] = false;
      (*voronoi_cell_faces_mesh)[MP_BLENDING] = MV_ALPHA;
      voronoi_cell_faces_mesh->camera_id_list = { i_cam->camera_id() };
   }
   // delaunay
   {
      delaunay_diag_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      delaunay_diag_mesh->name = "delaunay_diag_mesh";
      delaunay_diag_mesh->visible = diag_data->info.delaunay_diag_visible;
      (*delaunay_diag_mesh)[MP_SHADER_NAME] = vkb_line_sh;
      (*delaunay_diag_mesh)["u_v1_line_thickness"] = line_thickness;
      delaunay_diag_mesh->camera_id_list = { i_cam->camera_id() };
   }
   // convex hull
   {
      convex_hull_mesh = mws_sp<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v4_seg_data, a_iv4_color, a_v2_tex_coord, a_iv1_id")));
      convex_hull_mesh->name = "convex_hull_mesh";
      convex_hull_mesh->visible = diag_data->info.convex_hull_visible;
      (*convex_hull_mesh)[MP_SHADER_NAME] = vkb_line_sh;
      (*convex_hull_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*convex_hull_mesh)["u_v1_line_thickness"] = line_thickness;
      convex_hull_mesh->camera_id_list = { i_cam->camera_id() };
   }
   // cell borders
   {
      cell_borders = mws_vrn_cell_borders::nwi();
      cell_borders->visible = diag_data->info.cell_borders_visible;
      cell_borders->position += glm::vec3(0, 0, 1.f);
   }
   // attach them into the scene
   {
      name = "vrn-geom";
      attach(delaunay_diag_mesh);
      attach(nexus_pairs_mesh);
      attach(convex_hull_mesh);
      attach(voronoi_kernels_mesh);
      attach(voronoi_nexus_mesh);
      attach(voronoi_cell_faces_mesh);
      attach(cell_borders);
   }
   {
      mws_sp<gfx_plane> quad_mesh(new gfx_plane());
      quad_mesh->name = "quad_mesh";
      gfx_plane& r_quad_mesh = *quad_mesh;
      //r_quad_mesh.camera_id_list.clear();
      //r_quad_mesh.camera_id_list.push_back(persp_cam->camera_id());
      r_quad_mesh[MP_SHADER_NAME] = gfx::c_o_sh_id;
      r_quad_mesh["u_v4_color"] = glm::vec4(0, 0.05f, 0, 1);
      r_quad_mesh[MP_CULL_BACK] = false;
      r_quad_mesh.set_dimensions((float)diag_data->info.diag_width, (float)diag_data->info.diag_height);
      r_quad_mesh.position = glm::vec3(0, 0, -0.5f);
      quad_mesh->camera_id_list = { i_cam->camera_id() };
      //attach(quad_mesh);
   }
}

void mws_vrn_geom::init_shaders()
{
   // cell borders shader
   vkb_cell_borders_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_cell_borders_sh, true);
   if (!vkb_cell_borders_shader)
   {
      auto vsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         layout (location = 0) in vec3 a_v3_position;
         layout (location = 1) in vec2 a_v2_tex_coord;

         uniform mat4 u_m4_model_view_proj;

         smooth out vec2 v_v2_tex_coord;

         void main()
         {
	         v_v2_tex_coord = a_v2_tex_coord;
	         vec4 v4_position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
	
	         gl_Position = v4_position;
         }
         )"
         );

      auto fsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         #ifdef GL_ES
	         precision lowp float;
         #endif

         layout(location = 0) out vec4 v4_frag_color;

         uniform float u_v1_transparency;
         uniform sampler2D u_s2d_tex;

         smooth in vec2 v_v2_tex_coord;

         void main()
         {
	         vec4 v4_diff_color = texture(u_s2d_tex, v_v2_tex_coord);
	         v4_frag_color = v4_diff_color * u_v1_transparency;
         }
         )"
         );

      vkb_cell_borders_shader = gfx::i()->shader.new_program_from_src(vkb_cell_borders_sh, vsh, fsh);
   }

   // line shader
   vkb_line_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_line_sh, true);
   if (!vkb_line_shader)
   {
      auto vsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         layout(location = 0) in vec3 a_v3_position;
         layout(location = 1) in vec4 a_v4_seg_data;
         layout(location = 2) in vec4 a_iv4_color;
         layout(location = 3) in vec2 a_v2_tex_coord;

         uniform mat4 u_m4_model_view;
         uniform mat4 u_m4_model_view_proj;
         uniform mat4 u_m4_projection;

         uniform float u_v1_line_thickness;

         flat out vec4 v_v4_color;
         smooth out vec2 v_v2_tex_coord;

         void main()
         {
	         vec3 v3_cam_dir = normalize(u_m4_model_view[3].xyz);
	         vec3 v3_position = (u_m4_model_view * vec4(a_v3_position, 1.0)).xyz;
	         vec3 v3_direction = (u_m4_model_view * vec4(a_v4_seg_data.xyz, 0.0)).xyz;
	         vec3 vect = cross(v3_cam_dir, v3_direction);
	         float vsize = a_v4_seg_data.w * u_v1_line_thickness;
	
	         v3_position = v3_position + vect * vsize;
	         v_v4_color = a_iv4_color;
	         v_v2_tex_coord = a_v2_tex_coord;
	
	         vec4 v4_position = u_m4_projection * vec4(v3_position, 1.0);
	
	         gl_Position = v4_position;
         }
         )"
         );

      auto fsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         #ifdef GL_ES
	         precision highp float;
         #endif

         layout(location = 0) out vec4 v4_frag_color;

         uniform vec4 u_v4_color;

         flat in vec4 v_v4_color;
         in vec2 v_v2_tex_coord;

         void main()
         {
	         v4_frag_color = u_v4_color;
         }
         )"
         );

      vkb_line_shader = gfx::i()->shader.new_program_from_src(vkb_line_sh, vsh, fsh);
   }

   // point shader
   vkb_point_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_point_sh, true);
   if (!vkb_point_shader)
   {
      auto vsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         layout(location = 0) in vec3 a_v3_position;
         layout(location = 1) in vec2 a_v2_tex_coord;

         uniform mat4 u_m4_model_view;
         uniform mat4 u_m4_model_view_proj;
         uniform mat4 u_m4_projection;

         uniform float u_v1_point_size;

         smooth out vec2 v_v2_tex_coord;

         void main()
         {
	         float vsize = u_v1_point_size;
	         vec3 v3_position = (u_m4_model_view * vec4(a_v3_position, 1.0)).xyz;
	
	         v_v2_tex_coord = a_v2_tex_coord;
	         v3_position = v3_position + vec3(1., 0., 0.) * vsize * a_v2_tex_coord.x;
	         v3_position = v3_position + vec3(0., 1., 0.) * vsize * a_v2_tex_coord.y;
	
	         gl_Position = u_m4_projection * vec4(v3_position, 1.0);
         }
         )"
         );

      auto fsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         #ifdef GL_ES
	         precision highp float;	
         #endif

         layout(location = 0) out vec4 v4_frag_color;

         uniform vec4 u_v4_color;

         in vec2 v_v2_tex_coord;

         void main()
         {
	         v4_frag_color = u_v4_color;
         }
         )"
         );

      vkb_point_shader = gfx::i()->shader.new_program_from_src(vkb_point_sh, vsh, fsh);
   }

   // triangle shader
   vkb_triangle_shader = gfx::i()->shader.nwi_inex_by_shader_root_name(vkb_triangle_sh, true);
   if (!vkb_triangle_shader)
   {
      auto vsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         layout (location = 0) in vec3 a_v3_position;
         layout (location = 1) in vec4 a_iv4_color;
         layout (location = 2) in vec2 a_v2_tex_coord;

         uniform mat4 u_m4_model;
         uniform mat4 u_m4_model_view_proj;
         uniform mat4 u_m4_projection;
         uniform mat4 u_m4_view_inv;

         smooth out vec2 v_v2_tex_coord;
         smooth out vec4 v_v4_color;
         smooth out vec3 v_v3_pos_ms;
         smooth out vec3 v_v3_pos_ws;
         smooth out vec3 v_v3_cam_dir_ws;

         void main()
         {
	         v_v2_tex_coord = a_v2_tex_coord;
	         v_v4_color = a_iv4_color;
	         v_v3_pos_ms = a_v3_position;
	         v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	         v_v3_cam_dir_ws = normalize(v_v3_pos_ws - u_m4_view_inv[3].xyz);
	
	         gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
         }
         )"
         );

      auto fsh = std::make_shared<std::string>(
         R"(
         //@es #version 300 es
         //@dt #version 330 core

         #ifdef GL_ES
	         precision lowp float;
         #endif

         layout(location = 0) out vec4 v4_frag_color;

         uniform vec4 u_v4_color;

         void main()
         {
	         v4_frag_color = u_v4_color;
         }
         )"
         );

      vkb_triangle_shader = gfx::i()->shader.new_program_from_src(vkb_triangle_sh, vsh, fsh);
   }
}


// mws_vrn_main
mws_vrn_main::~mws_vrn_main()
{
   if (vgeom->get_parent())
   {
      vgeom->detach();
   }
}

mws_sp<mws_vrn_main> mws_vrn_main::nwi(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam)
{
   mws_sp<mws_vrn_main> inst(new mws_vrn_main());
   inst->setup(i_diag_width, i_diag_height, i_cam);
   return inst;
}

void mws_vrn_main::init()
{
   mws_assert(!vgen);
   vgen = mws_vrn_gen::nwi(diag_data);
   vgeom = mws_vrn_geom::nwi(diag_data, cam.lock());
}

void mws_vrn_main::toggle_voronoi_object(uint32 i_obj_type_mask)
{
   bool visibility_changed = false;
   mws_vrn_data::settings& s = diag_data->info;

   for (uint32 k = 0; k < (uint32)mws_vrn_obj_types::obj_count; k++)
   {
      uint32 idx = (1 << k);
      uint32 obj_type = uint32(idx & i_obj_type_mask);
      mws_sp<gfx_node> node;
      bool is_visible = false;
      visibility_changed = true;

      switch (obj_type)
      {
      case mws_vrn_obj_types::none:
         break;

      case mws_vrn_obj_types::kernel_points:
         is_visible = s.kernel_points_visible = !s.kernel_points_visible;
         node = (vgeom) ? vgeom->voronoi_kernels_mesh : nullptr;
         break;

      case mws_vrn_obj_types::nexus_points:
         is_visible = s.nexus_points_visible = !s.nexus_points_visible;
         node = (vgeom) ? vgeom->voronoi_nexus_mesh : nullptr;
         break;

      case mws_vrn_obj_types::nexus_pairs:
         is_visible = s.nexus_pairs_visible = !s.nexus_pairs_visible;
         node = (vgeom) ? vgeom->nexus_pairs_mesh : nullptr;
         break;

      case mws_vrn_obj_types::cells:
         is_visible = s.cell_triangles_visible = !s.cell_triangles_visible;
         node = (vgeom) ? vgeom->voronoi_cell_faces_mesh : nullptr;
         break;

      case mws_vrn_obj_types::convex_hull:
         is_visible = s.convex_hull_visible = !s.convex_hull_visible;
         node = (vgeom) ? vgeom->convex_hull_mesh : nullptr;
         break;

      case mws_vrn_obj_types::delaunay_diag:
         is_visible = s.delaunay_diag_visible = !s.delaunay_diag_visible;
         node = (vgeom) ? vgeom->delaunay_diag_mesh : nullptr;
         break;

      case mws_vrn_obj_types::cell_borders:
         is_visible = s.cell_borders_visible = !s.cell_borders_visible;
         node = (vgeom) ? vgeom->cell_borders : nullptr;
         break;

      default:
         mws_assert(false);
      }

      if (node)
      {
         node->visible = is_visible;
      }
   }

   if (visibility_changed)
   {
      if (vgen)
      {
         vgen->diag_data_state = mws_vrn_diag_data_change_type::diag_rebuild_geometry;
      }
   }
}

void mws_vrn_main::update_diag()
{
   vgen->update_diag();
   vgeom->update_geometry(diag_data);
}

void mws_vrn_main::update_geometry()
{
   vgeom->update_geometry(diag_data);
}

void mws_vrn_main::resize(uint32 i_diag_width, uint32 i_diag_height)
{
   mws_assert(i_diag_width > 0 && i_diag_height > 0);
   const std::vector<float>& vx = vgen->vx;
   const std::vector<float>& vy = vgen->vy;
   diag_data->info.diag_width = i_diag_width;
   diag_data->info.diag_height = i_diag_height;

   uint32 size = vx.size();
   glm::vec2 resize_fact = glm::vec2(diag_data->info.diag_width, diag_data->info.diag_height) /
      glm::vec2(diag_data->info.original_diag_width, diag_data->info.original_diag_height);
   std::vector<float> kvx(size);
   std::vector<float> kvy(size);

   for (uint32 k = 0; k < size; k++)
   {
      kvx[k] = vx[k] * resize_fact.x;
      kvy[k] = vy[k] * resize_fact.y;
   }

   vgen->voronoi_diag_impl->init_data(diag_data, kvx, kvy);
   update_diag();
}

const mws_sp<mws_vrn_data> mws_vrn_main::get_diag_data() const { return diag_data; }

void mws_vrn_main::update_nexus_pairs_geometry()
{
   if (diag_data->info.nexus_pairs_visible)
   {
      //vgen->set_altitude(diag_data->geom.nexus_pairs);
      vgeom->set_line_geometry(diag_data->geom.nexus_pairs, vgeom->nexus_pairs_mesh);
   }
}

void mws_vrn_main::set_kernel_points(std::vector<glm::vec2> i_kernel_points)
{
   std::vector<float>& vx = vgen->vx;
   std::vector<float>& vy = vgen->vy;
   uint32 size = i_kernel_points.size();

   vx.resize(size);
   vy.resize(size);

   for (uint32 k = 0; k < size; k++)
   {
      vx[k] = i_kernel_points[k].x;
      vy[k] = i_kernel_points[k].y;
   }

   vgen->voronoi_diag_impl->init_data(diag_data, vx, vy);
   update_diag();
}

mws_vrn_diag::idx_dist mws_vrn_main::get_kernel_idx_at(float i_x, float i_y) const
{
   return vgen->voronoi_diag_impl->get_kernel_idx_at(i_x, i_y);
}

glm::vec2 mws_vrn_main::get_kernel_at(uint32 i_idx) const
{
   return vgen->voronoi_diag_impl->get_kernel_at(i_idx);
}

void mws_vrn_main::move_kernel_to(uint32 i_idx, float i_x, float i_y)
{
   vgen->voronoi_diag_impl->move_kernel_to(i_idx, i_x, i_y);
   vgeom->update_geometry(diag_data);
}

void mws_vrn_main::insert_kernel_at(float i_x, float i_y)
{
   vgen->voronoi_diag_impl->insert_kernel_at(i_x, i_y);
   vgeom->update_geometry(diag_data);
}

void mws_vrn_main::remove_kernel(uint32 i_idx)
{
   vgen->voronoi_diag_impl->remove_kernel(i_idx);
   update_geometry();
}

mws_vrn_main::mws_vrn_main() {}

void mws_vrn_main::setup(uint32 i_diag_width, uint32 i_diag_height, mws_sp<gfx_camera> i_cam)
{
   diag_data = mws_sp<mws_vrn_data>(new mws_vrn_data());
   diag_data->geom.cell_points_ids.vdata = diag_data;
   diag_data->info.diag_width = i_diag_width;
   diag_data->info.diag_height = i_diag_height;
   cam = i_cam;
}
