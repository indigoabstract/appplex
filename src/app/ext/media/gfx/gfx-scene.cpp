#include "stdafx.h"

#include "gfx-scene.hpp"
#include "gfx-state.hpp"
#include "gfx-camera.hpp"
#include "gfx-vxo.hpp"
#include "gfx.hpp"
#include "gfx-tex.hpp"
#include "gfx-util.hpp"
#include "gfx-shader.hpp"
#include "pfmgl.h"


glm::vec3 gfx_transform::get_forward_dir()
{
   return orientation() * glm::vec3(0, 0, -1.f);
}

glm::vec3 gfx_transform::get_up_dir()
{
   return orientation() * glm::vec3(0, 1.f, 0);
}

glm::vec3 gfx_transform::get_right_dir()
{
   return orientation() * glm::vec3(1.f, 0, 0);
}

void gfx_transform::look_at(glm::vec3 direction, glm::vec3 desiredUp)
{
   orientation = gfx_util::look_at(direction, desiredUp);
}

void gfx_transform::look_at_pos(glm::vec3 iposition, glm::vec3 desiredUp)
{
   glm::vec3 direction = iposition - position();

   orientation = gfx_util::look_at(direction, desiredUp);
}


gfx_node::gfx_node(std::shared_ptr<gfx> i_gi) : gfx_obj(i_gi), name(this)
{
   node_type = regular_node;
   visible = true;
}

gfx_obj::e_gfx_obj_type gfx_node::get_type()const
{
   return e_gfx_node;
}

std::shared_ptr<gfx_node> gfx_node::get_shared_ptr()
{
   return std::static_pointer_cast<gfx_node>(get_inst());
}

std::shared_ptr<gfx_node> gfx_node::get_parent()
{
   return parent.lock();
}

std::shared_ptr<gfx_node> gfx_node::get_root()
{
   return root.lock();
}

std::shared_ptr<gfx_scene> gfx_node::get_scene()
{
   return std::static_pointer_cast<gfx_scene>(root.lock());
}

void gfx_node::add_to_draw_list(const std::string& icamera_id, std::vector<shared_ptr<gfx_vxo> >& idraw_list)
{
}

void gfx_node::update()
{
   std::vector<shared_ptr<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); it++)
   {
      (*it)->update();
   }
}

void gfx_node::attach(shared_ptr<gfx_node> inode)
{
   if (inode->parent.lock())
   {
      trx("this node is already part of a hierarchy");
   }
   else
   {
      if (inode->node_type == camera_node)
      {
         shared_ptr<gfx_camera> icamera = static_pointer_cast<gfx_camera>(inode);
         root.lock()->add_camera_node(icamera);
      }

      children.push_back(inode);
      inode->parent = get_shared_ptr();
      inode->root = root;
   }
}

void gfx_node::detach()
{
   shared_ptr<gfx_node> parent_node = parent.lock();

   if (parent_node)
   {
      if (node_type == camera_node)
      {
         shared_ptr<gfx_camera> icamera = static_pointer_cast<gfx_camera>(get_shared_ptr());

         root.lock()->remove_camera_node(icamera);
      }

      parent_node->children.erase(std::find(parent_node->children.begin(), parent_node->children.end(), get_shared_ptr()));
      parent = shared_ptr<gfx_node>();
      root = shared_ptr<gfx_scene>();
   }
   else
   {
      trx("this node is not part of a hierarchy");
   }
}

bool gfx_node::contains(const shared_ptr<gfx_node> inode)
{
   if (inode == get_shared_ptr())
   {
      return true;
   }

   std::vector<shared_ptr<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); it++)
   {
      bool contains_node = (*it)->contains(inode);

      if (contains_node)
      {
         return true;
      }
   }

   return false;
}

shared_ptr<gfx_node> gfx_node::find_node_by_name(const std::string& iname)
{
   if (iname == get_shared_ptr()->name())
   {
      return get_shared_ptr();
   }

   std::vector<shared_ptr<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); it++)
   {
      shared_ptr<gfx_node> node = (*it)->find_node_by_name(iname);

      if (node)
      {
         return node;
      }
   }

   return shared_ptr<gfx_node>();
}

gfx_scene::gfx_scene(std::shared_ptr<gfx> i_gi) : gfx_node(i_gi)
{
}

void gfx_scene::init()
{
   root = static_pointer_cast<gfx_scene>(get_shared_ptr());
}

void gfx_scene::draw()
{
   shared_ptr<gfx_state> gl_st = gfx::get_gfx_state();
   struct pred
   {
      bool operator()(const shared_ptr<gfx_camera> a, const shared_ptr<gfx_camera> b) const
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
   //plist.push_back({ gl::MULTISAMPLE, gl::TRUE_GL });
   plist.push_back({ gl::POLYGON_OFFSET_FILL, gl::FALSE_GL });
   plist.push_back({ gl::SCISSOR_TEST, gl::FALSE_GL });
   plist.push_back({ gl::STENCIL_TEST, gl::FALSE_GL });
   gl_st->set_state(&plist[0], plist.size());

   std::sort(camera_list.begin(), camera_list.end(), pred());
   std::vector<shared_ptr<gfx_camera> >::iterator it = camera_list.begin();

   for (; it != camera_list.end(); it++)
   {
      shared_ptr<gfx_camera> cam = *it;

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

      if (!plist.empty())
      {
         gl_st->set_state(&plist[0], plist.size());
      }

      cam->update_camera_state();

      std::vector<shared_ptr<gfx_vxo> > draw_list;

      for (auto it = children.begin(); it != children.end(); it++)
      {
         (*it)->add_to_draw_list(camera_id, draw_list);
      }

      auto it3 = draw_list.begin();

      for (; it3 != draw_list.end(); it3++)
      {
         shared_ptr<gfx_vxo> mesh = *it3;

         if (!mesh->visible)
         {
            continue;
         }

         gfx_material& mat = *mesh->get_material();
         shared_ptr<gfx_shader> shader = mat.get_shader();

         if (shader)
         {
            //mesh->push_material_params();
            mesh->render_mesh(cam);
         }
         else
         {
            vprint("mesh object at [%p] has null shader\n", mesh.get());
         }
      }
   }
}

void gfx_scene::update()
{
   std::vector<shared_ptr<gfx_node> >::iterator it = children.begin();

   for (; it != children.end(); it++)
   {
      (*it)->update();
   }
}

void gfx_scene::add_camera_node(shared_ptr<gfx_camera> icamera)
{
   camera_list.push_back(icamera);
}

void gfx_scene::remove_camera_node(shared_ptr<gfx_camera> icamera)
{
   camera_list.erase(std::find(camera_list.begin(), camera_list.end(), icamera));
}
