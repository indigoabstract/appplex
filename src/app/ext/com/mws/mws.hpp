#pragma once

// mws - multi-widget-set (as in, opposed to, say, a one-widget-set :) )
#include "pfm.hpp"
#include "com/util/util.hpp"
#include "com/unit/input-ctrl.hpp"
#include "media/gfx/gfx-scene.hpp"
#include <unordered_map>
#include <string>
#include <vector>
#ifdef MWS_USES_EXCEPTIONS
#include <exception>
#endif


class unitctrl;
class linear_transition;
class ms_linear_transition;
class unit;
class unit;
class mws;
class mws_page_tab;
class mws_page;
class mws_page_item;
class mws_button;
class mws_list;
class mws_tree;
class mws_font;
class mws_camera;
class gfx_vxo;
class text_vxo;


const std::string MWS_EVT_MODEL_UPDATE = "mws-model-update";
const std::string MWS_EVT_PAGE_TRANSITION = "mws-page-transition";


class mws_rect
{
public:
   mws_rect()
   {
      set(0, 0, 0, 0);
   }

   mws_rect(float ix, float iy, float iw, float ih)
   {
      set(ix, iy, iw, ih);
   }

   void set(float ix, float iy, float iw, float ih)
   {
      x = ix;
      y = iy;
      w = iw;
      h = ih;
   }

   float x, y, w, h;
};


class mws_model : public enable_shared_from_this<mws_model>, public ia_node
{
public:
   virtual ~mws_model() {}
   mws_sp<mws_model> get_instance();

   virtual void receive(mws_sp<iadp> idp);
   virtual void notify_update();
   void set_view(mws_sp<mws> iview);
   mws_sp<mws> get_view();

protected:
   mws_model() {}

   mws_sp<ia_sender> sender_inst();

   mws_wp<mws> view;
};


class mws : public gfx_node, public ia_node
{
public:
   static mws_sp<mws> nwi();

   virtual void init() {}
   virtual void on_destroy() {}
   virtual ~mws() {}
   mws_sp<mws> get_instance();

   virtual gfx_obj::e_gfx_obj_type get_type()const override;
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent) override;
   virtual void attach(mws_sp<gfx_node> i_node) override;
   virtual void list_mws_children(std::vector<mws_sp<mws> >& i_mws_subobj_list);
   virtual void set_enabled(bool i_is_enabled);
   bool is_enabled()const;
   void set_visible(bool iis_visible);
   bool is_visible()const;
   void set_id(std::string iid);
   const std::string& get_id();
   virtual mws_sp<mws> find_by_id(const std::string& iid);
   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws> get_mws_parent();
   mws_sp<mws_page_tab> get_mws_root();
   virtual mws_sp<unit> get_unit();

   virtual void receive(mws_sp<iadp> idp);
   virtual void set_receive_handler(std::function<void(mws_sp<mws> i_mws, mws_sp<iadp> i_idp)> i_receive_handler);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   mws_rect get_pos();
   virtual float get_z();
   virtual void set_z(float i_z_position);

protected:
   mws(mws_sp<gfx> i_gi = nullptr);
   virtual void setup(){}

   bool enabled = true;
   bool is_opaque = true;
   mws_rect mws_r;
   mws_wp<mws_page_tab> mwsroot;
   mws_wp<mws_camera> mws_cam;
   std::function<void(mws_sp<mws> i_mws, mws_sp<iadp> i_idp)> receive_handler;

private:
   virtual mws_sp<ia_sender> sender_inst();

   std::string id;
};


class mws_page_tab : public mws
{
public:
   virtual ~mws_page_tab() {}
   static mws_sp<mws_page_tab> nwi(mws_sp<unit> i_u);
   virtual void add_to_draw_list(const std::string& i_camera_id, std::vector<mws_sp<gfx_vxo> >& i_opaque, std::vector<mws_sp<gfx_vxo> >& i_translucent) override;
   virtual void init();
   virtual void init_subobj();
   virtual void on_destroy();

   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws_page_tab> get_mws_page_tab_instance();
   virtual mws_sp<unit> get_unit();
   bool is_empty();
   mws_sp<text_vxo> get_text_vxo() const;

   virtual void receive(mws_sp<iadp> idp);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   virtual void on_resize();
   virtual void add_page(mws_sp<mws_page> i_page);
   template <typename T> mws_sp<T> new_page() { mws_sp<T> p(new T()); add_page(p); return p; }

   std::vector<mws_sp<mws_page> > page_tab;

protected:
   mws_page_tab(mws_sp<unit> iu);

private:
   friend class unitctrl;
   friend class mws_page;

   void add(mws_sp<mws_page> p);
   int get_page_index(mws_sp<mws_page> p);
   void new_instance_helper();

   mws_sp<text_vxo> tab_text_vxo;
   mws_wp<unit> u;
};


class mws_page : public mws
{
public:
   virtual ~mws_page() {}
   static mws_sp<mws_page> nwi(mws_sp<mws_page_tab> iparent);
   virtual void init();
   virtual void on_destroy();

   virtual mws_sp<mws> contains_id(const std::string& iid);
   virtual bool contains_mws(const mws_sp<mws> i_mws);
   mws_sp<mws_page> get_mws_page_instance();
   mws_sp<mws_page_tab> get_mws_page_parent();

   virtual void on_visibility_changed(bool iis_visible);
   virtual void on_show_transition(const mws_sp<linear_transition> itransition);
   virtual void on_hide_transition(const mws_sp<linear_transition> itransition);

   virtual void receive(mws_sp<iadp> idp);
   virtual void update_input_sub_mws(mws_sp<iadp> idp);
   virtual void update_input_std_behaviour(mws_sp<iadp> idp);
   virtual void update_state();
   virtual void update_view(mws_sp<mws_camera> g);
   mws_sp<mws> get_mws_at(int idx);

protected:
   mws_page();
   virtual void on_resize();
   std::vector<mws_sp<mws> > mws_subobj_list;

private:
   friend class mws_page_tab;
   friend class mws_page_item;

   static mws_sp<mws_page> new_standalone_instance();
   void add(mws_sp<mws_page_item> b);

   std::vector<mws_sp<mws_page_item> > mlist;
};


class mws_page_item : public mws
{
public:
   virtual ~mws_page_item() {}

   virtual void set_rect(const mws_rect& i_rect);
   virtual void set_size(float i_width, float i_height);
   mws_sp<mws_page> get_mws_page_item_parent();

protected:
   mws_page_item();
   void setup() override;

   void add_to_page();
};
