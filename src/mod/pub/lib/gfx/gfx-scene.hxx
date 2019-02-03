#pragma once

#include "cpp-property.hxx"
#include "pfm-def.h"
#include "gfx-state.hxx"
#include "gfx-util.hxx"
#include <glm/inc.hpp>
#include <string>
#include <vector>

class gfx;
class gfx_scene;
class gfx_camera;
class gfx_vxo;


class gfx_transform
{
   //template <class host> class position_accessor : public vec3_accessor<host>
   //{
   //public:
   //	void set(host* ihost, const value_type& ivalue)
   //	{
   //		val = ivalue;
   //	}

   //	const value_type& get(host* ihost) const 
   //	{
   //		return val;
   //	}

   //private:
   //	friend host;
   //	value_type val;
   //};

   //template <class host> class orientation_accessor : public quat_accessor<host>
   //{
   //public:
   //	void set(host* ihost, const value_type& ivalue)
   //	{
   //		val = ivalue;
   //	}

   //	const value_type& get(host* ihost) const 
   //	{
   //		return val;
   //	}

   //private:
   //	friend host;
   //	value_type val;
   //};

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
   virtual e_gfx_obj_type get_type()const override;
   mws_sp<gfx_node> get_mws_sp();
   mws_sp<gfx_node> get_parent();
   mws_sp<gfx_node> get_root();
   mws_sp<gfx_scene> get_scene();
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent);
   virtual void draw_in_sync(mws_sp<gfx_camera> i_cam);
   virtual void draw_out_of_sync(mws_sp<gfx_camera> i_cam);
   virtual void attach(mws_sp<gfx_node> inode);
   virtual void detach();
   virtual void on_attach();
   virtual void on_detach();
   bool contains(const mws_sp<gfx_node> inode);
   mws_sp<gfx_node> find_node_by_name(const std::string& iname);
   // i_update_global_mx will be true when i_global_tf_mx has changed and so the subojects need to be updated
   virtual void update_recursive(const glm::mat4& i_global_tf_mx, bool i_update_global_mx);


   template <class host> class name_accessor : public string_accessor<host>
   {
   public:
      virtual void set(host* ihost, const std::string& ivalue)
      {
         if (ivalue != this->val)
         {
            this->val = ivalue;
         }
      }
      friend host;
   };
   def_string_prop(gfx_node, name_accessor) name;

   bool visible;

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
   gfx_scene(mws_sp<gfx> i_gi = nullptr);
   void init();
   virtual void update();
   void draw();
   void post_draw();

private:
   friend class gfx_node;
   void add_camera_node(mws_sp<gfx_camera> icamera);
   void remove_camera_node(mws_sp<gfx_camera> icamera);

   std::vector<mws_sp<gfx_camera> > camera_list;
   std::vector<gfx_param> plist;
};