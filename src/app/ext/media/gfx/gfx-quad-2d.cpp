#include "stdafx.h"

#include "gfx-quad-2d.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfm-gl.h"
#include <glm/inc.hpp>


using gfx_vxo_util::set_mesh_data;

mws_sp<gfx_quad_2d> gfx_quad_2d::nwi(mws_sp<gfx> i_gi)
{
   return mws_sp<gfx_quad_2d>(new gfx_quad_2d(i_gi));
}

void gfx_quad_2d::set_anchor(e_anchor_types ianchor_type)
{
   anchor_type = ianchor_type;
   set_translation(tr.x, tr.y);
}

float gfx_quad_2d::get_z() const
{
   return position().z;
}

void gfx_quad_2d::set_z(float i_z_position)
{
   position = glm::vec3(position().x, position().y, i_z_position);
}

glm::vec2 gfx_quad_2d::get_translation() const
{
   return tr;
}

void gfx_quad_2d::set_translation(const glm::vec2& i_tr)
{
   tr = i_tr;

   switch (anchor_type)
   {
   case e_top_left:
      position = glm::vec3(tr.x + sc.x / 2, tr.y + sc.y / 2, position().z);
      break;

   case e_center:
      position = glm::vec3(tr.x, tr.y, position().z);
      break;

   case e_btm_center:
      position = glm::vec3(tr.x, tr.y - sc.y / 2, position().z);
      break;
   }
}

void gfx_quad_2d::set_translation(float ix, float iy)
{
   set_translation(glm::vec2(ix, iy));
}

float gfx_quad_2d::get_rotation() const
{
   return a;
}

void gfx_quad_2d::set_rotation(float ia)
{
   a = ia;
}

glm::vec2 gfx_quad_2d::get_scale() const
{
   return sc;
}

void gfx_quad_2d::set_scale(const glm::vec2& i_sc)
{
   sc = i_sc;
   scaling = glm::vec3(sc.x, sc.y, 1.f);
   set_translation(tr.x, tr.y);
}

void gfx_quad_2d::set_scale(float ix, float iy)
{
   set_scale(glm::vec2(ix, iy));
}

void gfx_quad_2d::set_h_flip(bool i_flip)
{
   std::vector<vx_fmt_p3f_n3f_t2f> tvertices_data;

   if (i_flip)
   {
      vx_data =
      {
         { vx_data[0].pos, vx_data[0].nrm,{ vx_data[3].tex.s, vx_data[3].tex.t } },
      { vx_data[1].pos, vx_data[1].nrm,{ vx_data[2].tex.s, vx_data[2].tex.t } },
      { vx_data[2].pos, vx_data[2].nrm,{ vx_data[1].tex.s, vx_data[1].tex.t } },
      { vx_data[3].pos, vx_data[3].nrm,{ vx_data[0].tex.s, vx_data[0].tex.t } },
      };
   }
   else
   {
      vx_data =
      {
         { vx_data[0].pos, vx_data[0].nrm,{ vx_data[0].tex.s, vx_data[0].tex.t } },
      { vx_data[1].pos, vx_data[1].nrm,{ vx_data[1].tex.s, vx_data[1].tex.t } },
      { vx_data[2].pos, vx_data[2].nrm,{ vx_data[2].tex.s, vx_data[2].tex.t } },
      { vx_data[3].pos, vx_data[3].nrm,{ vx_data[3].tex.s, vx_data[3].tex.t } },
      };
   }


   const gfx_indices_type tindices_data[] =
   {
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)vx_data.data(), vx_data.size() * sizeof(vx_fmt_p3f_n3f_t2f), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}

void gfx_quad_2d::set_v_flip(bool iv_flip)
{
   std::vector<vx_fmt_p3f_n3f_t2f> tvertices_data;

   if (iv_flip)
   {
      vx_data =
      {
         { vx_data[0].pos, vx_data[0].nrm, { vx_data[1].tex.s, vx_data[1].tex.t } },
         { vx_data[1].pos, vx_data[1].nrm, { vx_data[0].tex.s, vx_data[0].tex.t } },
         { vx_data[2].pos, vx_data[2].nrm, { vx_data[3].tex.s, vx_data[3].tex.t } },
         { vx_data[3].pos, vx_data[3].nrm, { vx_data[2].tex.s, vx_data[2].tex.t } },
      };
   }
   else
   {
      vx_data =
      {
         { vx_data[0].pos, vx_data[0].nrm, { vx_data[0].tex.s, vx_data[0].tex.t } },
         { vx_data[1].pos, vx_data[1].nrm, { vx_data[1].tex.s, vx_data[1].tex.t } },
         { vx_data[2].pos, vx_data[2].nrm, { vx_data[2].tex.s, vx_data[2].tex.t } },
         { vx_data[3].pos, vx_data[3].nrm, { vx_data[3].tex.s, vx_data[3].tex.t } },
      };
   }


   const gfx_indices_type tindices_data[] =
   {
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)vx_data.data(), vx_data.size() * sizeof(vx_fmt_p3f_n3f_t2f), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}

void gfx_quad_2d::set_dimensions(float idx, float idy, float i_z_val)
{
   float p = 0.5;
   vx_data =
   {
      { { -p * idx,  p * idy, i_z_val }, { 0, 0, -1 }, { 0, 1 } },
   { { -p * idx, -p * idy, i_z_val }, { 0, 0, -1 }, { 0, 0 } },
   { { p * idx, -p * idy, i_z_val }, { 0, 0, -1 }, { 1, 0 } },
   { { p * idx,  p * idy, i_z_val }, { 0, 0, -1 }, { 1, 1 } },
   };

   const gfx_indices_type tindices_data[] =
   {
      //0, 1, 2, 0, 2, 3
      //2, 0, 1, 0, 2, 3,
      1, 0, 2, 3, 2, 0,
   };

   dx = idx;
   dy = idy;
   set_mesh_data((const uint8*)vx_data.data(), vx_data.size() * sizeof(vx_fmt_p3f_n3f_t2f), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}

void gfx_quad_2d::set_tex_coord(glm::vec2 lt, glm::vec2 rt, glm::vec2 rb, glm::vec2 lb)
{
   float p = 0.5;
   vx_data =
   {
      { vx_data[0].pos, vx_data[0].nrm, { lb.s, lb.t } },
      { vx_data[1].pos, vx_data[1].nrm, { lt.s, lt.t } },
      { vx_data[2].pos, vx_data[2].nrm, { rt.s, rt.t } },
      { vx_data[3].pos, vx_data[3].nrm, { rb.s, rb.t } },
   };

   const gfx_indices_type tindices_data[] =
   {
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)vx_data.data(), vx_data.size() * sizeof(vx_fmt_p3f_n3f_t2f), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_mws_sp()));
}

gfx_quad_2d::gfx_quad_2d(mws_sp<gfx> i_gi) : gfx_vxo(vx_info("a_v3_position, a_v3_normal, a_v2_tex_coord"), i_gi)
{
   anchor_type = e_top_left;
   tr = glm::vec2(0.f);
   a = 0;
   sc = glm::vec2(1.f);
   (*material)[MP_DEPTH_TEST] = false;
   (*material)[MP_DEPTH_WRITE] = false;
}
