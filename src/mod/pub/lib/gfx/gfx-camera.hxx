#pragma once

#include "pfm-def.h"
#include "data-seqv.hxx"
#include "gfx-scene.hxx"
#include "gfx-util.hxx"
#include "gfx-color.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
#include <functional> 
#include <string>
#include <vector>
#include <unordered_map>
mws_pop_disable_all_warnings

class gfx;
class gfx_scene;
class gfx_vxo;
class gfx_shader;
class gfx_camera;
class gfx_camera_impl;
class gfx_material;
class data_seqv_rw_mem_ops;


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
   draw_context(mws_sp<gfx_camera> icam);
   mws_sp<gfx_camera> get_cam() { return cam.lock(); }
   void draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness);
   void draw_texture(std::string tex_name, float ix, float iy, float i_width, float i_height);

   mws_wp<gfx_camera> cam;
   mws_sp<gfx_vxo> line_mesh;
   mws_sp<gfx_vxo> img_mesh;
};


class draw_op
{
public:
   virtual void read_data(data_seqv_rw_mem_ops& seq) = 0;
   virtual void write_data(data_seqv_rw_mem_ops& seq) = 0;
   virtual void draw(mws_sp<draw_context> idc) = 0;
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

   static mws_sp<gfx_camera> nwi(mws_sp<gfx> i_gi = nullptr);
   static mws_sp<gfx_camera> nwi_orthographic
   (
      mws_sp<gfx> i_gi = nullptr, const std::string& i_camera_id = "", float i_near_clip_distance = -100.f,
      float i_far_clip_distance = 100.f, bool i_clear_color = true, gfx_color i_clear_color_value = gfx_color::colors::black
   );
   static mws_sp<gfx_camera> nwi_perspective
   (
      mws_sp<gfx> i_gi = nullptr, const std::string& i_camera_id = "", float i_near_clip_distance = -100.f, float i_far_clip_distance = 100.f,
      float i_fov_y_deg = 60.f, bool i_clear_color = true, gfx_color i_clear_color_value = gfx_color::colors::black
   );
   virtual e_gfx_obj_type get_type()const override;
   void clear_buffers();
   void update_glp_params(mws_sp<gfx_vxo> imesh, mws_sp<gfx_shader> glp);
   void draw_arc(glm::vec3 position, float radius, glm::quat orientation, float startAngle, float stopAngle, const glm::vec4& color, float precision, float thickness);
   void draw_axes(const glm::vec3& istart, float length, float thickness);
   void draw_box(glm::vec3 position, glm::vec3 size, glm::quat orientation, const glm::vec4& color, float thickness);
   void draw_circle(glm::vec3 position, float radius, glm::vec3 normal, const glm::vec4& color, float precision, float thickness);
   void draw_grid(glm::vec3 position, glm::vec3 size, glm::quat orientation, const glm::vec4& color, float precision, float thickness);
   void draw_image(mws_sp<gfx_tex> img, float x, float y, float width = 0.f, float height = 0.f);
   void draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness);
   void draw_plane(glm::vec3 center, glm::vec3 look_at_dir, glm::vec2 size, const glm::vec4& color);
   void draw_point(glm::vec3 center, const glm::vec4& color, float thickness);
   void draw_sphere(glm::vec3 position, float radius, glm::quat orientation, const glm::vec4& color, float precision, float thickness);
   void draw_text_2d(glm::vec3 position, std::string text, const glm::vec4& color, glm::vec2 size, std::string fontName, glm::vec2 scale);
   void draw_mesh(mws_sp<gfx_vxo> imesh);
   virtual void update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx) override;
   virtual void update_camera_state();

   // if set, use this material for rendering, instead of each object's material
   mws_sp<gfx_material> overriding_mat;
   bool update_rt_cam_state;
   def_string_prop(gfx_camera, string_accessor) camera_id;
   std::vector<gfx_rt_info> rt_list;
   bool enabled = true;
   float near_clip_distance = -100.f;
   float far_clip_distance = 100.f;
   float fov_y_deg = 60.f;
   e_projection_type projection_type = e_perspective_proj;
   int rendering_priority = 0;
   std::string render_target = "";
   bool clear_color = false;
   gfx_color clear_color_value = gfx_color::colors::black;
   bool clear_depth = true;
   bool clear_stencil = true;
   std::vector<mws_sp<gfx_vxo> > opaque_obj_list;
   std::vector<mws_sp<gfx_vxo> > translucent_obj_list;
   std::function<void(mws_sp<gfx_camera> i_cam, std::vector<mws_sp<gfx_vxo> >& opaque, std::vector<mws_sp<gfx_vxo> >& translucent)> sort_function;
   static std::function<void(mws_sp<gfx_camera>, std::vector<mws_sp<gfx_vxo> >&, std::vector<mws_sp<gfx_vxo> >&)> z_order_sort_function;

protected:
   friend class gfx_scene;
   friend class gfx_camera_impl;
   gfx_camera(mws_sp<gfx> i_gi = nullptr);
   virtual void load(mws_sp<gfx_camera> inst);
   virtual void update_camera_state_impl();
   mws_sp<draw_context> draw_ctx;
   data_seqv_rw_mem_ops draw_ops;

public:
   glm::mat4 camera_mx = glm::mat4(1.f);
   glm::mat4 view_mx = glm::mat4(1.f);
   glm::mat4 projection_mx;
   mws_sp<gfx_camera_impl> p;

   static int camera_idx;
};


namespace seq_util
{
   inline glm::vec2 read_vec2(data_seqv_rw_mem_ops& i_sq)
   {
      glm::vec2 val;

      val.x = i_sq.r.read_f32();
      val.y = i_sq.r.read_f32();

      return val;
   }

   inline glm::vec3 read_vec3(data_seqv_rw_mem_ops& i_sq)
   {
      glm::vec3 val;

      val.x = i_sq.r.read_f32();
      val.y = i_sq.r.read_f32();
      val.z = i_sq.r.read_f32();

      return val;
   }

   inline glm::vec4 read_vec4(data_seqv_rw_mem_ops& i_sq)
   {
      glm::vec4 val;

      val.x = i_sq.r.read_f32();
      val.y = i_sq.r.read_f32();
      val.z = i_sq.r.read_f32();
      val.w = i_sq.r.read_f32();

      return val;
   }

   inline glm::quat read_quat(data_seqv_rw_mem_ops& i_sq)
   {
      glm::quat val;

      val.x = i_sq.r.read_f32();
      val.y = i_sq.r.read_f32();
      val.z = i_sq.r.read_f32();
      val.w = i_sq.r.read_f32();

      return val;
   }

   inline void write_vec2(data_seqv_rw_mem_ops& i_sq, glm::vec2& i_val)
   {
      i_sq.w.write_f32(i_val.x);
      i_sq.w.write_f32(i_val.y);
   }

   inline void write_vec3(data_seqv_rw_mem_ops& i_sq, glm::vec3& i_val)
   {
      i_sq.w.write_f32(i_val.x);
      i_sq.w.write_f32(i_val.y);
      i_sq.w.write_f32(i_val.z);
   }

   inline void write_vec4(data_seqv_rw_mem_ops& i_sq, glm::vec4& i_val)
   {
      i_sq.w.write_f32(i_val.x);
      i_sq.w.write_f32(i_val.y);
      i_sq.w.write_f32(i_val.z);
      i_sq.w.write_f32(i_val.w);
   }

   inline void write_quat(data_seqv_rw_mem_ops& i_sq, glm::quat& i_val)
   {
      i_sq.w.write_f32(i_val.x);
      i_sq.w.write_f32(i_val.y);
      i_sq.w.write_f32(i_val.z);
      i_sq.w.write_f32(i_val.w);
   }
}
