#pragma once

#include "pfm.hpp"
#include "data-sequence.hpp"
#include "gfx-scene.hpp"
#include "gfx-util.hpp"
#include "gfx-color.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>

class gfx;
class gfx_scene;
class gfx_vxo;
class gfx_shader;
class gfx_camera;
class gfx_camera_impl;


namespace seq_util
{
   inline float read_float(shared_ptr<rw_sequence> isq)
   {
      return isq->r.read_real32();
   }

   inline glm::vec2 read_vec2(shared_ptr<rw_sequence> isq)
   {
      glm::vec2 val;

      val.x = isq->r.read_real32();
      val.y = isq->r.read_real32();

      return val;
   }

   inline glm::vec3 read_vec3(shared_ptr<rw_sequence> isq)
   {
      glm::vec3 val;

      val.x = isq->r.read_real32();
      val.y = isq->r.read_real32();
      val.z = isq->r.read_real32();

      return val;
   }

   inline glm::vec4 read_vec4(shared_ptr<rw_sequence> isq)
   {
      glm::vec4 val;

      val.x = isq->r.read_real32();
      val.y = isq->r.read_real32();
      val.z = isq->r.read_real32();
      val.w = isq->r.read_real32();

      return val;
   }

   inline glm::quat read_quat(shared_ptr<rw_sequence> isq)
   {
      glm::quat val;

      val.x = isq->r.read_real32();
      val.y = isq->r.read_real32();
      val.z = isq->r.read_real32();
      val.w = isq->r.read_real32();

      return val;
   }

   inline void write_float(shared_ptr<rw_sequence> isq, float ival)
   {
      isq->w.write_real32(ival);
   }

   inline void write_vec2(shared_ptr<rw_sequence> isq, glm::vec2& ival)
   {
      isq->w.write_real32(ival.x);
      isq->w.write_real32(ival.y);
   }

   inline void write_vec3(shared_ptr<rw_sequence> isq, glm::vec3& ival)
   {
      isq->w.write_real32(ival.x);
      isq->w.write_real32(ival.y);
      isq->w.write_real32(ival.z);
   }

   inline void write_vec4(shared_ptr<rw_sequence> isq, glm::vec4& ival)
   {
      isq->w.write_real32(ival.x);
      isq->w.write_real32(ival.y);
      isq->w.write_real32(ival.z);
      isq->w.write_real32(ival.w);
   }

   inline void write_quat(shared_ptr<rw_sequence> isq, glm::quat& ival)
   {
      isq->w.write_real32(ival.x);
      isq->w.write_real32(ival.y);
      isq->w.write_real32(ival.z);
      isq->w.write_real32(ival.w);
   }
}


const std::string u_m4_model = "u_m4_model";
const std::string u_m4_view = "u_m4_view";
const std::string u_m4_view_inv = "u_m4_view_inv";
const std::string u_m4_model_view = "u_m4_model_view";
const std::string u_m4_projection = "u_m4_projection";
const std::string u_m4_model_view_proj = "u_m4_model_view_proj";


class gfx_rt_info
{
public:
   std::string color_buffer_id;
   std::string depth_buffer_id;
   std::string stenc_buffer_id;
};


class draw_context
{
public:
   draw_context(shared_ptr<gfx_camera> icam);
   shared_ptr<gfx_camera> get_cam() { return cam.lock(); }
   void draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness);
   void draw_texture(std::string tex_name, float ix, float iy, float iwidth, float iheight);

   weak_ptr<gfx_camera> cam;
   shared_ptr<gfx_vxo> line_mesh;
   shared_ptr<gfx_vxo> img_mesh;
};


class draw_op
{
public:
   virtual void read_data(shared_ptr<rw_sequence> seq) = 0;
   virtual void write_data(shared_ptr<rw_sequence> seq) = 0;
   virtual void draw(shared_ptr<draw_context> idc) = 0;
};


class gfx_camera : public gfx_node
{
public:
   enum e_projection_type
   {
      e_orthographic_proj,
      e_perspective_proj,
      e_custom_proj
   };

   static shared_ptr<gfx_camera> new_inst(std::shared_ptr<gfx> i_gi = nullptr);
   virtual e_gfx_obj_type get_type()const override;
   void clear_buffers();
   virtual void update();
   void update_glp_params(shared_ptr<gfx_vxo> imesh, shared_ptr<gfx_shader> glp);
   void draw_arc(glm::vec3 position, float radius, glm::quat orientation, float startAngle, float stopAngle, const glm::vec4& color, float precision, float thickness);
   void draw_axes(const glm::vec3& istart, float length, float thickness);
   void draw_box(glm::vec3 position, glm::vec3 size, glm::quat orientation, const glm::vec4& color, float thickness);
   void draw_circle(glm::vec3 position, float radius, glm::vec3 normal, const glm::vec4& color, float precision, float thickness);
   void draw_grid(glm::vec3 position, glm::vec3 size, glm::quat orientation, const glm::vec4& color, float precision, float thickness);
   void draw_image(shared_ptr<gfx_tex> img, float x, float y, float width = 0.f, float height = 0.f);
   void draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness);
   void draw_plane(glm::vec3 center, glm::vec3 look_at_dir, glm::vec2 size, const glm::vec4& color);
   void draw_point(glm::vec3 center, const glm::vec4& color, float thickness);
   void draw_sphere(glm::vec3 position, float radius, glm::quat orientation, const glm::vec4& color, float precision, float thickness);
   void draw_text_2d(glm::vec3 position, std::string text, const glm::vec4& color, glm::vec2 size, std::string fontName, glm::vec2 scale);
   void draw_mesh(shared_ptr<gfx_vxo> imesh);
   virtual void update_camera_state();

   def_string_prop(gfx_camera, string_accessor) camera_id;
   std::vector<gfx_rt_info> rt_list;
   bool enabled;
   float near_clip_distance;
   float far_clip_distance;
   float fov_y_deg;
   e_projection_type projection_type;
   int rendering_priority;
   std::string render_target;
   bool clear_color;
   gfx_color clear_color_value;
   bool clear_depth;
   bool clear_stencil;
   std::vector<mws_sp<gfx_vxo> > opaque_obj_list;
   std::vector<mws_sp<gfx_vxo> > translucent_obj_list;
   std::function<void(mws_sp<gfx_camera> i_cam, std::vector<mws_sp<gfx_vxo> >& opaque, std::vector<mws_sp<gfx_vxo> >& translucent)> sort_function;
   static std::function<void(mws_sp<gfx_camera>, std::vector<mws_sp<gfx_vxo> >&, std::vector<mws_sp<gfx_vxo> >&)> z_order_sort_function;

protected:
   friend class gfx_scene;
   friend class gfx_camera_impl;
   gfx_camera(std::shared_ptr<gfx> i_gi = nullptr);
   virtual void load(shared_ptr<gfx_camera> inst);
   virtual void update_camera_state_impl();
   shared_ptr<draw_context> draw_ctx;
   shared_ptr<rw_sequence> draw_ops;

public:
   glm::mat4 view_translation;
   glm::mat4 view_rotation;
   glm::mat4 camera;
   glm::mat4 view;
   glm::mat4 projection;
   shared_ptr<gfx_camera_impl> p;

   static int camera_idx;
};
