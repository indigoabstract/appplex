#pragma once

#include "ext/gfx-surface.hpp"
#include <string>
#include <vector>

class gfx;


class gfx_quad_2d : public gfx_plane
{
public:
   enum e_anchor_types
   {
      e_top_left,
      e_center,
      e_btm_center,
   };

   gfx_quad_2d(std::shared_ptr<gfx> i_gi = nullptr);
   void set_anchor(e_anchor_types ianchor_type);
   virtual glm::vec2 get_translation() const;
   virtual void set_translation(const glm::vec2& i_tr);
   virtual void set_translation(float ix, float iy);
   virtual float get_rotation() const;
   virtual void set_rotation(float ia);
   virtual glm::vec2 get_scale() const;
   virtual void set_scale(const glm::vec2& i_sc);
   virtual void set_scale(float ix, float iy);
   virtual void set_v_flip(bool iv_flip);
   virtual void set_dimensions(float idx, float idy) override;
   virtual void set_tex_coord(glm::vec2 lt, glm::vec2 rt, glm::vec2 rb, glm::vec2 lb);

protected:
   e_anchor_types anchor_type;
   float dx, dy;
   float a;
   glm::vec2 tr;
   glm::vec2 sc;
};


class gfx_2d_sprite : public gfx_quad_2d
{
public:
   gfx_2d_sprite(std::shared_ptr<gfx> gfx_inst) : gfx_quad_2d(gfx_inst)
   {
      sx_factor = sy_factor = 1.f;
      anchor_type = e_center;
   }

   bool is_hit(float x, float y)
   {
      glm::vec2 p(x, y);

      return point_inside_triangle(p, tl_ws, tr_ws, br_ws) || point_inside_triangle(p, tl_ws, br_ws, bl_ws);
   }

   float get_width()
   {
      return sc.x * sx_factor;
   }

   float get_height()
   {
      return sc.y * sy_factor;
   }

   glm::vec2 get_center()
   {
      return tr;
   }

   glm::vec2 get_top_left_ws()
   {
      return tl_ws;
   }

   glm::vec2 get_top_right_ws()
   {
      return tr_ws;
   }

   glm::vec2 get_btm_right_ws()
   {
      return br_ws;
   }

   glm::vec2 get_btm_left_ws()
   {
      return bl_ws;
   }

   void set_translation(float ix, float iy) override
   {
      float tsx = get_width();
      float tsy = get_height();
      tr.x = ix;
      tr.y = iy;

      switch (anchor_type)
      {
      case e_top_left:
         position = glm::vec3(tr.x + tsx / 2, tr.y + tsy / 2, position().z);
         break;

      case e_center:
         position = glm::vec3(tr.x, tr.y, position().z);
         break;

      case e_btm_center:
         position = glm::vec3(tr.x, tr.y - tsy / 2, position().z);
         break;
      }

      update_coord();
      //mws_print("pos: %f, %f\n", position.x, position.y);
   }

   void set_rotation(float ia) override
   {
      const float pi = glm::pi<float>();
      a = ia;

      if (a < 0)
      {
         a = 2 * pi + a;
      }
      else if (a > 2 * pi)
      {
         a = a - 2 * pi;
      }
      //mws_print("rot: %f\n", a);

      orientation = glm::rotate(glm::quat(), a, glm::vec3(0, 0, 1));
      update_coord();
   }

   void set_scale(const glm::vec2& i_sc) override
   {
      sc = i_sc;
      scaling = glm::vec3(get_width(), get_height(), 1.f);
      set_translation(tr.x, tr.y);
   }

   void set_scale(float ix, float iy) override
   {
      set_scale(glm::vec2(ix, iy));
   }

   void update_coord()
   {
      glm::vec3 p;

      p = orientation() * glm::vec3(-get_width() / 2, -get_height() / 2, 0.f);
      tl_ws = glm::vec2(tr.x + p.x, tr.y + p.y);
      p = orientation() * glm::vec3(get_width() / 2, -get_height() / 2, 0.f);
      tr_ws = glm::vec2(tr.x + p.x, tr.y + p.y);
      p = orientation() * glm::vec3(get_width() / 2, get_height() / 2, 0.f);
      br_ws = glm::vec2(tr.x + p.x, tr.y + p.y);
      p = orientation() * glm::vec3(-get_width() / 2, get_height() / 2, 0.f);
      bl_ws = glm::vec2(tr.x + p.x, tr.y + p.y);
   }

   bool point_inside_triangle(glm::vec2& s, glm::vec2& a, glm::vec2& b, glm::vec2& c)
   {
      float as_x = s.x - a.x;
      float as_y = s.y - a.y;
      bool s_ab = (b.x - a.x) * as_y - (b.y - a.y) * as_x > 0.f;

      if ((c.x - a.x) * as_y - (c.y - a.y) * as_x > 0.f == s_ab)
      {
         return false;
      }

      if ((c.x - b.x) * (s.y - b.y) - (c.y - b.y) * (s.x - b.x) > 0.f != s_ab)
      {
         return false;
      }

      return true;
   }

   float sx_factor;
   float sy_factor;
   glm::vec2 tl_ws;
   glm::vec2 tr_ws;
   glm::vec2 br_ws;
   glm::vec2 bl_ws;
};
