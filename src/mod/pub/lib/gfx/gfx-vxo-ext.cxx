#include "stdafx.hxx"

#include "gfx-vxo-ext.hxx"
#include "data-seqv.hxx"


using gfx_vxo_util::set_mesh_data;

gfx_debug_vxo::gfx_debug_vxo(vx_info i_vxi, bool i_is_submesh) : gfx_vxo(i_vxi, i_is_submesh)
{
}

void gfx_debug_vxo::draw_in_sync(mws_sp<gfx_camera> i_camera)
{
   gfx_vxo::draw_in_sync(i_camera);
}


gfx_obj_vxo::gfx_obj_vxo() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
   is_loaded = false;
}

void gfx_obj_vxo::operator=(const std::string& i_mesh_name)
{
   set_mesh_name(i_mesh_name);
}

//void gfx_obj_mesh::draw_in_sync(mws_sp<gfx_camera> i_camera)
//{
//	std::vector<mws_sp<gfx_mesh> >::iterator it = mesh_list.begin();
//
//	for(; it != mesh_list.end(); ++it)
//	{
//		(*it)->draw_in_sync(i_camera);
//	}
//}


gfx_plane::gfx_plane(mws_sp<gfx> i_gi) : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"), i_gi)
{
}

void gfx_plane::set_dimensions(float i_dx, float i_dy, float i_z_val)
{
   float p = 0.5;
   const vx_fmt_p3f_n3f_t2f tvertices_data[] =
      // xyz, uv
   {
      {{-p * i_dx,  p * i_dy, i_z_val}, {0, 0, -1}, {0, 1}},
      {{-p * i_dx, -p * i_dy, i_z_val}, {0, 0, -1}, {0, 0}},
      {{ p * i_dx, -p * i_dy, i_z_val}, {0, 0, -1}, {1, 0}},
      {{ p * i_dx,  p * i_dy, i_z_val}, {0, 0, -1}, {1, 1}},
   };

   const gfx_indices_type tindices_data[] =
   {
      //0, 1, 2, 0, 2, 3
      //2, 0, 1, 0, 2, 3,
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


gfx_billboard::gfx_billboard()
{
}


gfx_grid::gfx_grid() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_grid::set_dimensions(int i_h_point_count, int i_v_point_count)
{
   static float p = 1.f;
   static glm::vec3 vx[] =
   {
      glm::vec3(-p, p, 0), glm::vec3(-p, -p, 0), glm::vec3(p, -p, 0), glm::vec3(p, p, 0),
   };
   static glm::vec2 tx[] =
   {
      glm::vec2(0, 1), glm::vec2(0, 0), glm::vec2(1, 0), glm::vec2(1, 1),
   };
   std::vector<vx_fmt_p3f_n3f_t2f> tvertices_data(i_h_point_count * i_v_point_count);
   std::vector<gfx_indices_type> tindices_data;

   for (int i = 0; i < i_v_point_count; i++)
   {
      float vf = i / float(i_v_point_count - 1);
      glm::vec3 left_vx = glm::mix(vx[0], vx[1], vf);
      glm::vec2 left_tx = glm::mix(tx[0], tx[1], vf);
      glm::vec3 right_vx = glm::mix(vx[3], vx[2], vf);
      glm::vec2 right_tx = glm::mix(tx[3], tx[2], vf);

      for (int j = 0; j < i_h_point_count; j++)
      {
         float hf = j / float(i_h_point_count - 1);
         glm::vec3 it_vx = glm::mix(left_vx, right_vx, hf);
         glm::vec2 it_tx = glm::mix(left_tx, right_tx, hf);
         auto& r = tvertices_data[i_h_point_count * i + j];

         r.pos = it_vx;
         r.nrm = glm::vec3(0, 0, -1);
         r.tex = it_tx;
      }
   }

   for (int i = 1; i < i_v_point_count; i++)
   {
      for (int j = 1; j < i_h_point_count; j++)
      {
         int v = i_h_point_count * i;
         int v_m1 = i_h_point_count * (i - 1);
         int h = j;
         int h_m1 = j - 1;

         // lower left triangle: 1, 0, 2 in quad drawing
         tindices_data.push_back(v + h_m1);
         tindices_data.push_back(v_m1 + h_m1);
         tindices_data.push_back(v + h);
         // upper right triangle: 3, 2, 0 in quad drawing
         tindices_data.push_back(v_m1 + h);
         tindices_data.push_back(v + h);
         tindices_data.push_back(v_m1 + h_m1);
      }
   }

   set_mesh_data((const uint8*)tvertices_data.data(), sizeof(vx_fmt_p3f_n3f_t2f) * tvertices_data.size(),
      tindices_data.data(), sizeof(gfx_indices_type) * tindices_data.size(), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


gfx_box::gfx_box() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_box::set_dimensions(float i_dx, float i_dy, float i_dz)
{
   vx_fmt_p3f_n3f_t2f tvertices_data[] =
      // x,y,z,	nx,ny,nz,	u,v,
   {
      {{-1, -1, +1,}, {}, {0, 0,}},		{{-1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, +1,}, {}, {1, 1,}},		{{+1, -1, +1,}, {}, {1, 0,}}, // front
      {{+1, -1, -1,}, {}, {0, 0,}},		{{+1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, -1,}, {}, {1, 1,}},		{{-1, -1, -1,}, {}, {1, 0,}}, // back
      {{-1, +1, +1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, +1, +1,}, {}, {1, 0,}}, // top
      {{+1, -1, +1,}, {}, {0, 0,}},		{{+1, -1, -1,}, {}, {0, 1,}},		{{-1, -1, -1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // bottom
      {{-1, -1, -1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, +1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // left
      {{+1, -1, +1,}, {}, {0, 0,}},		{{+1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // right
   };
   const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);

   const gfx_indices_type tindices_data[] =
   {
      1, 0, 2, 2, 0, 3,
      5, 4, 6, 6, 4, 7,
      9, 8, 10, 10, 8, 11,
      13, 12, 14, 14, 12, 15,
      17, 16, 18, 18, 16, 19,
      21, 20, 22, 22, 20, 23,
   };

   for (int k = 0; k < 6; k++)
   {
      int i = k * 4;
      glm::vec3& pos0 = tvertices_data[i + 0].pos;
      glm::vec3& pos1 = tvertices_data[i + 1].pos;
      glm::vec3& pos2 = tvertices_data[i + 2].pos;
      glm::vec3 v0(pos0.x, pos0.y, pos0.z);
      glm::vec3 v1(pos1.x, pos1.y, pos1.z);
      glm::vec3 v2(pos2.x, pos2.y, pos2.z);
      glm::vec3 va = v0 - v1;
      glm::vec3 vb = v2 - v1;
      glm::vec3 sn = glm::cross(va, vb);
      sn = glm::normalize(sn);

      for (int l = 0; l < 4; l++)
      {
         tvertices_data[i + l].nrm.x = sn.x;
         tvertices_data[i + l].nrm.y = sn.y;
         tvertices_data[i + l].nrm.z = sn.z;
      }
   }

   for (int k = 0; k < tvertices_data_length; k++)
   {
      glm::vec3& pos = tvertices_data[k].pos;

      pos.x *= i_dx;
      pos.y *= i_dy;
      pos.z *= i_dz;
   }

   set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


gfx_vpc_ring_sphere::gfx_vpc_ring_sphere() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_vpc_ring_sphere::set_dimensions(float i_radius, int i_grid_point_count)
{
   int gridPointsWidth = i_grid_point_count;

   if (gridPointsWidth % 2 == 0)
      // the number horizontal points has to be odd, not even and > 1
   {
      gridPointsWidth++;
   }

   // the number vertical points has to be odd, not even and > 1
   int gridPointsHeight = gridPointsWidth / 2 + 1;
   int gridSizeWidth = gridPointsWidth - 1;
   int gridSizeHeight = gridPointsHeight - 1;
   float igdWidth = 1.f / float(gridPointsWidth - 1);
   float igdHeight = 1.f / float(gridPointsHeight - 1);
   int indicesLength = 6 * gridSizeWidth * gridSizeHeight + 6 * (gridSizeWidth - 1);
   int verticesLength = gridPointsWidth * gridPointsHeight;
   std::vector<vx_fmt_p3f_n3f_t2f> rs_vertices_data;
   std::vector<gfx_indices_type> rs_indices_data;

   rs_vertices_data.resize(verticesLength);
   rs_indices_data.resize(indicesLength);

   // form a triangle strip
   //unsigned short ind[] = {
   //	3 * 0 + 0, 3 * 0 + 1, 3 * 1 + 0,
   //	3 * 0 + 1, 3 * 1 + 0, 3 * 1 + 1,
   //	3 * 1 + 0, 3 * 1 + 1, 3 * 2 + 0,
   //	3 * 1 + 1, 3 * 2 + 0, 3 * 2 + 1,
   //	// distorted triangles
   //	3 * 2 + 0, 3 * 2 + 1, 3 * 2 + 1,
   //	3 * 2 + 1, 3 * 2 + 1, 3 * 2 + 2,
   //	// second strip
   //	3 * 2 + 1, 3 * 2 + 2, 3 * 1 + 1,
   //	3 * 2 + 2, 3 * 1 + 1, 3 * 1 + 2,
   //	3 * 1 + 1, 3 * 1 + 2, 3 * 0 + 1,
   //	3 * 1 + 2, 3 * 0 + 1, 3 * 0 + 2,
   //};
   //RNG rng;

   // set vertex and texture coordinates
   for (int h = 0; h < gridPointsHeight; h++)
   {
      int idx = gridPointsWidth * h;
      // spherical coordinates theta; theta in [0*, 180*]
      float theta = (h / float(gridPointsHeight - 1)) * glm::pi<float>();
      // spherical coordinates z is mapped as y, with y in [-1, +1]
      float y = i_radius * -glm::cos(theta);
      float sintheta = glm::sin(theta);

      for (int w = 0; w < gridPointsWidth; w++, idx++)
      {
         // spherical coordinates phi; phi in [0*, 360*]
         float phi = (w / float(gridPointsWidth - 1)) * 2 * glm::pi<float>();
         //radius = rng.range(1000, 1025) / 1000.f;
         float x = i_radius * glm::sin(phi) * sintheta;
         float z = i_radius * glm::cos(phi) * sintheta;

         //vertexCrd[idx].x = x;
         //vertexCrd[idx].y = y;
         //vertexCrd[idx].z = z;
         //vertexCrd[idx].u = w * igdWidth;
         //vertexCrd[idx].v = h * igdHeight;
         //vertexCrd[idx].a = 255;
         //vertexCrd[idx].r = 255;
         //vertexCrd[idx].g = 255;
         //vertexCrd[idx].b = 255;
         glm::vec3 nrm(x, y, z);
         nrm = glm::normalize(nrm);
         vx_fmt_p3f_n3f_t2f vx = { {x, y, z}, {nrm.x, nrm.y, nrm.z}, {w * igdWidth, 1.f - h * igdHeight} };
         rs_vertices_data[idx] = vx;
      }
   }

   // set indices
   for (int hIdx = 0, idx = 0; hIdx < gridSizeWidth;)
   {
      // left column
      for (int vIdx = 0; vIdx < gridSizeHeight; vIdx++)
      {
         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx;
         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx + 1;
         rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx;

         rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx;
         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx + 1;
         rs_indices_data[idx++] = gridPointsWidth * (vIdx + 1) + hIdx + 1;
      }

      // distorted triangles
      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx;
      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;

      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 1;
      //rs_indices_data[idx++] = gridPointsWidth * gridSizeHeight + hIdx + 2;

      hIdx++;

      // right column
      for (int vIdx = gridSizeHeight; vIdx > 0; vIdx--)
      {
         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx + 1;
         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx;
         rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx;

         rs_indices_data[idx++] = gridPointsWidth * vIdx + hIdx + 1;
         rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx;
         rs_indices_data[idx++] = gridPointsWidth * (vIdx - 1) + hIdx + 1;
      }

      // distorted triangles. if this is the last column, do not continue
      //if(hIdx < gridSizeWidth - 1)
      //{
      //	rs_indices_data[idx++] = hIdx;
      //	rs_indices_data[idx++] = hIdx + 1;
      //	rs_indices_data[idx++] = hIdx + 1;

      //	rs_indices_data[idx++] = hIdx + 1;
      //	rs_indices_data[idx++] = hIdx + 1;
      //	rs_indices_data[idx++] = hIdx + 2;
      //}

      hIdx++;
   }

   int vdata_size = rs_vertices_data.size() * sizeof(vx_fmt_p3f_n3f_t2f);
   int idata_size = rs_indices_data.size() * sizeof(gfx_indices_type);
   set_mesh_data((const uint8*)rs_vertices_data.data(), vdata_size, rs_indices_data.data(), idata_size, std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
   //mws_print("ind length %d") % indicesLength;
}


gfx_right_prism::gfx_right_prism() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord")) {}

void gfx_right_prism::set_dimensions(const std::vector<glm::vec2>& i_base_vertices, float i_height)
{
   const uint32 side_count = i_base_vertices.size();
   mws_assert(side_count >= 3);
   const gfx_indices_type vx_offset = 2;
   // origin in zero
   const glm::vec3 origin(0.f);
   std::vector<gfx_indices_type> indices_data;
   data_seqv_writer dsw(std::make_shared<mem_data_seqv>());

   // write bottom origin positions
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   // normals
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   // tex coords
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);

   // write top origin positions
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(i_height);
   // normals
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   // tex coords
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);

   // write bottom face vertices
   for (uint32 k = 0; k < side_count; k++)
   {
      const glm::vec2& vx = i_base_vertices[k];
      // bottom face positions
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
   }

   // write top face vertices
   for (uint32 k = 0; k < side_count; k++)
   {
      const glm::vec2& vx = i_base_vertices[k];
      // top face positions
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(i_height);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
   }

   // write side face vertices
   for (uint32 k = 0; k < side_count; k++)
   {
      uint32 kxx = (k + 1) % side_count;
      const glm::vec2& vx = i_base_vertices[k];
      const glm::vec2& vx_next = i_base_vertices[kxx];
      // positions 0 bottom
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);

      // positions 1 bottom
      dsw.write_fltp32(vx_next.x);
      dsw.write_fltp32(vx_next.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);

      // positions 2 top
      dsw.write_fltp32(vx_next.x);
      dsw.write_fltp32(vx_next.y);
      dsw.write_fltp32(i_height);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);

      // positions 3 top
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(i_height);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
   }

   for (uint32 k = 0; k < side_count; k++)
   {
      uint32 k_next = (k + 1) % side_count;
      // bottom face indices
      indices_data.push_back(0);
      indices_data.push_back(vx_offset + k);
      indices_data.push_back(vx_offset + k_next);
      // top face indices
      indices_data.push_back(1);
      indices_data.push_back(vx_offset + side_count + k_next);
      indices_data.push_back(vx_offset + side_count + k);
   }

   // side face indices
   for (uint32 k = 0; k < side_count; k++)
   {
      const gfx_indices_type vx_side_offset = vx_offset + 2 * side_count;
      indices_data.push_back(vx_side_offset + 4 * k + 0);
      indices_data.push_back(vx_side_offset + 4 * k + 2);
      indices_data.push_back(vx_side_offset + 4 * k + 1);
      indices_data.push_back(vx_side_offset + 4 * k + 0);
      indices_data.push_back(vx_side_offset + 4 * k + 3);
      indices_data.push_back(vx_side_offset + 4 * k + 2);
   }

   std::shared_ptr<data_seqv> ds = dsw.get_data_sequence();
   const uint8_t* vertices_data = ds->data_as_byte_array();

   set_data(vertices_data, ds->size(), indices_data.data(), indices_data.size());
}


gfx_right_pyramid::gfx_right_pyramid() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord")) {}

void gfx_right_pyramid::set_dimensions(const std::vector<glm::vec2>& i_base_vertices, float i_height)
{
   const uint32 side_count = i_base_vertices.size();
   mws_assert(side_count >= 3);
   const gfx_indices_type vx_offset = 1;
   // origin in zero
   const glm::vec3 origin(0.f);
   std::vector<gfx_indices_type> indices_data;
   data_seqv_writer dsw(std::make_shared<mem_data_seqv>());

   // write bottom origin positions
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   // normals
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);
   // tex coords
   dsw.write_fltp32(0.f);
   dsw.write_fltp32(0.f);

   // write bottom face vertices
   for (uint32 k = 0; k < side_count; k++)
   {
      const glm::vec2& vx = i_base_vertices[k];
      // bottom face positions
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
   }

   // write side face vertices
   for (uint32 k = 0; k < side_count; k++)
   {
      uint32 kxx = (k + 1) % side_count;
      const glm::vec2& vx = i_base_vertices[k];
      const glm::vec2& vx_next = i_base_vertices[kxx];
      // positions 0 bottom
      dsw.write_fltp32(vx.x);
      dsw.write_fltp32(vx.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);

      // positions 1 bottom
      dsw.write_fltp32(vx_next.x);
      dsw.write_fltp32(vx_next.y);
      dsw.write_fltp32(0.f);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);

      // positions 2 top origin
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(i_height);
      // normals
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
      // tex coords
      dsw.write_fltp32(0.f);
      dsw.write_fltp32(0.f);
   }

   for (uint32 k = 0; k < side_count; k++)
   {
      uint32 k_next = (k + 1) % side_count;
      // bottom face indices
      indices_data.push_back(0);
      indices_data.push_back(vx_offset + k);
      indices_data.push_back(vx_offset + k_next);
   }

   // side face indices
   for (uint32 k = 0; k < side_count; k++)
   {
      const gfx_indices_type vx_side_offset = vx_offset + side_count;
      indices_data.push_back(vx_side_offset + 3 * k + 0);
      indices_data.push_back(vx_side_offset + 3 * k + 2);
      indices_data.push_back(vx_side_offset + 3 * k + 1);
   }

   std::shared_ptr<data_seqv> ds = dsw.get_data_sequence();
   const uint8_t* vertices_data = ds->data_as_byte_array();

   set_data(vertices_data, ds->size(), indices_data.data(), indices_data.size());
}
