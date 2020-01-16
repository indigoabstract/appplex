#include "stdafx.hxx"

#include "gfx-surface.hxx"
#include "rng/rng.hxx"


using gfx_vxo_util::set_mesh_data;

gfx_icosahedron::gfx_icosahedron() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord")) {}

void gfx_icosahedron::set_dimensions(float iradius)
{
   float size = iradius;
   float t = (1.f + glm::sqrt(5.f)) / 2.f;


   static vx_fmt_p3f_n3f_t2f tvertices_data[] =
      // x,y,z,	nx,ny,nz,	u,v,
   {
      {{-size, t*size, 0}, {}, {0, 0,}},
      {{size, t*size, 0}, {}, {0, 0,}},
      {{-size, -t * size, 0}, {}, {0, 0,}},
      {{size, -t * size, 0}, {}, {0, 0,}},

      {{0, -size, t*size}, {}, {0, 0,}},
      {{0, size, t*size}, {}, {0, 0,}},
      {{0, -size, -t * size}, {}, {0, 0,}},
      {{0, size, -t * size}, {}, {0, 0,}},

      {{t*size, 0, -size}, {}, {0, 0,}},
      {{t*size, 0, size}, {}, {0, 0,}},
      {{-t * size, 0, -size}, {}, {0, 0,}},
      {{-t * size, 0, size}, {}, {0, 0,}},
   };
   const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);

   const gfx_indices_type tindices_data[] =
   {
      // 5 faces around point 0
      0, 11, 5,
      0, 5, 1,
      0, 1, 7,
      0, 7, 10,
      0, 10, 11,

      // 5 adjacent faces
      1, 5, 9,
      5, 11, 4,
      11, 10, 2,
      10, 7, 6,
      7, 1, 8,

      // 5 faces around point 3
      3, 9, 4,
      3, 4, 2,
      3, 2, 6,
      3, 6, 8,
      3, 8, 9,

      // 5 adjacent faces
      4, 9, 5,
      2, 4, 11,
      6, 2, 10,
      8, 6, 7,
      9, 8, 1,
   };

   set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


gfx_vpc_box::gfx_vpc_box() : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"))
{
}

void gfx_vpc_box::set_dimensions(float iradius, int isegments)
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

      pos.x *= iradius;
      pos.y *= iradius;
      pos.z *= iradius;
   }

   int side_vertex_count = isegments + 1;
   int face_vertex_count = side_vertex_count * side_vertex_count;
   int total_face_vertex_count = 6 * face_vertex_count;
   std::vector<vx_fmt_p3f_n3f_t2f> ks_vertices_data;
   std::vector<gfx_indices_type> ks_indices_data;

   for (int k = 0, idx = 0; k < 6; k++)
   {
      const vx_fmt_p3f_n3f_t2f& bl = tvertices_data[4 * k + 0];
      const vx_fmt_p3f_n3f_t2f& tl = tvertices_data[4 * k + 1];
      const vx_fmt_p3f_n3f_t2f& tr = tvertices_data[4 * k + 2];
      const vx_fmt_p3f_n3f_t2f& br = tvertices_data[4 * k + 3];

      for (int i = 0; i < isegments; i++)
      {
         float vt1 = float(i) / isegments;
         float vt2 = float(i + 1) / isegments;
         glm::vec3 hstart_pos1 = glm::mix(tl.pos, bl.pos, vt1);
         glm::vec3 hstart_pos2 = glm::mix(tl.pos, bl.pos, vt2);
         glm::vec3 hend_pos1 = glm::mix(tr.pos, br.pos, vt1);
         glm::vec3 hend_pos2 = glm::mix(tr.pos, br.pos, vt2);
         glm::vec3 hstart_nrm1 = glm::mix(tl.nrm, bl.nrm, vt1);
         glm::vec3 hstart_nrm2 = glm::mix(tl.nrm, bl.nrm, vt2);
         glm::vec3 hend_nrm1 = glm::mix(tr.nrm, br.nrm, vt1);
         glm::vec3 hend_nrm2 = glm::mix(tr.nrm, br.nrm, vt2);
         glm::vec2 hstart_tex1 = glm::mix(tl.tex, bl.tex, vt1);
         glm::vec2 hstart_tex2 = glm::mix(tl.tex, bl.tex, vt2);
         glm::vec2 hend_tex1 = glm::mix(tr.tex, br.tex, vt1);
         glm::vec2 hend_tex2 = glm::mix(tr.tex, br.tex, vt2);

         for (int j = 0; j < isegments; j++, idx++)
         {
            int i_off = idx * 4;
            float ht1 = float(j) / isegments;
            float ht2 = float(j + 1) / isegments;
            vx_fmt_p3f_n3f_t2f vx0 = { glm::mix(hstart_pos2, hend_pos2, ht1), glm::mix(hstart_nrm2, hend_nrm2, ht1), glm::mix(hstart_tex2, hend_tex2, ht1) };
            vx_fmt_p3f_n3f_t2f vx1 = { glm::mix(hstart_pos1, hend_pos1, ht1), glm::mix(hstart_nrm1, hend_nrm1, ht1), glm::mix(hstart_tex1, hend_tex1, ht1) };
            vx_fmt_p3f_n3f_t2f vx2 = { glm::mix(hstart_pos1, hend_pos1, ht2), glm::mix(hstart_nrm1, hend_nrm1, ht2), glm::mix(hstart_tex1, hend_tex1, ht2) };
            vx_fmt_p3f_n3f_t2f vx3 = { glm::mix(hstart_pos2, hend_pos2, ht2), glm::mix(hstart_nrm2, hend_nrm2, ht2), glm::mix(hstart_tex2, hend_tex2, ht2) };

            ks_vertices_data.push_back(vx0);
            ks_vertices_data.push_back(vx1);
            ks_vertices_data.push_back(vx2);
            ks_vertices_data.push_back(vx3);
            ks_indices_data.push_back(i_off + 1);
            ks_indices_data.push_back(i_off + 0);
            ks_indices_data.push_back(i_off + 2);
            ks_indices_data.push_back(i_off + 2);
            ks_indices_data.push_back(i_off + 0);
            ks_indices_data.push_back(i_off + 3);
         }
      }
   }

   int vdata_size = ks_vertices_data.size() * sizeof(vx_fmt_p3f_n3f_t2f);
   int idata_size = ks_indices_data.size() * sizeof(gfx_indices_type);
   set_mesh_data((const uint8*)begin_ptr(ks_vertices_data), vdata_size, begin_ptr(ks_indices_data), idata_size, std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}


gfx_vpc_kubic_sphere::gfx_vpc_kubic_sphere() : gfx_vxo(vx_info("a_v3_position, a_iv4_color, a_v3_normal, a_v2_tex_coord"))
{
}

enum segment_point
{
   start_point,
   end_point,
};

enum cube_face_type
{
   front_face,
   back_face,
   top_face,
   bottom_face,
   left_face,
   right_face,
};
struct segment
{
   segment_point start;
   segment_point end;
};

struct cube_edge
{
   cube_face_type face_type;
   segment vertical;
   segment horizontal;
};

struct common_edge
{
   cube_edge first;
   cube_edge second;
};

void gfx_vpc_kubic_sphere::set_dimensions(float iradius, int isegments)
{
   static vx_fmt_p3f_n3f_t2f tvertices_data[] =
      // x,y,z,	nx,ny,nz,	u,v,
   {
      {{-1, -1, +1,}, {}, {0, 0,}},		{{-1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, +1,}, {}, {1, 1,}},		{{+1, -1, +1,}, {}, {1, 0,}}, // front
      {{+1, -1, -1,}, {}, {0, 0,}},		{{+1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, -1,}, {}, {1, 1,}},		{{-1, -1, -1,}, {}, {1, 0,}}, // back
      {{-1, +1, +1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, +1, +1,}, {}, {1, 0,}}, // top
      {{-1, -1, -1,}, {}, {0, 0,}},		{{-1, -1, +1,}, {}, {0, 1,}},		{{+1, -1, +1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // bottom
      {{-1, -1, -1,}, {}, {0, 0,}},		{{-1, +1, -1,}, {}, {0, 1,}},		{{-1, +1, +1,}, {}, {1, 1,}},		{{-1, -1, +1,}, {}, {1, 0,}}, // left
      {{+1, -1, +1,}, {}, {0, 0,}},		{{+1, +1, +1,}, {}, {0, 1,}},		{{+1, +1, -1,}, {}, {1, 1,}},		{{+1, -1, -1,}, {}, {1, 0,}}, // right
   };
   const int tvertices_data_length = sizeof(tvertices_data) / sizeof(vx_fmt_p3f_n3f_t2f);
   int side_vertex_count = isegments + 1;
   int face_vertex_count = side_vertex_count * side_vertex_count;
   int total_face_vertex_count = 6 * face_vertex_count;
   std::vector<vx_fmt_p3f_c4b_n3f_t2f> ks_vertices_data;
   std::vector<gfx_indices_type> ks_indices_data;
   RNG rng;
   std::vector<std::vector<std::vector<float> > > displ_map(6);
   float displ_delta = iradius * 0.005f;
   float displ_pos_range = 100000.f;

   static int face_links[6][4] =
   {
      {top_face, right_face, bottom_face, left_face}, // front_face
      {top_face, left_face, bottom_face, right_face}, // back_face
      {back_face, right_face, front_face, left_face}, // top_face
      {front_face, right_face, back_face, left_face}, // bottom_face
      {top_face, front_face, bottom_face, back_face}, // left_face
      {top_face, back_face, bottom_face, front_face}, // right_face
   };

   static common_edge common_edges_tab[] =
   {
      {{front_face, {start_point, start_point}, {start_point, end_point}}, {top_face, {end_point, end_point}, {start_point, end_point}}},
      {{front_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {start_point, end_point}, {start_point, start_point}}},
      {{top_face, {start_point, start_point}, {start_point, end_point}}, {back_face, {start_point, start_point}, {end_point, start_point}}},
      {{top_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {start_point, start_point}, {end_point, start_point}}},
      {{left_face, {start_point, start_point}, {start_point, end_point}}, {top_face, {start_point, end_point}, {start_point, start_point}}},
      {{left_face, {start_point, end_point}, {end_point, end_point}}, {front_face, {start_point, end_point}, {start_point, start_point}}},
      {{left_face, {start_point, end_point}, {start_point, start_point}}, {back_face, {start_point, end_point}, {end_point, end_point}}},
      {{left_face, {end_point, end_point}, {start_point, end_point}}, {bottom_face, {end_point, start_point}, {start_point, start_point}}},
      {{bottom_face, {start_point, start_point}, {start_point, end_point}}, {front_face, {end_point, end_point}, {start_point, end_point}}},
      {{bottom_face, {start_point, end_point}, {end_point, end_point}}, {right_face, {end_point, end_point}, {start_point, end_point}}},
      {{right_face, {start_point, end_point}, {end_point, end_point}}, {back_face, {start_point, end_point}, {start_point, start_point}}},
      {{back_face, {end_point, end_point}, {start_point, end_point}}, {bottom_face, {end_point, end_point}, {end_point, start_point}}},
   };
   gfx_color face_colors[6] =
   {
      {255, 000, 000, 255},
      {000, 255, 000, 255},
      {000, 000, 255, 255},
      {255, 255, 000, 255},
      {000, 255, 255, 255},
      {255, 000, 255, 255},
   };

   for (int k = 0; k < 6; k++)
   {
      displ_map[k].resize(side_vertex_count);

      for (int i = 0; i < side_vertex_count; i++)
      {
         displ_map[k][i].resize(side_vertex_count);

         for (int j = 0; j < side_vertex_count; j++)
         {
            displ_map[k][i][j] = rng.range(-displ_pos_range, displ_pos_range) / displ_pos_range * displ_delta;
         }
      }
   }

   struct point_val
   {
      int operator()(segment_point isp, int istart, int iend) const
      {
         if (isp == start_point)
         {
            return istart;
         }

         return iend;
      }
   };
   point_val pv;

   for (int k = 0; k < 12; k++)
   {
      common_edge c = common_edges_tab[k];
      cube_edge fe = c.first;
      cube_edge se = c.second;
      // vertical coord, first, second
      int vfs = pv(fe.vertical.start, 0, isegments);
      int vfe = pv(fe.vertical.end, 0, isegments);
      int vfi = (vfe - vfs) / isegments;
      int vss = pv(se.vertical.start, 0, isegments);
      int vse = pv(se.vertical.end, 0, isegments);
      int vsi = (vse - vss) / isegments;
      // horizontal coord, first, second
      int hfs = pv(fe.horizontal.start, 0, isegments);
      int hfe = pv(fe.horizontal.end, 0, isegments);
      int hfi = (hfe - hfs) / isegments;
      int hss = pv(se.horizontal.start, 0, isegments);
      int hse = pv(se.horizontal.end, 0, isegments);
      int hsi = (hse - hss) / isegments;

      for (int idx = 0; idx < side_vertex_count; idx++)
      {
         float& fv = displ_map[fe.face_type][vfs][hfs];
         float& sv = displ_map[se.face_type][vss][hss];
         float av = (fv + sv) * 0.5f;
         fv = sv = av;
         vfs += vfi;
         vss += vsi;
         hfs += hfi;
         hss += hsi;
      }
   }

   int vi[4] = { 1, 0, 0, 1 };
   int vj[4] = { 0, 0, 1, 1 };

   for (int k = 0, idx = 0; k < 6; k++)
   {
      const vx_fmt_p3f_n3f_t2f& bl = tvertices_data[4 * k + 0];
      const vx_fmt_p3f_n3f_t2f& tl = tvertices_data[4 * k + 1];
      const vx_fmt_p3f_n3f_t2f& tr = tvertices_data[4 * k + 2];
      const vx_fmt_p3f_n3f_t2f& br = tvertices_data[4 * k + 3];

      for (int i = 0; i < isegments; i++)
      {
         float vt1 = float(i) / isegments;
         float vt2 = float(i + 1) / isegments;
         glm::vec3 hstart_pos1 = glm::mix(tl.pos, bl.pos, vt1);
         glm::vec3 hstart_pos2 = glm::mix(tl.pos, bl.pos, vt2);
         glm::vec3 hend_pos1 = glm::mix(tr.pos, br.pos, vt1);
         glm::vec3 hend_pos2 = glm::mix(tr.pos, br.pos, vt2);
         glm::vec2 hstart_tex1 = glm::mix(tl.tex, bl.tex, vt1);
         glm::vec2 hstart_tex2 = glm::mix(tl.tex, bl.tex, vt2);
         glm::vec2 hend_tex1 = glm::mix(tr.tex, br.tex, vt1);
         glm::vec2 hend_tex2 = glm::mix(tr.tex, br.tex, vt2);

         for (int j = 0; j < isegments; j++, idx++)
         {
            int i_off = idx * 4;
            float ht1 = float(j) / isegments;
            float ht2 = float(j + 1) / isegments;
            glm::vec3 pos[4] =
            {
               glm::mix(hstart_pos2, hend_pos2, ht1),
               glm::mix(hstart_pos1, hend_pos1, ht1),
               glm::mix(hstart_pos1, hend_pos1, ht2),
               glm::mix(hstart_pos2, hend_pos2, ht2),
            };
            glm::vec3 nrm[4];
            glm::vec3 gpos[4];

            for (int v = 0; v < 4; v++)
            {
               gpos[v] = glm::normalize(glm::vec3(pos[v].x, pos[v].y, pos[v].z));
               nrm[v].x = 0;//gpos[v].x;
               nrm[v].y = 0;//gpos[v].y;
               nrm[v].z = 0;//gpos[v].z;

               float height_val = 0;//displ_map[k][i + vi[v]][j + vj[v]];

               gpos[v] *= (iradius + height_val);
               pos[v].x = gpos[v].x;
               pos[v].y = gpos[v].y;
               pos[v].z = gpos[v].z;
               //pos[v].x *= iradius;
               //pos[v].y *= iradius;
               //pos[v].z *= iradius;
            }

            vx_fmt_p3f_c4b_n3f_t2f vx0 = { pos[0], face_colors[k], nrm[0], glm::mix(hstart_tex2, hend_tex2, ht1) };
            vx_fmt_p3f_c4b_n3f_t2f vx1 = { pos[1], face_colors[k], nrm[1], glm::mix(hstart_tex1, hend_tex1, ht1) };
            vx_fmt_p3f_c4b_n3f_t2f vx2 = { pos[2], face_colors[k], nrm[2], glm::mix(hstart_tex1, hend_tex1, ht2) };
            vx_fmt_p3f_c4b_n3f_t2f vx3 = { pos[3], face_colors[k], nrm[3], glm::mix(hstart_tex2, hend_tex2, ht2) };

            ks_vertices_data.push_back(vx0);
            ks_vertices_data.push_back(vx1);
            ks_vertices_data.push_back(vx2);
            ks_vertices_data.push_back(vx3);
            ks_indices_data.push_back(i_off + 1);
            ks_indices_data.push_back(i_off + 0);
            ks_indices_data.push_back(i_off + 2);
            ks_indices_data.push_back(i_off + 2);
            ks_indices_data.push_back(i_off + 0);
            ks_indices_data.push_back(i_off + 3);
         }
      }
   }

   for (int i = 0; i < ks_indices_data.size(); i += 3)
   {
      vx_fmt_p3f_c4b_n3f_t2f& tv0 = ks_vertices_data[ks_indices_data[i]];
      vx_fmt_p3f_c4b_n3f_t2f& tv1 = ks_vertices_data[ks_indices_data[i + 1]];
      vx_fmt_p3f_c4b_n3f_t2f& tv2 = ks_vertices_data[ks_indices_data[i + 2]];
      glm::vec3 v0(tv0.pos.x, tv0.pos.y, tv0.pos.z);
      glm::vec3 v1(tv1.pos.x, tv1.pos.y, tv1.pos.z);
      glm::vec3 v2(tv2.pos.x, tv2.pos.y, tv2.pos.z);

      glm::vec3 normal = -glm::normalize(glm::cross(v2 - v0, v1 - v0));
      glm::vec3 nrm = { normal.x, normal.y, normal.z };

      glm::vec3& n0 = ks_vertices_data[ks_indices_data[i]].nrm;
      glm::vec3& n1 = ks_vertices_data[ks_indices_data[i + 1]].nrm;
      glm::vec3& n2 = ks_vertices_data[ks_indices_data[i + 2]].nrm;
      n0.x += nrm.x;
      n0.y += nrm.y;
      n0.z += nrm.z;
      n1.x += nrm.x;
      n1.y += nrm.y;
      n1.z += nrm.z;
      n2.x += nrm.x;
      n2.y += nrm.y;
      n2.z += nrm.z;
   }

   for (int i = 0; i < ks_vertices_data.size(); i++)
   {
      glm::vec3 n(ks_vertices_data[i].nrm.x, ks_vertices_data[i].nrm.y, ks_vertices_data[i].nrm.z);
      n = glm::normalize(n);
      glm::vec3 nn = { n.x, n.y, n.z };
      ks_vertices_data[i].nrm = nn;
   }

   int vdata_size = ks_vertices_data.size() * sizeof(vx_fmt_p3f_c4b_n3f_t2f);
   int idata_size = ks_indices_data.size() * sizeof(gfx_indices_type);
   set_mesh_data((const uint8*)begin_ptr(ks_vertices_data), vdata_size, begin_ptr(ks_indices_data), idata_size, std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}
