#include "stdafx.h"

#include "gfx-quad-2d.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfm-gl.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using gfx_vxo_util::set_mesh_data;

void gfx_quad_2d::set_anchor(e_anchor_types ianchor_type)
{
   anchor_type = ianchor_type;
   set_translation(tr.x, tr.y);
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

void gfx_quad_2d::set_v_flip(bool iv_flip)
{
   float p = 0.5;
   std::vector<vx_fmt_p3f_n3f_t2f> tvertices_data;

   if (iv_flip)
   {
      // xyz, uv
      tvertices_data =
      {
         { { -p * dx,  p * dy, 0 },{ 0, 0, -1 },{ 0, 0 } },
         { { -p * dx, -p * dy, 0 },{ 0, 0, -1 },{ 0, 1 } },
         { { p * dx, -p * dy, 0 },{ 0, 0, -1 },{ 1, 1 } },
         { { p * dx,  p * dy, 0 },{ 0, 0, -1 },{ 1, 0 } },
      };
   }
   else
   {
      // xyz, uv
      tvertices_data =
      {
         { { -p * dx,  p * dy, 0 },{ 0, 0, -1 },{ 0, 1 } },
         { { -p * dx, -p * dy, 0 },{ 0, 0, -1 },{ 0, 0 } },
         { { p * dx, -p * dy, 0 },{ 0, 0, -1 },{ 1, 0 } },
         { { p * dx,  p * dy, 0 },{ 0, 0, -1 },{ 1, 1 } },
      };
   }


   const gfx_indices_type tindices_data[] =
   {
      //0, 1, 2, 0, 2, 3
      //2, 0, 1, 0, 2, 3,
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)tvertices_data.data(), sizeof(vx_fmt_p3f_n3f_t2f) * tvertices_data.size(), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}

void gfx_quad_2d::set_dimensions(float idx, float idy)
{
   dx = idx;
   dy = idy;
   gfx_plane::set_dimensions(dx, dy);
}

void gfx_quad_2d::set_tex_coord(glm::vec2 lt, glm::vec2 rt, glm::vec2 rb, glm::vec2 lb)
{
   float p = 0.5;
   const vx_fmt_p3f_n3f_t2f tvertices_data[] =
      // xyz, uv
   {
      { { -p * dx,  p * dy, 0 },{ 0, 0, -1 },{ lb.s, lb.t } },
      { { -p * dx, -p * dy, 0 },{ 0, 0, -1 },{ lt.s, lt.t } },
      { { p * dx, -p * dy, 0 },{ 0, 0, -1 },{ rt.s, rt.t } },
      { { p * dx,  p * dy, 0 },{ 0, 0, -1 },{ rb.s, rb.t } },
   };

   const gfx_indices_type tindices_data[] =
   {
      //0, 1, 2, 0, 2, 3
      //2, 0, 1, 0, 2, 3,
      1, 0, 2, 3, 2, 0,
   };

   set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(get_shared_ptr()));
}

gfx_quad_2d::gfx_quad_2d(std::shared_ptr<gfx> i_gi) : gfx_plane(i_gi)
{
   anchor_type = e_top_left;
   tr = glm::vec2(0.f);
   a = 0;
   sc = glm::vec2(1.f);
   (*material)[MP_DEPTH_TEST] = false;
   (*material)[MP_DEPTH_WRITE] = false;
}
