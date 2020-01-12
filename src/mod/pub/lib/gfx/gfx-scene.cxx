#include "stdafx.hxx"

#include "gfx-scene.hxx"
#include "gfx-state.hxx"
#include "gfx-camera.hxx"
#include "gfx-vxo.hxx"
#include "gfx.hxx"
#include "gfx-tex.hxx"
#include "gfx-util.hxx"
#include "gfx-shader.hxx"
#include "pfm-gl.h"
#include <glm/inc.hpp>


gfx_transform::gfx_transform() : position(this), orientation(this), scaling(this), transform_mx(this)
{
   position = glm::vec3(0.f);
   orientation = glm::quat(1.f, 0.f, 0.f, 0.f);
   scaling = glm::vec3(1.f);
}

glm::vec3 gfx_transform::get_forward_dir() { return orientation()* glm::vec3(0, 0, -1.f); }

glm::vec3 gfx_transform::get_up_dir() { return orientation()* glm::vec3(0, 1.f, 0); }

glm::vec3 gfx_transform::get_right_dir() { return orientation()* glm::vec3(1.f, 0, 0); }

void gfx_transform::look_at(glm::vec3 direction, glm::vec3 desiredUp) { orientation = gfx_util::look_at(direction, desiredUp); }

void gfx_transform::look_at_pos(glm::vec3 iposition, glm::vec3 desiredUp)
{
   glm::vec3 direction = iposition - position;
   orientation = gfx_util::look_at(direction, desiredUp);
}

const glm::mat4& gfx_transform::get_global_tf_mx() const { return global_tf_mx; }


gfx_node::gfx_node(mws_sp<gfx> i_gi) : gfx_obj(i_gi), name(this), visible(this) { node_type = regular_node; visible = true; }

gfx_node::~gfx_node() {}

gfx_obj::e_gfx_obj_type gfx_node::get_type()const { return e_node; }

mws_sp<gfx_node> gfx_node::get_mws_sp() { return std::static_pointer_cast<gfx_node>(get_inst()); }

mws_sp<gfx_node> gfx_node::get_parent() const { return parent.lock(); }

mws_sp<gfx_node> gfx_node::get_root() const { return root.lock(); }

mws_sp<gfx_scene> gfx_node::get_scene() const { return std::static_pointer_cast<gfx_scene>(root.lock()); }

void gfx_node::add_to_draw_list(const std::string & i_camera_id, std::vector<mws_sp<gfx_vxo> > & i_opaque, std::vector<mws_sp<gfx_vxo> > & i_translucent)
{
   for (auto it = children.begin(); it != children.end(); ++it)
   {
      if ((*it)->visible)
      {
         (*it)->add_to_draw_list(i_camera_id, i_opaque, i_translucent);
      }
   }
}

void gfx_node::draw_in_sync(mws_sp<gfx_camera> i_cam)
{
   for (auto it = children.begin(); it != children.end(); ++it)
   {
      (*it)->draw_in_sync(i_cam);
   }
}

void gfx_node::draw_out_of_sync(mws_sp<gfx_camera> i_cam)
{
   i_cam->update_camera_state();
   update_recursive(glm::mat4(1.f), true);

   for (auto it = children.begin(); it != children.end(); ++it)
   {
      (*it)->draw_in_sync(i_cam);
   }
}

void gfx_node::attach(mws_sp<gfx_node> inode)
{
   if (inode->parent.lock())
   {
      mws_signal_error("this node is already part of a hierarchy");
   }
   else
   {
      if (inode->node_type == camera_node)
      {
         mws_sp<gfx_camera> i_camera = static_pointer_cast<gfx_camera>(inode);
         root.lock()->add_camera_node(i_camera);
      }

      children.push_back(inode);
      inode->parent = get_mws_sp();
      inode->root = root;
      inode->on_attach();
   }
}

void gfx_node::detach()
{
   mws_sp<gfx_node> parent_node = parent.lock();

   if (parent_node)
   {
      if (node_type == camera_node)
      {
         mws_sp<gfx_camera> i_camera = static_pointer_cast<gfx_camera>(get_mws_sp());

         root.lock()->remove_camera_node(i_camera);
      }

      parent_node->children.erase(std::find(parent_node->children.begin(), parent_node->children.end(), get_mws_sp()));
      parent = mws_sp<gfx_node>();
      root = mws_sp<gfx_scene>();
      on_detach();
   }
   else
   {
      mws_signal_error("this node is not part of a hierarchy");
   }
}

void gfx_node::on_attach() {}

void gfx_node::on_detach() {}

bool gfx_node::contains(const mws_sp<gfx_node> inode)
{
   if (inode == get_mws_sp())
   {
      return true;
   }

   std::vector<mws_sp<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); ++it)
   {
      bool contains_node = (*it)->contains(inode);

      if (contains_node)
      {
         return true;
      }
   }

   return false;
}

mws_sp<gfx_node> gfx_node::find_node_by_name(const std::string & i_name)
{
   if (i_name == get_mws_sp()->name())
   {
      return get_mws_sp();
   }

   std::vector<mws_sp<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); ++it)
   {
      mws_sp<gfx_node> node = (*it)->find_node_by_name(i_name);

      if (node)
      {
         return node;
      }
   }

   return nullptr;
}

void gfx_node::update_recursive(const glm::mat4 & i_global_tf_mx, bool i_update_global_mx)
{
   if (!visible)
   {
      return;
   }

   bool update_tf_mx = false;

   if (transform_mx.value_changed())
   {
      glm::vec3 skew;
      glm::vec4 perspective;

      glm::decompose(transform_mx(), (glm::vec3&)scaling, (glm::quat&)orientation, (glm::vec3&)position, skew, perspective);
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

      if (scaling.value_changed())
      {
         scaling_mx = glm::scale(scaling.read());
         update_tf_mx = true;
      }

      if (update_tf_mx)
      {
         transform_mx = translation_mx * rotation_mx * scaling_mx;
      }
   }

   i_update_global_mx = i_update_global_mx || update_tf_mx;

   if (i_update_global_mx)
   {
      global_tf_mx = i_global_tf_mx * transform_mx.read();
   }

   for (auto c : children)
   {
      if (c->visible)
      {
         c->update_recursive(global_tf_mx, i_update_global_mx);
      }
   }
}

gfx_scene::gfx_scene(mws_sp<gfx> i_gi) : gfx_node(i_gi)
{
}

void gfx_scene::init()
{
   root = static_pointer_cast<gfx_scene>(get_mws_sp());
}

void gfx_scene::update()
{
   update_recursive(global_tf_mx, false);
}

void gfx_scene::draw()
{
   mws_sp<gfx_state> gl_st = gi()->get_gfx_state();
   struct pred
   {
      bool operator()(const mws_sp<gfx_camera> a, const mws_sp<gfx_camera> b) const
      {
         return a->rendering_priority < b->rendering_priority;
      }
   };

   plist.clear();
   plist.push_back({ gl::BLEND, gl::FALSE_GL });
   plist.push_back({ gl::CULL_FACE, gl::FALSE_GL });
   plist.push_back({ gl::DEPTH_TEST, gl::FALSE_GL });
   plist.push_back({ gl::DEPTH_WRITEMASK, gl::TRUE_GL });
   plist.push_back({ gl::DITHER, gl::TRUE_GL });
   plist.push_back({ gl::POLYGON_OFFSET_FILL, gl::FALSE_GL });
   plist.push_back({ gl::SCISSOR_TEST, gl::FALSE_GL });
   plist.push_back({ gl::STENCIL_TEST, gl::FALSE_GL });
   gl_st->set_state(&plist[0], plist.size());

   std::sort(camera_list.begin(), camera_list.end(), pred());

   for (auto it = camera_list.begin(); it != camera_list.end(); ++it)
   {
      mws_sp<gfx_camera> cam = *it;

      if (!cam->enabled)
      {
         continue;
      }

      const std::string& camera_id = cam->camera_id;
      gfx_uint clear_mask = 0;
      plist.clear();

      if (cam->clear_color)
      {
         glm::vec4 c = cam->clear_color_value.to_vec4();

         plist.push_back({ gl::COLOR_CLEAR_VALUE, c.r, c.g, c.b, c.a });
         clear_mask |= gl::COLOR_BUFFER_BIT_GL;
      }

      if (cam->clear_depth)
      {
         clear_mask |= gl::DEPTH_BUFFER_BIT_GL;
      }

      if (cam->clear_stencil)
      {
         clear_mask |= gl::STENCIL_BUFFER_BIT_GL;
      }

      if (clear_mask != 0)
      {
         plist.push_back({ gl::CLEAR_MASK, clear_mask });
      }

      mws_report_gfx_errs();
      if (!plist.empty())
      {
         gl_st->set_state(&plist[0], plist.size());
      }

      mws_report_gfx_errs();
      auto& opaque_obj_list = cam->opaque_obj_list;
      auto& translucent_obj_list = cam->translucent_obj_list;

      opaque_obj_list.clear();
      translucent_obj_list.clear();
      cam->update_camera_state();

      for (auto it = children.begin(); it != children.end(); ++it)
      {
         if ((*it)->visible)
         {
            (*it)->add_to_draw_list(camera_id, opaque_obj_list, translucent_obj_list);
         }
      }

      if (cam->sort_function)
      {
         cam->sort_function(cam, opaque_obj_list, translucent_obj_list);
      }

      // draw opaque objects first
      {
         for (auto it = opaque_obj_list.begin(); it != opaque_obj_list.end(); ++it)
         {
            mws_sp<gfx_vxo> mesh = *it;

            if (!mesh->visible)
            {
               continue;
            }

            gfx_material& mat = *mesh->get_material();
            mws_sp<gfx_shader> shader = mat.get_shader();

            if (shader)
            {
               mesh->draw_in_sync(cam);
            }
            else
            {
               mws_print("mesh object at [%p] has null shader\n", mesh.get());
            }
         }
      }

      // draw translucent objects after opaque ones
      {
         for (auto it = translucent_obj_list.begin(); it != translucent_obj_list.end(); ++it)
         {
            mws_sp<gfx_vxo> mesh = *it;

            if (!mesh->visible)
            {
               continue;
            }

            gfx_material& mat = *mesh->get_material();
            mws_sp<gfx_shader> shader = mat.get_shader();

            if (shader)
            {
               mesh->draw_in_sync(cam);
            }
            else
            {
               mws_print("mesh object at [%p] has null shader\n", mesh.get());
            }
         }
      }
   }
}

void gfx_scene::post_draw()
{
   for (auto it = camera_list.begin(); it != camera_list.end(); ++it)
   {
      mws_sp<gfx_camera> cam = *it;

      if (!cam->enabled)
      {
         continue;
      }

      cam->update_camera_state();
   }
}

void gfx_scene::add_camera_node(mws_sp<gfx_camera> i_camera)
{
   camera_list.push_back(i_camera);
}

void gfx_scene::remove_camera_node(mws_sp<gfx_camera> i_camera)
{
   camera_list.erase(std::find(camera_list.begin(), camera_list.end(), i_camera));
}
