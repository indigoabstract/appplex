#include "stdafx.h"

#include "gfx-camera.hpp"
#include "gfx-vxo.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "gfx-state.hpp"
#include "gfx-color.hpp"
#include "pfm-gl.h"
#include <glm/inc.hpp>

draw_context::draw_context(shared_ptr<gfx_camera> icam)
{
   cam = icam;
   line_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord")));
   (*line_mesh)[MP_SHADER_NAME] = "c-o-shader";
   (*line_mesh)["u_v4_color"] = glm::vec4(0.f, 0, 1, 1.f);
   (*line_mesh)[MP_DEPTH_TEST] = true;
   (*line_mesh)[MP_DEPTH_WRITE] = true;
   (*line_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
   (*line_mesh)[MP_BLENDING] = MV_ALPHA;
   (*line_mesh)[MP_CULL_BACK] = false;
   (*line_mesh)[MP_CULL_FRONT] = false;
   //line_mesh->render_method = GLPT_POINTS;

   img_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord")));
   (*img_mesh)[MP_SHADER_NAME] = "basic-tex-shader";
   (*img_mesh)["u_s2d_tex"] = "";
   (*img_mesh)[MP_DEPTH_TEST] = true;
   (*img_mesh)[MP_DEPTH_WRITE] = true;
   (*img_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
   (*img_mesh)[MP_BLENDING] = MV_ALPHA;
   (*img_mesh)[MP_CULL_BACK] = false;
   (*img_mesh)[MP_CULL_FRONT] = false;
   //line_mesh->render_method = GLPT_POINTS;
}

void draw_context::draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness)
{
   shared_ptr<gfx_camera> cam = get_cam();
   glm::vec3 n = glm::normalize(start - finish);
   glm::vec3 bl;
   glm::vec3 tl;
   glm::vec3 tr;
   glm::vec3 br;

   if (cam->projection_type == gfx_camera::e_perspective_proj)
   {
      glm::vec3 start_cam_pos = start - cam->position();
      glm::vec3 finish_cam_pos = finish - cam->position();
      glm::vec3 plane_vect_start = glm::normalize(glm::cross(start - cam->position(), n));
      glm::vec3 plane_vect_finish = glm::normalize(glm::cross(finish - cam->position(), n));
      float start_thickness = thickness * glm::length(start_cam_pos) / 500.f;
      float finish_thickness = thickness * glm::length(finish_cam_pos) / 500.f;

      bl = start - plane_vect_start * start_thickness;
      tl = start + plane_vect_start * start_thickness;
      tr = finish + plane_vect_finish * finish_thickness;
      br = finish - plane_vect_finish * finish_thickness;
   }
   else if (cam->projection_type == gfx_camera::e_orthographic_proj)
   {
      glm::vec3 ortho_vect = glm::normalize(glm::cross(n, glm::vec3(0, 0, 1)));

      //thickness = thickness * glm::length(ortho_vect) / 500.f;
      bl = start - ortho_vect * thickness;
      br = start + ortho_vect * thickness;
      tr = finish + ortho_vect * thickness;
      tl = finish - ortho_vect * thickness;
   }

   const vx_fmt_p3f_t2f tvertices_data[] =
      // xyz, uv
   {
      { { bl.x, bl.y, bl.z, }, { 0, 0, } },
      { { tl.x, tl.y, tl.z, }, { 0, 1, } },
      { { tr.x, tr.y, tr.z, }, { 1, 1, } },
      { { br.x, br.y, br.z, }, { 1, 0, } },
   };

   const gfx_indices_type tindices_data[] =
   {
      1, 0, 2, 2, 0, 3,
      //1, 0, 2, 3, 2, 0,
      //2, 0, 1, 0, 2, 3,
   };

   gfx_vxo_util::set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(line_mesh));

   (*line_mesh)["u_v4_color"] = color;
   line_mesh->draw_in_sync(cam);
}

void draw_context::draw_texture(std::string tex_name, float ix, float iy, float iwidth, float iheight)
{
   auto tex = cam.lock()->gi()->tex.get_texture_by_name(tex_name);

   if (tex)
   {
      float tex_w = (iwidth > 0) ? iwidth : tex->get_width();
      float tex_h = (iheight > 0) ? iheight : tex->get_height();

      shared_ptr<gfx_camera> cam = get_cam();
      glm::vec3 bl;
      glm::vec3 tl;
      glm::vec3 tr;
      glm::vec3 br;

      bl = glm::vec3(ix, iy, 0.f);
      br = glm::vec3(ix + tex_w, iy, 0.f);
      tr = glm::vec3(ix + tex_w, iy + tex_h, 0.f);
      tl = glm::vec3(ix, iy + tex_h, 0.f);;

      const vx_fmt_p3f_t2f tvertices_data[] =
         // xyz, uv
      {
         { { bl.x, bl.y, bl.z, }, { 0, 0, } },
         { { tl.x, tl.y, tl.z, }, { 0, 1, } },
         { { tr.x, tr.y, tr.z, }, { 1, 1, } },
         { { br.x, br.y, br.z, }, { 1, 0, } },
      };

      const gfx_indices_type tindices_data[] =
      {
         1, 0, 2, 2, 0, 3,
      };

      gfx_vxo_util::set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(img_mesh));

      (*img_mesh)["u_s2d_tex"] = tex_name;
      img_mesh->draw_in_sync(cam);
   }
}


class draw_axes_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, const glm::vec3& istart, float ilength, float ithickness)
   {
      start = istart;
      length = ilength;
      thickness = ithickness;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      start = seq_util::read_vec3(seq);
      length = seq_util::read_float(seq);
      thickness = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, start);
      seq_util::write_float(seq, length);
      seq_util::write_float(seq, thickness);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      idc->draw_line(start, start + glm::vec3(1, 0, 0) * length, gfx_color::colors::red.to_vec4(), thickness);
      idc->draw_line(start, start + glm::vec3(0, 1, 0) * length, gfx_color::colors::green.to_vec4(), thickness);
      idc->draw_line(start, start + glm::vec3(0, 0, 1) * length, gfx_color::colors::blue.to_vec4(), thickness);
   }

   glm::vec3 start;
   float length;
   float thickness;
};


class draw_box_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, glm::vec3& iposition, glm::vec3& isize, glm::quat& iorientation, const glm::vec4& icolor, float ithickness)
   {
      position = iposition;
      size = isize;
      orientation = iorientation;
      color = icolor;
      thickness = ithickness;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      position = seq_util::read_vec3(seq);
      size = seq_util::read_vec3(seq);
      orientation = seq_util::read_quat(seq);
      color = seq_util::read_vec4(seq);
      thickness = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, position);
      seq_util::write_vec3(seq, size);
      seq_util::write_quat(seq, orientation);
      seq_util::write_vec4(seq, color);
      seq_util::write_float(seq, thickness);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      glm::vec3 points[8] =
      {
         glm::vec3(-1.f, -1, +1), glm::vec3(-1.f, +1, +1), glm::vec3(+1.f, +1, +1), glm::vec3(+1.f, -1, +1), // front
         glm::vec3(+1.f, -1, -1), glm::vec3(+1.f, +1, -1), glm::vec3(-1.f, +1, -1), glm::vec3(-1.f, -1, -1), // back
      };

      for (int k = 0; k < 8; k++)
      {
         points[k] = position + (points[k] * size * 0.5f * orientation);
      }

      idc->draw_line(points[0], points[1], color, thickness);
      idc->draw_line(points[1], points[2], color, thickness);
      idc->draw_line(points[2], points[3], color, thickness);
      idc->draw_line(points[3], points[0], color, thickness);

      idc->draw_line(points[4], points[5], color, thickness);
      idc->draw_line(points[5], points[6], color, thickness);
      idc->draw_line(points[6], points[7], color, thickness);
      idc->draw_line(points[7], points[4], color, thickness);

      idc->draw_line(points[0], points[7], color, thickness);
      idc->draw_line(points[1], points[6], color, thickness);
      idc->draw_line(points[2], points[5], color, thickness);
      idc->draw_line(points[3], points[4], color, thickness);
   }

   glm::vec3 position;
   glm::vec3 size;
   glm::quat orientation;
   glm::vec4 color;
   float thickness;
};


class draw_circle_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, glm::vec3& iposition, float iradius, glm::vec3& inormal, const glm::vec4& icolor, float iprecision, float ithickness)
   {
      position = iposition;
      radius = iradius;
      normal = inormal;
      color = icolor;
      precision = iprecision;
      thickness = ithickness;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      position = seq_util::read_vec3(seq);
      radius = seq_util::read_float(seq);
      normal = seq_util::read_vec3(seq);
      color = seq_util::read_vec4(seq);
      precision = seq_util::read_float(seq);
      thickness = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, position);
      seq_util::write_float(seq, radius);
      seq_util::write_vec3(seq, normal);
      seq_util::write_vec4(seq, color);
      seq_util::write_float(seq, precision);
      seq_util::write_float(seq, thickness);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
   }

   glm::vec3 position;
   float radius;
   glm::vec3 normal;
   glm::vec4 color;
   float precision;
   float thickness;
};


class draw_line_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, glm::vec3& istart, glm::vec3& ifinish, const glm::vec4& icolor, float ithickness)
   {
      start = istart;
      finish = ifinish;
      color = icolor;
      thickness = ithickness;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      start = seq_util::read_vec3(seq);
      finish = seq_util::read_vec3(seq);
      color = seq_util::read_vec4(seq);
      thickness = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, start);
      seq_util::write_vec3(seq, finish);
      seq_util::write_vec4(seq, color);
      seq_util::write_float(seq, thickness);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      idc->draw_line(start, finish, color, thickness);
   }

   glm::vec3 start;
   glm::vec3 finish;
   glm::vec4 color;
   float thickness;
};


class draw_image_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, shared_ptr<gfx_tex> img, float ix, float iy, float iwidth, float iheight)
   {
      name = img->get_name();
      x = ix;
      y = iy;
      width = iwidth;
      height = iheight;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      name = seq->r.read_string();
      x = seq_util::read_float(seq);
      y = seq_util::read_float(seq);
      width = seq_util::read_float(seq);
      height = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq->w.write_string(name);
      seq_util::write_float(seq, x);
      seq_util::write_float(seq, y);
      seq_util::write_float(seq, width);
      seq_util::write_float(seq, height);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      idc->draw_texture(name, x, y, width, height);
   }

   std::string name;
   float x;
   float y;
   float width;
   float height;
};


class draw_plane_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, glm::vec3& icenter, glm::vec3& ilook_at_dir, glm::vec2& isize, const glm::vec4& icolor)
   {
      center = icenter;
      look_at_dir = ilook_at_dir;
      size = isize;
      color = icolor;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      center = seq_util::read_vec3(seq);
      look_at_dir = seq_util::read_vec3(seq);
      size = seq_util::read_vec2(seq);
      color = seq_util::read_vec4(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, center);
      seq_util::write_vec3(seq, look_at_dir);
      seq_util::write_vec2(seq, size);
      seq_util::write_vec4(seq, color);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      shared_ptr<gfx_camera> cam = idc->get_cam();
      look_at_dir = glm::normalize(look_at_dir);
      glm::vec3 up;
      glm::vec3 right;

      glm::vec3 bl;
      glm::vec3 tl;
      glm::vec3 tr;
      glm::vec3 br;

      if (cam->projection_type == gfx_camera::e_perspective_proj)
      {
      }
      else if (cam->projection_type == gfx_camera::e_orthographic_proj)
      {
         float p = 0.5;
         float idx = size.x;
         float idy = size.y;

         bl = glm::vec3(-p * idx, p * idy, 0) + center;
         tl = glm::vec3(-p * idx, -p * idy, 0) + center;
         tr = glm::vec3(p * idx, -p * idy, 0) + center;
         br = glm::vec3(p * idx, p * idy, 0) + center;
      }

      const vx_fmt_p3f_t2f tvertices_data[] =
         // xyz, uv
      {
         { { bl.x, bl.y, bl.z, }, { 0, 0, } },
         { { tl.x, tl.y, tl.z, }, { 0, 1, } },
         { { tr.x, tr.y, tr.z, }, { 1, 1, } },
         { { br.x, br.y, br.z, }, { 1, 0, } },
      };

      const gfx_indices_type tindices_data[] =
      {
         //1, 0, 2, 2, 0, 3,
         1, 0, 2, 3, 2, 0,
         //2, 0, 1, 0, 2, 3,
      };

      shared_ptr<gfx_vxo> line_mesh = idc->line_mesh;

      gfx_vxo_util::set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(line_mesh));

      (*line_mesh)["u_v4_color"] = color;
      line_mesh->draw_in_sync(cam);
   }

   glm::vec3 center;
   glm::vec3 look_at_dir;
   glm::vec2 size;
   glm::vec4 color;
};


class draw_point_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, glm::vec3& icenter, const glm::vec4& icolor, float ithickness)
   {
      center = icenter;
      color = icolor;
      thickness = ithickness;
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      center = seq_util::read_vec3(seq);
      color = seq_util::read_vec4(seq);
      thickness = seq_util::read_float(seq);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq_util::write_vec3(seq, center);
      seq_util::write_vec4(seq, color);
      seq_util::write_float(seq, thickness);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      shared_ptr<gfx_camera> cam = idc->get_cam();
      glm::mat4& camera = cam->camera_mx;
      glm::vec3 up;
      glm::vec3 right;
      glm::vec3 bl;
      glm::vec3 tl;
      glm::vec3 tr;
      glm::vec3 br;

      if (cam->projection_type == gfx_camera::e_perspective_proj)
      {
         glm::vec3 center_cam_pos = center - cam->position();
         up = glm::vec3(camera[1].x, camera[1].y, camera[1].z);
         right = glm::normalize(glm::cross(center_cam_pos, up));
         //thickness = thickness * glm::length(center_cam_pos) / 1500.f;
      }
      else if (cam->projection_type == gfx_camera::e_orthographic_proj)
      {
         up = glm::vec3(camera[1].x, camera[1].y, camera[1].z);
         right = glm::vec3(camera[0].x, camera[0].y, camera[0].z);
      }

      bl = center - (up + right) * thickness;
      tl = center + (up - right) * thickness;
      tr = center + (up + right) * thickness;
      br = center - (up - right) * thickness;

      const vx_fmt_p3f_t2f tvertices_data[] =
         // xyz, uv
      {
         { { bl.x, bl.y, bl.z, }, { 0, 0, } },
         { { tl.x, tl.y, tl.z, }, { 0, 1, } },
         { { tr.x, tr.y, tr.z, }, { 1, 1, } },
         { { br.x, br.y, br.z, }, { 1, 0, } },
      };

      const gfx_indices_type tindices_data[] =
      {
         1, 0, 2, 2, 0, 3,
      };

      shared_ptr<gfx_vxo> line_mesh = idc->line_mesh;

      gfx_vxo_util::set_mesh_data((const uint8*)tvertices_data, sizeof(tvertices_data), tindices_data, sizeof(tindices_data), std::static_pointer_cast<gfx_vxo>(line_mesh));

      (*line_mesh)["u_v4_color"] = color;
      line_mesh->draw_in_sync(cam);
   }

   glm::vec3 center;
   glm::vec4 color;
   float thickness;
};


class draw_mesh_op : public draw_op
{
public:
   void push_data(shared_ptr<rw_sequence> seq, shared_ptr<gfx_vxo> imesh)
   {
      mesh = imesh.get();
      seq->w.write_pointer(this);
      write_data(seq);
   }

   virtual void read_data(shared_ptr<rw_sequence> seq)
   {
      seq->r.read_pointer(mesh);
   }

   virtual void write_data(shared_ptr<rw_sequence> seq)
   {
      seq->w.write_pointer(mesh);
   }

   virtual void draw(shared_ptr<draw_context> idc)
   {
      mesh->draw_in_sync(idc->cam.lock());
   }

   gfx_vxo* mesh;
};


class gfx_camera_impl
{
public:
   gfx_camera_impl()
   {
      line_mesh = shared_ptr<gfx_vxo>(new gfx_vxo(vx_info("a_v3_position, a_v2_tex_coord")));
      (*line_mesh)[MP_SHADER_NAME] = "c-o-shader";
      (*line_mesh)["u_v4_color"] = glm::vec4(0.f, 0, 1, 1.f);
      (*line_mesh)[MP_DEPTH_TEST] = true;
      (*line_mesh)[MP_DEPTH_WRITE] = true;
      (*line_mesh)[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      (*line_mesh)[MP_BLENDING] = MV_ALPHA;
      //(*line_mesh)[MP_CULL_BACK] = false;
      //(*line_mesh)[MP_CULL_FRONT] = false;
      //line_mesh->render_method = GLPT_POINTS;
   }

   void flush_commands(shared_ptr<gfx_camera> icam)
   {
      shared_ptr<rw_sequence> draw_ops = icam->draw_ops;
      int size = draw_ops->get_size();

      draw_ops->rewind();
      icam->draw_ctx->cam = icam;

      int read_pos = draw_ops->get_read_position();

      while (read_pos < size)
      {
         draw_op* d_o = 0;

         draw_ops->r.read_pointer(d_o);
         d_o->read_data(draw_ops);
         d_o->draw(icam->draw_ctx);
         read_pos = draw_ops->get_read_position();
      }

      draw_ops->reset();
   }

   weak_ptr<gfx_camera> camera;
   shared_ptr<gfx_vxo> line_mesh;

   draw_axes_op d_axes;
   draw_box_op d_box;
   draw_circle_op d_circle;
   draw_line_op d_line;
   draw_image_op d_img;
   draw_plane_op d_plane;
   draw_point_op d_point;
   draw_mesh_op d_mesh;
   std::weak_ptr<gfx_rt> last_rt;
};

void z_order_sort(mws_sp<gfx_camera> i_cam, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent)
{
   static auto z_sort = [](mws_sp<gfx_vxo> a, mws_sp<gfx_vxo> b)
   {
      auto& pos_0 = gfx_util::get_pos_from_tf_mx(a->get_global_tf_mx());
      auto& pos_1 = gfx_util::get_pos_from_tf_mx(b->get_global_tf_mx());

      return (pos_0.z < pos_1.z);
   };

   std::sort(i_opaque.begin(), i_opaque.end(), z_sort);
   std::sort(i_translucent.begin(), i_translucent.end(), z_sort);
}

std::function<void(mws_sp<gfx_camera>, std::vector<mws_sp<gfx_vxo> >&, std::vector<mws_sp<gfx_vxo> >&)> gfx_camera::z_order_sort_function = z_order_sort;
int gfx_camera::camera_idx = 0;

shared_ptr<gfx_camera> gfx_camera::nwi(std::shared_ptr<gfx> i_gi)
{
   shared_ptr<gfx_camera> inst(new gfx_camera(i_gi));
   inst->load(inst);
   return inst;
}

gfx_obj::e_gfx_obj_type gfx_camera::get_type()const
{
   return e_cam;
}

void gfx_camera::clear_buffers()
{
   gfx_bitfield bf = 0;

   if (clear_color)
   {
      auto cc = clear_color_value.to_vec4();

      bf |= GL_COLOR_BUFFER_BIT;
      glClearColor(cc.r, cc.g, cc.b, cc.a);
   }

   if (clear_depth)
   {
      bf |= GL_DEPTH_BUFFER_BIT;
   }

   if (clear_stencil)
   {
      bf |= GL_STENCIL_BUFFER_BIT;
   }

   if (bf != 0)
   {
      glClear(bf);
   }
}

void gfx_camera::update_glp_params(shared_ptr<gfx_vxo> imesh, shared_ptr<gfx_shader> glp)
{
   if (update_rt_cam_state)
   {
      auto crt_rt = gi()->rt.get_current_render_target();
      auto last_rt = p->last_rt.lock();

      if (last_rt != crt_rt)
      {
         update_camera_state_impl();
         p->last_rt = crt_rt;
      }
   }

   auto& tf_mx = imesh->get_global_tf_mx();
   glm::mat4 model_view = view_mx * tf_mx;
   glm::mat4 model_view_proj = projection_mx * model_view;

   glp->update_uniform(u_m4_model, glm::value_ptr(tf_mx));
   glp->update_uniform(u_m4_view, glm::value_ptr(view_mx));
   glp->update_uniform(u_m4_view_inv, glm::value_ptr(camera_mx));
   glp->update_uniform(u_m4_model_view, glm::value_ptr(model_view));
   glp->update_uniform(u_m4_projection, glm::value_ptr(projection_mx));
   glp->update_uniform(u_m4_model_view_proj, glm::value_ptr(model_view_proj));
}

void gfx_camera::draw_axes(const glm::vec3& istart, float length, float thickness)
{
   if (enabled)
   {
      p->d_axes.push_data(draw_ops, istart, length, thickness);
   }
}

void gfx_camera::draw_box(glm::vec3 iposition, glm::vec3 size, glm::quat orientation, const glm::vec4& color, float thickness)
{
   if (enabled)
   {
      p->d_box.push_data(draw_ops, iposition, size, orientation, color, thickness);
   }
}

void gfx_camera::draw_circle(glm::vec3 iposition, float radius, glm::vec3 normal, const glm::vec4& color, float precision, float thickness)
{
   if (enabled)
   {
      p->d_circle.push_data(draw_ops, iposition, radius, normal, color, precision, thickness);
   }
}

void gfx_camera::draw_image(shared_ptr<gfx_tex> img, float x, float y, float width, float height)
{
   if (enabled)
   {
      p->d_img.push_data(draw_ops, img, x, y, width, height);
   }
}

void gfx_camera::draw_line(glm::vec3 start, glm::vec3 finish, const glm::vec4& color, float thickness)
{
   if (enabled)
   {
      p->d_line.push_data(draw_ops, start, finish, color, thickness);
   }
}

void gfx_camera::draw_plane(glm::vec3 center, glm::vec3 look_at_dir, glm::vec2 size, const glm::vec4& color)
{
   if (enabled)
   {
      p->d_plane.push_data(draw_ops, center, look_at_dir, size, color);
   }
}

void gfx_camera::draw_point(glm::vec3 center, const glm::vec4& color, float thickness)
{
   if (enabled)
   {
      p->d_point.push_data(draw_ops, center, color, thickness);
   }
}

void gfx_camera::draw_mesh(shared_ptr<gfx_vxo> imesh)
{
   if (enabled)
   {
      p->d_mesh.push_data(draw_ops, imesh);
   }
   //shared_ptr<glsl_program> glp = imesh->get_material()->get_shader();

   //update_glp_params(imesh, glp);
   //imesh->draw_in_sync(static_pointer_cast<gfx_camera>(get_shared_ptr()));
}

gfx_camera::gfx_camera(std::shared_ptr<gfx> i_gi) : gfx_node(i_gi), camera_id(this)
{
   update_rt_cam_state = true;
   node_type = camera_node;
}

void gfx_camera::update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx)
{
   bool update_tf_mx = false;

   if (transform_mx.value_changed())
   {
      glm::vec3 skew;
      glm::vec4 perspective;

      glm::decompose(transform_mx(), (glm::vec3&)scaling(), (glm::quat&)orientation(), (glm::vec3&)position(), skew, perspective);
      update_tf_mx = true;
   }
   else
   {
      if (position.value_changed())
      {
         translation_mx = glm::translate(position.read());
         update_tf_mx = true;
      }

      if (orientation.value_changed())
      {
         rotation_mx = glm::toMat4(orientation.read());
         update_tf_mx = true;
      }

      //if (scaling.value_changed())
      //{
      //   scaling_mx = glm::scale(scaling.read());
      //   update_tf_mx = true;
      //}

      if (update_tf_mx)
      {
         camera_mx = translation_mx * rotation_mx;
         view_mx = glm::inverse(camera_mx);
         transform_mx = camera_mx;
      }
   }

   i_update_global_mx = i_update_global_mx || update_tf_mx;

   if (i_update_global_mx)
   {
      global_tf_mx = i_global_tf_mx * transform_mx.read();
   }

   for (auto c : children)
   {
      c->update_recursive(global_tf_mx, i_update_global_mx);
   }
}

void gfx_camera::load(shared_ptr<gfx_camera> inst)
{
   draw_ctx = shared_ptr<draw_context>(new draw_context(shared_ptr<gfx_camera>()));
   draw_ops = rw_sequence::nwi();

   p = shared_ptr<gfx_camera_impl>(new gfx_camera_impl());
   p->camera = inst;

   if (camera_idx == 0)
   {
      camera_id = "default";
   }
   else
   {
      char name[256];

      sprintf(name, "default%d", camera_idx);
      camera_id = name;
   }

   camera_idx++;
}

void gfx_camera::update_camera_state_impl()
{
   if (projection_type == e_perspective_proj)
   {
      float aspect = gi()->rt.get_render_target_width() / float(gi()->rt.get_render_target_height());
      projection_mx = glm::perspective(glm::radians(fov_y_deg), aspect, near_clip_distance, far_clip_distance);
   }
   else if (projection_type == e_orthographic_proj)
   {
      float left = 0;
      float right = gi()->rt.get_render_target_width();
      float bottom = gi()->rt.get_render_target_height();
      float top = 0;

      projection_mx = glm::ortho(left, right, bottom, top, near_clip_distance, far_clip_distance);
   }
}

void gfx_camera::update_camera_state()
{
   if (update_rt_cam_state)
   {
      update_camera_state_impl();
      shared_ptr<gfx_camera> inst = static_pointer_cast<gfx_camera>(get_shared_ptr());

      p->flush_commands(inst);
   }
}
