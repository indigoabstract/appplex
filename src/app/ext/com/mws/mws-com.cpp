#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS

#include "mws-com.hpp"
#include "mws-camera.hpp"
#include "mws-font.hpp"
#include "unit.hpp"
#include "com/util/util.hpp"
#include "com/unit/transitions.hpp"
#include "gfx-quad-2d.hpp"
#include "gfx.hpp"
#include "gfx-util.hpp"
#include "glm/vec2.hpp"
#include <algorithm>

using std::string;
using std::vector;


shared_ptr<mws_panel> mws_panel::nwi()
{
   auto inst = std::shared_ptr<mws_panel>(new mws_panel());
   inst->vxo = std::make_shared<gfx_quad_2d>();
   inst->attach(inst->vxo);

   {
      auto& rvxo = *inst->vxo;
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = "c-o-shader";
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(0, 1.f, 0, 1);
      rvxo.set_dimensions(1, 1);
   }

   return inst;
}

void mws_panel::set_rect(const mws_rect& i_rect)
{
   vxo->set_translation(i_rect.x, i_rect.y);
   vxo->set_scale(i_rect.w, i_rect.h);
   mws_r = i_rect;
}

mws_sp<gfx_quad_2d> mws_panel::get_vxo()
{
   return vxo;
}


std::shared_ptr<mws_img_btn> mws_img_btn::nwi()
{
   auto inst = std::shared_ptr<mws_img_btn>(new mws_img_btn());
   inst->vxo = std::make_shared<gfx_quad_2d>();
   inst->attach(inst->vxo);

   {
      auto& rvxo = *inst->vxo;
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = "basic-tex-shader";
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_s2d_tex"] = "";
      rvxo.set_dimensions(1, 1);
      rvxo.set_anchor(gfx_quad_2d::e_center);
      rvxo[MP_BLENDING] = MV_ALPHA;
   }

   return inst;
}

void mws_img_btn::set_rect(const mws_rect& i_rect)
{
   //vxo->set_translation(i_rect.x, i_rect.y);
   position = glm::vec3(i_rect.x, i_rect.y, position().z);
   vxo->set_scale(i_rect.w, i_rect.h);
   mws_r = i_rect;
}

void mws_img_btn::set_img_name(std::string i_img_name)
{
   //gfx_tex_params prm;
   //mws_sp<gfx_tex> tex;

   //prm.wrap_s = prm.wrap_t = gfx_tex_params::e_twm_clamp_to_edge;
   //prm.max_anisotropy = 0.f;
   ////prm.min_filter = gfx_tex_params::e_tf_linear_mipmap_linear;
   ////prm.mag_filter = gfx_tex_params::e_tf_linear;
   //prm.min_filter = gfx_tex_params::e_tf_linear;
   //prm.mag_filter = gfx_tex_params::e_tf_linear;
   //prm.gen_mipmaps = false;

   //tex = gi()->tex.new_tex_2d(i_img_name, &prm);
   (*vxo)["u_s2d_tex"] = i_img_name;
}

void mws_img_btn::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      if (!is_hit(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y))
      {
         return;
      }

      auto type = ts->get_type();

      //mws_print("evt type [%d]\n", type);
      switch (type)
      {
      case touch_sym_evt::TS_PRESSED:
         on_click();
         ts->process();
         break;
      }
   }
}

bool mws_img_btn::is_hit(float x, float y)
{
   auto& tf = get_vxo()->get_global_tf_mx();
   auto& pos = gfx_util::get_pos_from_tf_mx(tf);
   //auto& scale = gfx_util::get_scale_from_tf_mx(tf);
   //bool hit = is_inside_box(x, y, pos.x - scale.x / 2, pos.y - scale.y / 2, scale.x, scale.y);
   bool hit = is_inside_box(x, y, pos.x - mws_r.w / 2, pos.y - mws_r.h / 2, mws_r.w, mws_r.h);

   return hit;
}

void mws_img_btn::on_click()
{
   if (on_click_handler)
   {
      on_click_handler(std::static_pointer_cast<mws_img_btn>(get_instance()));
   }
}

void mws_img_btn::set_on_click_handler(std::function<void(mws_sp<mws_img_btn> i_img_btn)> i_on_click_handler)
{
   on_click_handler = i_on_click_handler;
}

mws_sp<gfx_quad_2d> mws_img_btn::get_vxo()
{
   return vxo;
}


mws_button::mws_button(shared_ptr<mws_page> iparent)
{
   color = gfx_color::colors::black;
   font = mws_font::new_inst(24.f);
   font->set_color(gfx_color::colors::white);
   set_text("n/a");
}

shared_ptr<mws_button> mws_button::new_instance(shared_ptr<mws_page> iparent)
{
   shared_ptr<mws_button> u(new mws_button(iparent));
   u->add_to_page();
   return u;
}

shared_ptr<mws_button> mws_button::new_shared_instance(mws_button* newButtonClassInstance)
{
   shared_ptr<mws_button> u(newButtonClassInstance);
   u->add_to_page();
   return u;
}

void mws_button::init(mws_rect i_rect, int iColor, string iText)
{
   mws_r.x = i_rect.x - i_rect.w / 2;
   mws_r.y = i_rect.y - i_rect.h / 2;
   mws_r.w = i_rect.w;
   mws_r.h = i_rect.h;

   color = gfx_color(iColor);

   set_text(iText);
}

void mws_button::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      if (!is_hit(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y))
      {
         return;
      }

      switch (ts->get_type())
      {
      case touch_sym_evt::TS_PRESSED:
         on_click();
         ts->process();
         break;
      }
   }
}

bool mws_button::is_hit(float x, float y)
{
   return false;
}

void mws_button::on_click() {}
void mws_button::update_state() {}

void mws_button::update_view(shared_ptr<mws_camera> g)
{
   //if(visible && mws_r.w > 0 && mws_r.h > 0)
   //{
   //	g->setColor(color.argb());
   //	g->fillRect(mws_r.x, mws_r.y, mws_r.w, mws_r.h);
   //	//draw_bar(g, mws_r.x, mws_r.y, mws_r.w, mws_r.h, color);
   //	g->drawText(text, mws_r.x + 10, mws_r.y + mws_r.h / 2 - 10, font);
   //}
}

void mws_button::set_color(const gfx_color& icolor)
{
   color = icolor;
}

void mws_button::set_text(string iText)
{
   text = iText;
}


std::shared_ptr<mws_slider> mws_slider::nwi()
{
   auto inst = std::shared_ptr<mws_slider>(new mws_slider());
   inst->slider_bar = std::make_shared<gfx_quad_2d>();
   inst->slider_ball = std::make_shared<gfx_quad_2d>();
   inst->attach(inst->slider_bar);
   inst->attach(inst->slider_ball);
   inst->set_z(0.f);

   // slider bar
   {
      auto& rvxo = *std::static_pointer_cast<gfx_quad_2d>(inst->slider_bar);
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = "c-o-shader";
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(0.75f);
      rvxo.set_dimensions(1, 1);
   }

   // slider ball
   {
      auto& rvxo = *std::static_pointer_cast<gfx_quad_2d>(inst->slider_ball);
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = "c-o-shader";
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(1.f, 1.f, 1.f, 0.75f);
      rvxo[MP_BLENDING] = MV_ADD;
      rvxo.set_dimensions(1, 1);
      rvxo.position = glm::vec3(0.f, 0.f, 0.1f);
   }

   return inst;
}

void mws_slider::set_rect(const mws_rect& i_rect)
{
   auto s_bar = std::static_pointer_cast<gfx_quad_2d>(slider_bar);
   auto s_ball = std::static_pointer_cast<gfx_quad_2d>(slider_ball);
   s_bar->set_translation(i_rect.x, i_rect.y + i_rect.h / 3);
   s_bar->set_scale(i_rect.w, i_rect.h / 3);
   s_ball->set_translation(i_rect.x - i_rect.h / 2, i_rect.y);
   s_ball->set_scale(i_rect.h, i_rect.h);

   mws_r = i_rect;
}

void mws_slider::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);
      auto type = ts->get_type();
      bool dragging_detected = dragging_dt.detect_helper(ts->crt_state.te);
      bool process = false;

      if (dragging_detected && active)
      {
         auto s_ball = std::static_pointer_cast<gfx_quad_2d>(slider_ball);
         auto tr = s_ball->get_translation();
         float tx = tr.x + dragging_dt.drag_diff.x;
         float x_off = mws_r.x - mws_r.h / 2;
         float t_val = 0.f;

         //mws_print("TS_PRESS_AND_DRAG [%f, %f]\n", dx, tx);
         if (tx < x_off)
         {
            tx = x_off;
            t_val = 0.f;
         }
         else if (tx >(x_off + mws_r.w))
         {
            tx = x_off + mws_r.w;
            t_val = 1.f;
         }
         else
         {
            t_val = (tx - x_off) / mws_r.w;
         }

         if (t_val != value)
         {
            value = t_val;
            s_ball->set_translation(tx, tr.y);

            if (on_drag_handler)
            {
               on_drag_handler(std::static_pointer_cast<mws_slider>(get_instance()));
            }
         }
      }

      //mws_print("evt type [%d]\n", type);
      switch (type)
      {
      case touch_sym_evt::TS_PRESSED:
         if (is_hit(ts->crt_state.te->points[0].x, ts->crt_state.te->points[0].y))
         {
            active = true;
            process = true;
         }
         break;

      case touch_sym_evt::TS_RELEASED:
         active = false;
         process = true;
         break;
      }

      if (!ts->is_processed() && process)
      {
         ts->process();
      }
   }
}

bool mws_slider::is_hit(float x, float y)
{
   auto& tf_bar = get_bar_vxo()->get_global_tf_mx();
   auto& pos_bar = gfx_util::get_pos_from_tf_mx(tf_bar);
   auto& scale_bar = gfx_util::get_scale_from_tf_mx(tf_bar);
   auto& pos_ball = gfx_util::get_pos_from_tf_mx(get_ball_vxo()->get_global_tf_mx());
   auto& scale_ball = gfx_util::get_scale_from_tf_mx(get_ball_vxo()->get_global_tf_mx());

   bool hit = is_inside_box(x, y, pos_bar.x - scale_bar.x / 2, pos_bar.y - scale_bar.y / 2, scale_bar.x, scale_bar.y);
   hit = hit || is_inside_box(x, y, pos_ball.x - scale_ball.x / 2, pos_ball.y - scale_ball.y / 2, scale_ball.x, scale_ball.y);

   return hit;
}

mws_sp<gfx_vxo> mws_slider::get_bar_vxo() const
{
   return slider_bar;
}

mws_sp<gfx_vxo> mws_slider::get_ball_vxo() const
{
   return slider_ball;
}

void mws_slider::set_on_drag_handler(std::function<void(mws_sp<mws_slider> i_img_btn)> i_on_drag_handler)
{
   on_drag_handler = i_on_drag_handler;
}

mws_slider::mws_slider()
{
   value = 0.f;
   active = false;
}


mws_list_model::mws_list_model()
{
   selected_elem = 0;
}

int mws_list_model::get_selected_elem()
{
   return selected_elem;
}

void mws_list_model::set_selected_elem(int iselectedElem)
{
   selected_elem = iselectedElem;
}


mws_list::mws_list(shared_ptr<mws_page> iparent)
{
   mws_r.set(0, 0, iparent->get_pos().w, iparent->get_pos().h);
   item_height = 150;
   vertical_space = 55;
   item_w = mws_r.w * 80 / 100;
   item_x = (mws_r.w - item_w) / 2;
}

shared_ptr<mws_list> mws_list::new_instance(shared_ptr<mws_page> i_parent)
{
   shared_ptr<mws_list> u(new mws_list(i_parent));
   i_parent->attach(u);
   u->add_to_page();
   return u;
}

shared_ptr<mws_list> mws_list::new_shared_instance(shared_ptr<mws_page> i_parent, mws_list* newListClassInstance)
{
   shared_ptr<mws_list> u(newListClassInstance);
   i_parent->attach(u);
   u->add_to_page();
   return u;
}

void mws_list::init() {}

void mws_list::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(MWS_EVT_MODEL_UPDATE))
   {
      float listheight = 0;

      for (int k = 0; k < model->get_length(); k++)
      {
         listheight += item_height + vertical_space;
      }

      if (listheight > 0)
      {
         listheight -= vertical_space;
      }

      mws_r.h = listheight;
   }
   else if (idp->is_type(touch_sym_evt::TOUCHSYM_EVT_TYPE))
   {
      shared_ptr<touch_sym_evt> ts = touch_sym_evt::as_touch_sym_evt(idp);

      switch (ts->get_type())
      {
      case touch_sym_evt::TS_FIRST_TAP:
      {
         float x = ts->pressed.te->points[0].x;
         float y = ts->pressed.te->points[0].y;

         if (ts->tap_count == 1)
         {
            int idx = element_at(x, y);

            if (idx >= 0)
            {
               model->set_selected_elem(idx);
               model->on_elem_selected(idx);
            }

            ts->process();
         }

         break;
      }
      }
   }
}

bool mws_list::is_hit(float x, float y)
{
   return false;
}

void mws_list::update_state()
{
}

void mws_list::update_view(shared_ptr<mws_camera> g)
{
   if (!model)
   {
      return;
   }

   mws_rect pr = get_mws_parent()->get_pos();
   int size = model->get_length();
   float vertOffset = pr.y;
   int selectedElem = model->get_selected_elem();

   for (int k = 0; k < size; k++)
   {
      if (k == selectedElem)
      {
         g->setColor(0xff7f00);
      }
      else
      {
         g->setColor(0x5000af);
      }

      g->fillRect(pr.x + item_x, vertOffset, item_w, item_height);
      g->drawText(model->elem_at(k), pr.x + item_x + item_w / 2, vertOffset + item_height / 2 - 10);
      vertOffset += item_height + vertical_space;
   }
}

void mws_list::set_model(shared_ptr<mws_list_model> imodel)
{
   model = imodel;
   model->set_view(get_instance());
}

shared_ptr<mws_list_model> mws_list::get_model()
{
   return model;
}

int mws_list::element_at(float x, float y)
{
   if (!is_hit(x, y))
   {
      return -1;
   }

   float vertOffset = get_mws_parent()->get_pos().y;

   for (int k = 0; k < model->get_length(); k++)
   {
      if (is_inside_box(x, y, item_x, vertOffset, item_w, item_height))
      {
         return k;
      }

      vertOffset += item_height + vertical_space;
   }

   return -1;
}


mws_tree_model::mws_tree_model()
{
   length = 0;
}

void mws_tree_model::set_length(int ilength)
{
   length = ilength;
}

int mws_tree_model::get_length()
{
   return length;
}

void mws_tree_model::set_root_node(shared_ptr<mws_tree_model_node> iroot)
{
   root = iroot;
}

shared_ptr<mws_tree_model_node> mws_tree_model::get_root_node()
{
   return root;
}

mws_tree::mws_tree(shared_ptr<mws_page> iparent)
{
}

shared_ptr<mws_tree> mws_tree::new_instance(shared_ptr<mws_page> iparent)
{
   shared_ptr<mws_tree> u(new mws_tree(iparent));
   u->add_to_page();
   return u;
}

shared_ptr<mws_tree> mws_tree::new_shared_instance(mws_tree* newTreeClassInstance)
{
   shared_ptr<mws_tree> u(newTreeClassInstance);
   u->add_to_page();
   return u;
}

void mws_tree::init()
{
}

void mws_tree::receive(shared_ptr<iadp> idp)
{
   if (idp->is_type(MWS_EVT_MODEL_UPDATE))
   {
      float h = 25.f + model->get_length() * 20.f;
      float w = 0;

      if (model->get_root_node())
      {
         //shared_ptr<mws_font> f = gfx_openvg::get_instance()->getFont();
         //get_max_width(f, model->get_root_node(), 0, w);
      }

      mws_r.h = h;
      mws_r.w = w / 2;
   }
}

void mws_tree::update_state()
{
}

void mws_tree::update_view(shared_ptr<mws_camera> g)
{
   shared_ptr<mws_tree_model_node> node = model->get_root_node();

   if (node->nodes.size() > 0)
   {
      int elemIdx = 0;

      draw_tree_elem(g, node, 0, elemIdx);
   }
}

void mws_tree::set_model(shared_ptr<mws_tree_model> imodel)
{
   model = imodel;
   model->set_view(get_instance());
}

shared_ptr<mws_tree_model> mws_tree::get_model()
{
   return model;
}

void mws_tree::get_max_width(shared_ptr<mws_font> f, const shared_ptr<mws_tree_model_node> node, int level, float& maxWidth)
{
   int size = node->nodes.size();

   for (int k = 0; k < size; k++)
   {
      shared_ptr<mws_tree_model_node> kv = node->nodes[k];

      float textWidth = 0;//get_text_width(f, kv->data);
      float twidth = 25 + level * 20 + textWidth;

      if (twidth > maxWidth)
      {
         maxWidth = twidth;
      }

      if (kv->nodes.size() > 0)
      {
         get_max_width(f, kv, level + 1, maxWidth);
      }
   }
}

void mws_tree::draw_tree_elem(shared_ptr<mws_camera> g, const shared_ptr<mws_tree_model_node> node, int level, int& elemIdx)
{
   int size = node->nodes.size();
   mws_rect r = get_mws_parent()->get_pos();

   for (int k = 0; k < size; k++)
   {
      shared_ptr<mws_tree_model_node> kv = node->nodes[k];
      glm::vec2 dim = g->get_font()->get_text_dim(kv->data);

      g->setColor(0xff00ff);
      g->drawRect(r.x + 20 + level * 20, r.y + 20 + elemIdx * dim.y, dim.x, dim.y);
      g->drawText(kv->data, r.x + 20 + level * 20, r.y + 20 + elemIdx * dim.y);
      elemIdx++;

      if (kv->nodes.size() > 0)
      {
         draw_tree_elem(g, kv, level + 1, elemIdx);
      }
   }
}

#endif
