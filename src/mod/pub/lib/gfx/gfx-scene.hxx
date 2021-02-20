#pragma once

#include "cpp-property.hxx"
#include "pfm-def.h"
#include "gfx-obj.hxx"
#include "gfx-state.hxx"
#include "gfx-util.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
#include <functional>
#include <string>
#include <vector>
mws_pop_disable_all_warnings


class gfx;
class gfx_scene;
class gfx_camera;
class gfx_vxo;
class mws_mod;


class gfx_transform
{
public:
   gfx_transform();

   glm::vec3 get_forward_dir();
   glm::vec3 get_up_dir();
   glm::vec3 get_right_dir();
   void look_at(glm::vec3 direction, glm::vec3 desiredUp);
   void look_at_pos(glm::vec3 iposition, glm::vec3 desiredUp);
   const glm::mat4& get_global_tf_mx() const;

   def_number_prop(gfx_transform, vec3_accessor) position;
   def_quat_prop(gfx_transform, quat_accessor) orientation;
   def_number_prop(gfx_transform, vec3_accessor) scaling;
   def_mat_prop(gfx_transform, mat4_accessor) transform_mx;

protected:
   glm::mat4 global_tf_mx = glm::mat4(1.f);
   glm::mat4 translation_mx = glm::mat4(1.f);
   glm::mat4 rotation_mx = glm::mat4(1.f);
   glm::mat4 scaling_mx = glm::mat4(1.f);
};


class gfx_node : public gfx_obj, public gfx_transform
{
public:
   enum e_node_type
   {
      camera_node,
      regular_node,
   };

   gfx_node(mws_sp<gfx> i_gi);
   virtual ~gfx_node();
   virtual e_gfx_obj_type get_type() const override;
   mws_sp<gfx_node> get_mws_sp();
   mws_sp<gfx_node> get_parent() const;
   mws_sp<gfx_node> get_root() const;
   mws_sp<gfx_scene> get_scene() const;
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent);
   virtual void draw_in_sync(mws_sp<gfx_camera> i_cam);
   virtual void draw_out_of_sync(mws_sp<gfx_camera> i_cam);
   virtual void attach(mws_sp<gfx_node> inode);
   virtual void detach();
   virtual void on_attach();
   virtual void on_detach();
   virtual void on_visibility_changed_handler(bool /*i_is_visible*/) {}
   bool contains(const mws_sp<gfx_node> inode);
   mws_sp<gfx_node> find_node_by_name(const std::string& i_name);
   // i_update_global_mx will be true when i_global_tf_mx has changed and so the subojects need to be updated
   virtual void update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx);
   /** if not null, this function will be called instead of on_visibility_changed_handler() */
   std::function<void(bool i_visible)> on_visibility_changed;

   template <class host> class name_accessor : public string_accessor<host>
   {
   public:
      virtual void set(host* /*i_host*/, const std::string& i_value) override
      {
         if (i_value != this->val)
         {
            this->val = i_value;
         }
      }
   };
   def_string_prop(gfx_node, name_accessor) name;

   class visible_accessor : public bool_accessor<gfx_node>
   {
   public:
      virtual void set(gfx_node* i_host, const bool& i_value) override
      {
         if (i_value != this->val)
         {
            this->val = i_value;
            val_changed = true;

            if (i_host->on_visibility_changed)
            {
               i_host->on_visibility_changed(i_value);
            }
            else
            {
               i_host->on_visibility_changed_handler(i_value);
            }
         }
      }
   };
   def_prop(gfx_node, visible_accessor) visible;

protected:
   e_node_type node_type;
   std::vector<mws_sp<gfx_node> > children;

private:
   friend class gfx_scene;
   mws_wp<gfx_node> parent;
   mws_wp<gfx_scene> root;
};


class gfx_scene : public gfx_node
{
public:
   gfx_scene(mws_sp<mws_mod> i_mod, mws_sp<gfx> i_gi = nullptr);
   void init();
   virtual void update();
   void draw();
   void post_draw();
   mws_sp<mws_mod> get_mod() const;

private:
   friend class gfx_node;
   void add_camera_node(mws_sp<gfx_camera> i_camera);
   void remove_camera_node(mws_sp<gfx_camera> i_camera);

   std::vector<mws_sp<gfx_camera> > camera_list;
   std::vector<gfx_param> plist;
   mws_wp<mws_mod> mod;
};
