#include "stdafx.hxx"

#include "mws-com.hxx"
#include "fonts/mws-text-vxo.hxx"
#include "fonts/mws-font.hxx"
#include "fonts/mws-font-db.hxx"
#include "mws-camera.hxx"
#include "mws-mod.hxx"
#include "util/util.hxx"
#include "input/transitions.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx.hxx"
#include "gfx-util.hxx"
#include "gfx-color.hxx"
mws_push_disable_all_warnings
#include <glm/vec2.hpp>
#include <algorithm>
mws_pop_disable_all_warnings


static const uint32_t u32_max = std::numeric_limits<uint32_t>::max();


// mws_table_border
mws_sp<mws_table_border> mws_table_border::nwi()
{
   mws_sp<mws_table_border> tb = mws_sp<mws_table_border>(new mws_table_border());
   tb->setup();
   return tb;
}

void mws_table_border::set_color(const gfx_color& i_color)
{
   (*this)["u_v4_color"] = i_color.to_vec4();
}

mws_table_border::mws_table_border() : gfx_2d_sprite(nullptr) {}

void mws_table_border::setup()
{
   auto& rvxo = *this;
   rvxo.camera_id_list = { "mws_cam" };
   rvxo[MP_SHADER_NAME] = gfx::c_o_sh_id;
   rvxo[MP_DEPTH_TEST] = true;
   rvxo[MP_DEPTH_WRITE] = true;
   rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
   //rvxo["u_v4_color"] = gfx_color::from_argb(0xff050505).to_vec4();
   rvxo["u_v4_color"] = gfx_color::colors::magenta.to_vec4();
   rvxo.set_dimensions(1, 1);
   rvxo.set_anchor(gfx_quad_2d::e_top_left);
}


// mws_table_layout
mws_sp<mws_table_layout> mws_table_layout::nwi()
{
   mws_sp<mws_table_layout> table = mws_sp<mws_table_layout>(new mws_table_layout());
   table->setup();
   return table;
}

void mws_table_layout::on_resize()
{
   set_border_size(border_size);
}

void mws_table_layout::set_position(const glm::vec2& i_position)
{
   mws_page_item::set_position(i_position);
}

void mws_table_layout::set_size(const glm::vec2& i_size)
{
   mws_page_item::set_size(i_size);
}

void mws_table_layout::add_row(mws_sp<mws_page_item> i_item)
{
   attach(i_item);
   item_rows.push_back(i_item);

   if (item_rows.size() > 1)
   {
      mws_sp<mws_table_border> row_div = mws_table_border::nwi();

      row_div->set_color(color);
      attach(row_div);
      row_divs.push_back(row_div);
   }
}

void mws_table_layout::add_col(uint32_t i_row_idx, mws_sp<mws_page_item> i_item)
{
   //attach(i_item);
}

void mws_table_layout::set_cell_at(uint32_t i_row_idx, uint32_t i_col_idx, mws_sp<mws_page_item> i_item)
{
   //attach(i_item);
}

// returns one of the 4 table enclosing borders
mws_sp<mws_table_border> mws_table_layout::get_border(border_types i_border_type) const
{
   return borders[i_border_type];
}

// returns one of the inner row borders/dividers
mws_sp<mws_table_border> mws_table_layout::get_row_divider(uint32_t i_row_idx) const
{
   return row_divs[i_row_idx];
}

// returns one of the inner col borders/dividers in the specified row
mws_sp<mws_table_border> mws_table_layout::get_col_divider(uint32_t i_row_idx, uint32_t i_col_idx) const
{
   return nullptr;
}

void mws_table_layout::set_color(const gfx_color& i_color)
{
   color = i_color;

   for (mws_sp<mws_table_border>& tb : borders)
   {
      tb->set_color(i_color);
   }

   for (mws_sp<mws_table_border>& tb : row_divs)
   {
      tb->set_color(i_color);
   }
}

float mws_table_layout::get_border_size() const { return border_size; }

void mws_table_layout::set_border_size(float i_border_size)
{
   border_size = i_border_size;
   borders[e_left_border]->set_translation(mws_r.x, mws_r.y);
   borders[e_left_border]->set_scale(border_size, mws_r.h);
   borders[e_right_border]->set_translation(mws_r.x + mws_r.w - border_size, mws_r.y);
   borders[e_right_border]->set_scale(border_size, mws_r.h);
   borders[e_top_border]->set_translation(mws_r.x + border_size, mws_r.y);
   borders[e_top_border]->set_scale(mws_r.x + mws_r.w - 2.f * border_size, border_size);
   borders[e_btm_border]->set_translation(border_size, mws_r.y + mws_r.h - border_size);
   borders[e_btm_border]->set_scale(mws_r.x + mws_r.w - 2.f * border_size, border_size);

   if (!item_rows.empty())
   {
      mws_sp<mws_page_item> item_0 = item_rows[0];
      float tx_0 = borders[e_left_border]->get_translation().x + borders[e_left_border]->get_scale().x;
      float ty_0 = borders[e_top_border]->get_translation().y + borders[e_top_border]->get_scale().y;
      const mws_size& size_0 = item_0->get_best_size();
      //float sx_0 = std::get<mws_px>(size_0.get_width()).val();
      float sy_0 = std::get<mws_px>(size_0.get_height()).val();
      float row_width = borders[e_right_border]->get_translation().x - tx_0;

      item_0->set_rect(mws_rect(tx_0, ty_0, row_width, sy_0));

      if (!row_divs.empty())
      {
         mws_sp<mws_table_border> row_0 = row_divs[0];

         row_0->set_translation(tx_0, item_0->get_pos().y + item_0->get_pos().h);
         row_0->set_scale(borders[e_right_border]->get_translation().x - tx_0, i_border_size);

         for (uint32_t k = 1; k < item_rows.size(); k++)
         {
            mws_sp<mws_page_item> item_km1 = item_rows[k - 1];
            mws_sp<mws_table_border> row_km1 = row_divs[k - 1];
            float row_km1_y = row_km1->get_translation().y;
            float row_km1_h = row_km1->get_scale().y;
            mws_sp<mws_page_item> item_k = item_rows[k];
            const mws_size& size_k = item_k->get_best_size();
            float sy_k = std::get<mws_px>(size_k.get_height()).val();

            item_k->set_rect(mws_rect(tx_0, row_km1_y + row_km1_h, row_width, sy_k));

            if (k < row_divs.size())
            {
               mws_sp<mws_table_border> row_k = row_divs[k];
               float item_k_y = item_k->get_pos().y;
               float item_k_h = item_k->get_pos().h;

               row_k->set_translation(tx_0, item_k_y + item_k_h);
               row_k->set_scale(borders[e_right_border]->get_translation().x - tx_0, i_border_size);
            }
         }
      }
   }
}


void mws_table_layout::setup()
{
   mws_page_item::setup();

   for (uint32_t k = 0; k < 4; k++)
   {
      mws_sp<mws_table_border> border = mws_table_border::nwi();

      attach(border);
      borders.push_back(border);
   }
}


// mws_stack_page_nav
mws_sp<mws_stack_page_nav> mws_stack_page_nav::nwi(mws_sp<mws_page_tab> i_tab)
{
   auto i = mws_sp<mws_stack_page_nav>(new mws_stack_page_nav());
   i->tab = i_tab;
   i->setup();
   return i;
}

const std::string& mws_stack_page_nav::get_main_page_id() const
{
   return main_page_id;
}

void mws_stack_page_nav::set_main_page_id(const std::string& i_main_page_id)
{
   main_page_id = i_main_page_id;
}

uint32_t mws_stack_page_nav::page_stack_size() const
{
   return page_stack.size();
}

const std::string& mws_stack_page_nav::top_page() const
{
   return page_stack.top();
}

void mws_stack_page_nav::pop()
{
   std::string active_page;

   if (!page_stack.empty())
   {
      page_stack.pop();
   }

   if (page_stack.empty())
   {
      active_page = get_main_page_id();
   }
   else
   {
      active_page = page_stack.top();
   }

   set_current(active_page);
}

void mws_stack_page_nav::push(std::string i_page_id)
{
   page_stack.push(i_page_id);
   set_current(i_page_id);
}

void mws_stack_page_nav::set_current(const std::string& i_page_id)
{
   mws_sp<mws_page_tab> tab_inst = tab.lock();
   auto page = std::static_pointer_cast<mws_page>(tab_inst->find_by_id(i_page_id));

   if (page != nullptr)
   {
      auto& pt = tab_inst->page_tab;
      auto it = std::find(pt.begin(), pt.end(), page);
      mws_assert(it != pt.end());

      if (it != pt.end() - 1)
      {
         std::iter_swap(it, pt.end() - 1);
      }

      for (auto& p : pt)
      {
         if (p->visible && p != page)
         {
            p->visible = false;
         }
      }

      if (!page->visible)
      {
         page->visible = true;
      }
   }
}

void mws_stack_page_nav::reset_pages()
{
   page_stack = std::stack<std::string>();
   set_current(get_main_page_id());
}

void mws_stack_page_nav::setup() {}


// mws_panel
mws_sp<mws_panel> mws_panel::nwi()
{
   auto inst = mws_sp<mws_panel>(new mws_panel());
   inst->setup();
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

void mws_panel::setup()
{
   mws_page_item::setup();
   vxo = gfx_quad_2d::nwi();
   attach(vxo);

   {
      auto& rvxo = *vxo;
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = gfx::mws_sh_id;
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(0, 1.f, 0, 1);
      rvxo["u_v1_is_enabled"] = 1.f;
      rvxo["u_v1_has_tex"] = 0.f;
      rvxo.set_dimensions(1, 1);
   }
}


// mws_label
mws_sp<mws_label> mws_label::nwi()
{
   auto inst = mws_sp<mws_label>(new mws_label());
   inst->setup();
   return inst;
}

void mws_label::set_rect(const mws_rect& i_rect)
{
   position = glm::vec3(i_rect.x, i_rect.y, position().z);
   mws_r = i_rect;
}

void mws_label::update_state()
{
   if (!text.empty())
   {
      auto& tf = get_global_tf_mx();
      auto& pos_v4 = gfx_util::get_pos_from_tf_mx(tf);
      glm::vec2 pos(pos_v4.x - mws_r.w / 2, pos_v4.y);
      auto root_ref = get_mws_root();
      auto text_ref = root_ref->get_text_vxo();

      text_ref->add_text(text, pos, get_font());
   }
}

void mws_label::set_text(std::string i_text)
{
   text = i_text;
}

mws_sp<mws_font> mws_label::get_font() const
{
   mws_sp<mws_font> f = (font) ? font : mws_cam.lock()->get_font();
   return f;
}

void mws_label::set_font(mws_sp<mws_font> i_font)
{
   font = i_font;
}

void mws_label::setup()
{
   mws_page_item::setup();
}


// mws_img_btn
mws_sp<mws_img_btn> mws_img_btn::nwi()
{
   auto inst = mws_sp<mws_img_btn>(new mws_img_btn());
   inst->setup();
   return inst;
}

void mws_img_btn::set_enabled(bool i_is_enabled)
{
   if (i_is_enabled != enabled)
   {
      float u_v1_is_enabled = i_is_enabled ? 1.f : 0.f;
      (*get_vxo())["u_v1_is_enabled"] = u_v1_is_enabled;
   }

   mws_page_item::set_enabled(i_is_enabled);
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

   //tex = gi()->tex.nwi(i_img_name, &prm);
   (*vxo)["u_s2d_tex"] = i_img_name;
}

void mws_img_btn::receive(mws_sp<mws_dp> i_dp)
{
   mws_page_item::receive(i_dp);

   if (!i_dp->is_processed())
   {
      if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
      {
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);

         if (!is_hit(ts->points[0].x, ts->points[0].y))
         {
            return;
         }

         //mws_print("evt type [%d]\n", type);
         if (ts->type == ts->touch_began)
         {
            if (on_click)
            {
               on_click();
            }
            else
            {
               on_click_handler();
            }

            process(ts);
         }
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

mws_sp<gfx_quad_2d> mws_img_btn::get_vxo()
{
   return vxo;
}

void mws_img_btn::setup()
{
   mws_page_item::setup();
   vxo = gfx_quad_2d::nwi();
   attach(vxo);

   {
      auto& rvxo = *vxo;
      rvxo.camera_id_list = { "mws_cam" };
      rvxo[MP_SHADER_NAME] = gfx::mws_sh_id;
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_s2d_tex"] = "";
      rvxo["u_v1_is_enabled"] = 1.f;
      rvxo["u_v1_has_tex"] = 1.f;
      rvxo.set_dimensions(1, 1);
      rvxo.set_anchor(gfx_quad_2d::e_center);
      rvxo[MP_BLENDING] = MV_ALPHA;
   }
}


// mws_button
mws_sp<mws_button> mws_button::nwi()
{
   auto inst = mws_sp<mws_button>(new mws_button());
   inst->setup();
   return inst;
}

void mws_button::set_enabled(bool i_is_enabled)
{
   if (i_is_enabled != enabled)
   {
      float u_v1_is_enabled = i_is_enabled ? 1.f : 0.f;
      (*get_vxo())["u_v1_is_enabled"] = u_v1_is_enabled;
   }

   mws_page_item::set_enabled(i_is_enabled);
}

void mws_button::set_rect(const mws_rect& i_rect)
{
   //vxo->set_translation(i_rect.x, i_rect.y);
   position = glm::vec3(i_rect.x, i_rect.y, position().z);
   vxo->set_scale(i_rect.w, i_rect.h);
   mws_r = i_rect;
}

void mws_button::receive(mws_sp<mws_dp> i_dp)
{
   mws_page_item::receive(i_dp);

   if (!i_dp->is_processed())
   {
      if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
      {
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);

         if (!is_hit(ts->points[0].x, ts->points[0].y))
         {
            return;
         }

         //mws_print("evt type [%d]\n", type);
         if (ts->type == ts->touch_began)
         {
            if (on_click)
            {
               on_click();
            }
            else
            {
               on_click_handler();
            }

            process(ts);
         }
      }
   }
}

bool mws_button::is_hit(float x, float y)
{
   auto& tf = get_vxo()->get_global_tf_mx();
   auto& pos = gfx_util::get_pos_from_tf_mx(tf);
   //auto& scale = gfx_util::get_scale_from_tf_mx(tf);
   //bool hit = is_inside_box(x, y, pos.x - scale.x / 2, pos.y - scale.y / 2, scale.x, scale.y);
   bool hit = is_inside_box(x, y, pos.x - mws_r.w / 2, pos.y - mws_r.h / 2, mws_r.w, mws_r.h);

   return hit;
}

void mws_button::update_state()
{
   if (text_visible && !text.empty())
   {
      mws_sp<mws_font> fnt = get_font();
      const glm::mat4& tf = get_vxo()->get_global_tf_mx();
      const glm::vec4& pos_v4 = gfx_util::get_pos_from_tf_mx(tf);
      glm::vec2 text_dim = fnt->get_text_dim(text);
      glm::vec2 pos(pos_v4.x - mws_r.w / 2 + (mws_r.w - text_dim.x) / 2.f, pos_v4.y - (mws_r.h - text_dim.y) / 2.f);
      const auto& root_ref = get_mws_root();
      mws_sp<mws_text_vxo> text_ref = root_ref->get_text_vxo();

      text_ref->add_text(text, pos, fnt);
   }
}

const std::string& mws_button::get_text() const
{
   return text;
}

void mws_button::set_text(std::string i_text)
{
   text = i_text;
}

const gfx_color& mws_button::get_bg_color() const
{
   return color;
}

void mws_button::set_bg_color(const gfx_color& i_color)
{
   color = i_color;
   (*get_vxo())["u_v4_color"] = color.to_vec4();
}

void mws_button::set_bg_visible(bool i_visible)
{
   get_vxo()->visible = i_visible;
}

mws_sp<mws_font> mws_button::get_font() const
{
   mws_sp<mws_font> f = (font) ? font : mws_cam.lock()->get_font();
   return f;
}

void mws_button::set_font(mws_sp<mws_font> i_font)
{
   font = i_font;
}

mws_sp<gfx_quad_2d> mws_button::get_vxo()
{
   return vxo;
}

void mws_button::setup()
{
   mws_page_item::setup();
   vxo = gfx_quad_2d::nwi();
   attach(vxo);

   {
      auto& rvxo = *vxo;
      color = gfx_color::from_argb(0xff333333);
      rvxo.camera_id_list = { "mws_cam" };
      rvxo[MP_SHADER_NAME] = gfx::mws_sh_id;
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = color.to_vec4();
      rvxo["u_v1_is_enabled"] = 1.f;
      rvxo["u_v1_has_tex"] = 0.f;
      rvxo.set_dimensions(1, 1);
   }
}


// mws_slider
mws_sp<mws_slider> mws_slider::nwi()
{
   auto inst = mws_sp<mws_slider>(new mws_slider());
   inst->setup();
   return inst;
}

void mws_slider::set_value(float i_value)
{
   mws_assert(i_value >= 0.f && i_value <= 1.f);
   float x_off = mws_r.x;
   auto s_ball = std::static_pointer_cast<gfx_quad_2d>(slider_ball);
   auto tr = s_ball->get_translation();
   float tx = x_off + i_value * mws_r.w;

   s_ball->set_translation(tx, tr.y);

   if (value != i_value)
   {
      value = i_value;

      if (on_drag)
      {
         on_drag();
      }
      else
      {
         on_drag_handler();
      }
   }
}

void mws_slider::set_rect(const mws_rect& i_rect)
{
   auto s_bar = std::static_pointer_cast<gfx_quad_2d>(slider_bar);
   auto s_ball = std::static_pointer_cast<gfx_quad_2d>(slider_ball);
   s_bar->set_translation(i_rect.x, i_rect.y + i_rect.h / 3);
   s_bar->set_scale(i_rect.w, i_rect.h / 3);
   s_ball->set_translation(i_rect.x, i_rect.y + i_rect.h / 2);
   s_ball->set_scale(i_rect.h, i_rect.h);

   mws_r = i_rect;
   set_value(value);
}

void mws_slider::receive(mws_sp<mws_dp> i_dp)
{
   mws_page_item::receive(i_dp);

   if (!i_dp->is_processed())
   {
      if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
      {
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);
         bool dragging_detected = dragging_dt.detect_helper(ts);
         bool process_evt = false;

         if (dragging_detected && active)
         {
            auto s_ball = std::static_pointer_cast<gfx_quad_2d>(slider_ball);
            auto tr = s_ball->get_translation();
            float tx = tr.x + dragging_dt.drag_diff.x;
            float x_off = mws_r.x;
            float t_val = 0.f;

            //mws_print("TS_PRESS_AND_DRAG [%f, %f]\n", dx, tx);
            if (tx < x_off)
            {
               tx = x_off;
               t_val = 0.f;
            }
            else if (tx > (x_off + mws_r.w))
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
               set_value(t_val);
            }
         }

         //mws_print("evt type [%d]\n", type);
         switch (ts->type)
         {
         case mws_ptr_evt::touch_began:
         {
            bool ball_hit;
            bool bar_hit;

            if (is_hit(ts->points[0].x, ts->points[0].y, ball_hit, bar_hit))
            {
               // if we hit the bar, but not the ball, move the ball to the hit position
               if (bar_hit && !ball_hit)
               {
                  float x_off = mws_r.x;
                  float t_val = (ts->points[0].x - x_off) / mws_r.w;

                  if (t_val != value)
                  {
                     set_value(t_val);
                  }
               }

               active = true;
               process_evt = true;
            }
            break;
         }

         case mws_ptr_evt::touch_ended:
            active = false;
            process_evt = false;
            break;
         }

         if (!ts->is_processed() && process_evt)
         {
            process(ts);
         }
      }
   }
}

bool mws_slider::is_hit(float x, float y, bool& i_ball_hit, bool& i_bar_hit)
{
   auto& tf_bar = get_bar_vxo()->get_global_tf_mx();
   auto& pos_bar = gfx_util::get_pos_from_tf_mx(tf_bar);
   auto& scale_bar = gfx_util::get_scale_from_tf_mx(tf_bar);
   auto& pos_ball = gfx_util::get_pos_from_tf_mx(get_ball_vxo()->get_global_tf_mx());
   auto& scale_ball = gfx_util::get_scale_from_tf_mx(get_ball_vxo()->get_global_tf_mx());

   i_bar_hit = is_inside_box(x, y, pos_bar.x - scale_bar.x / 2, pos_bar.y - scale_ball.y / 2, scale_bar.x, scale_ball.y);
   i_ball_hit = is_inside_box(x, y, pos_ball.x - scale_ball.x / 2, pos_ball.y - scale_ball.y / 2, scale_ball.x, scale_ball.y);

   bool hit = i_bar_hit || i_ball_hit;

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

mws_slider::mws_slider()
{
   value = 0.f;
   active = false;
}

void mws_slider::setup()
{
   mws_page_item::setup();
   slider_bar = gfx_quad_2d::nwi();
   slider_ball = gfx_quad_2d::nwi();
   attach(slider_bar);
   attach(slider_ball);
   set_z(0.f);

   // slider bar
   {
      auto& rvxo = *std::static_pointer_cast<gfx_quad_2d>(slider_bar);
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = gfx::mws_sh_id;
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(0.75f);
      rvxo["u_v1_is_enabled"] = 1.f;
      rvxo["u_v1_has_tex"] = 0.f;
      rvxo.set_dimensions(1, 1);
   }

   // slider ball
   {
      auto& rvxo = *std::static_pointer_cast<gfx_quad_2d>(slider_ball);
      rvxo.camera_id_list.clear();
      rvxo.camera_id_list.push_back("mws_cam");
      rvxo[MP_SHADER_NAME] = gfx::mws_sh_id;
      rvxo[MP_DEPTH_TEST] = true;
      rvxo[MP_DEPTH_WRITE] = true;
      rvxo[MP_DEPTH_FUNCTION] = MV_LESS_OR_EQUAL;
      rvxo["u_v4_color"] = glm::vec4(1.f, 1.f, 1.f, 0.75f);
      rvxo["u_v1_is_enabled"] = 1.f;
      rvxo["u_v1_has_tex"] = 0.f;
      rvxo[MP_BLENDING] = MV_ADD;
      rvxo.set_dimensions(1, 1);
      rvxo.position = glm::vec3(0.f, 0.f, 0.1f);
      rvxo.set_anchor(gfx_quad_2d::e_center);
   }
}


// mws_list_model
mws_list_model::mws_list_model()
{
   selected_elem = 0;
}

uint32_t mws_list_model::get_selected_elem()
{
   return selected_elem;
}

void mws_list_model::set_selected_elem(uint32_t iselectedElem)
{
   selected_elem = iselectedElem;
}


// mws_list
mws_list::mws_list()
{
}

void mws_list::setup()
{
   mws_page_item::setup();
   //mws_r.set(0, 0, get_mws_parent()->get_pos().w, get_mws_parent()->get_pos().h);
   //item_height = 150;
   //vertical_space = 55;
   //item_w = mws_r.w * 80 / 100;
   //item_x = (mws_r.w - item_w) / 2;
}

mws_sp<mws_list> mws_list::nwi()
{
   auto inst = mws_sp<mws_list>(new mws_list());
   inst->setup();
   return inst;
}

void mws_list::receive(mws_sp<mws_dp> i_dp)
{
   mws_page_item::receive(i_dp);

   if (!i_dp->is_processed())
   {
      if (i_dp->is_type(MWS_EVT_MODEL_UPDATE))
      {
         float listheight = 0;

         for (uint32_t k = 0, size = model->get_length(); k < size; k++)
         {
            listheight += item_height + vertical_space;
         }

         if (listheight > 0)
         {
            listheight -= vertical_space;
         }

         mws_r.h = listheight;
      }
      else if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
      {
         //mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);

         //switch (ts->get_type())
         //{
         //case touch_sym_evt::TS_FIRST_TAP:
         //{
         //   float x = ts->pressed.te->points[0].x;
         //   float y = ts->pressed.te->points[0].y;

         //   if (ts->tap_count == 1)
         //   {
         //      uint32_t idx = element_at(x, y);

         //      if (idx >= 0)
         //      {
         //         model->set_selected_elem(idx);
         //         model->on_elem_selected(idx);
         //      }

         //      ts->process();
         //   }

         //   break;
         //}
         //}
      }
   }
}

bool mws_list::is_hit(float, float)
{
   return false;
}

void mws_list::update_state()
{
   uint32_t size = model->get_length();

   if (size > 0)
   {
      auto& tf = get_global_tf_mx();
      auto& pos_v4 = gfx_util::get_pos_from_tf_mx(tf);
      glm::vec2 pos(pos_v4.x - mws_r.w / 2, pos_v4.y);
      auto root_ref = get_mws_root();
      auto text_ref = root_ref->get_text_vxo();
      mws_sp<mws_font> f = mws_cam.lock()->get_font();
      pos = glm::vec2(20.f);

      for (uint32_t k = 0; k < size; k++)
      {
         std::string text = model->elem_at(k);
         text_ref->add_text(text, pos, f);
         pos.y += f->get_height() * 1.25f;
      }
   }
}

void mws_list::set_model(mws_sp<mws_list_model> imodel)
{
   model = imodel;
   model->set_view(get_instance());
}

mws_sp<mws_list_model> mws_list::get_model()
{
   return model;
}

uint32_t mws_list::element_at(float x, float y)
{
   if (!is_hit(x, y))
   {
      return u32_max;
   }

   float vert_offset = get_mws_parent()->get_pos().y;

   for (uint32_t k = 0; k < model->get_length(); k++)
   {
      if (is_inside_box(x, y, item_x, vert_offset, item_w, item_height))
      {
         return k;
      }

      vert_offset += item_height + vertical_space;
   }

   return u32_max;
}


// mws_tree_model
mws_tree_model::mws_tree_model()
{
   length = 0;
}

void mws_tree_model::set_length(uint32_t i_length)
{
   length = i_length;
}

uint32_t mws_tree_model::get_length()
{
   return length;
}

void mws_tree_model::set_root_node(mws_sp<mws_tree_model_node> i_root)
{
   root = i_root;
}

mws_sp<mws_tree_model_node> mws_tree_model::get_root_node()
{
   return root;
}


// mws_tree
void mws_tree::setup()
{
   mws_page_item::setup();
}

mws_sp<mws_tree> mws_tree::nwi()
{
   mws_sp<mws_tree> u(new mws_tree());
   u->setup();
   return u;
}

void mws_tree::init()
{
}

void mws_tree::receive(mws_sp<mws_dp> i_dp)
{
   mws_page_item::receive(i_dp);

   if (!i_dp->is_processed())
   {
      if (i_dp->is_type(mws_ptr_evt::ptr_evt_type))
      {
         mws_sp<mws_ptr_evt> ts = mws_ptr_evt::as_pointer_evt(i_dp);
         bool dbl_tap_detected = dbl_tap_det.detect_helper(ts);
         glm::vec2 press_pos(0.f);
         bool selected = true;

         if (ts->type == mws_ptr_evt::touch_began)
         {
            press_pos = mws_ptr_evt::get_pos(ts->points[0]); selected = false;
         }

         if (dbl_tap_detected)
         {
            press_pos = dbl_tap_det.get_avg_press_pos(); selected = false;
         }

         for (uint32_t k = 0, size = bounding_box_list.size(); k < size; k++)
         {
            node_bounding_box& nbb = bounding_box_list[k];
            mws_rect& bbx = nbb.bounding_box;

            if (is_inside_box(press_pos.x, press_pos.y, bbx.x, bbx.y, bbx.w, bbx.h))
            {
               if (dbl_tap_detected)
               {
                  if (on_click)
                  {
                     on_click(nbb.node_ref, nbb.child_list_idx);
                  }
                  else
                  {
                     on_click_handler(nbb.node_ref);
                  }
               }

               selected = true;
               selected_idx = k;
               break;
            }
         }

         if (!selected)
         {
            selected_idx = mws_u32_max;
         }
      }
      else if (i_dp->is_type(MWS_EVT_MODEL_UPDATE))
      {
         bounding_box_list.clear();

         if (model->get_root_node())
         {
            mws_sp<mws_font> font = mws_font_db::inst()->get_global_font();
            calc_bounding_box_list(font, model->get_root_node(), 0);
         }
      }
   }
}

void mws_tree::update_view(mws_sp<mws_camera> i_g)
{
   mws_sp<mws_tree_model_node> node = model->get_root_node();

   if (node && node->nodes.size() > 0)
   {
      uint32_t i_elem_idx = 0;

      draw_tree_elem(i_g, node, 0, i_elem_idx);
   }
}

void mws_tree::set_model(mws_sp<mws_tree_model> i_model)
{
   model = i_model;
   model->set_view(get_instance());
}

mws_sp<mws_tree_model> mws_tree::get_model()
{
   return model;
}

void mws_tree::draw_tree_elem(mws_sp<mws_camera> i_g, const mws_sp<mws_tree_model_node> i_node, uint32_t i_level, uint32_t& i_elem_idx)
{
   mws_rect r = get_mws_parent()->get_pos();

   for (uint32_t k = 0, size = i_node->nodes.size(); k < size; k++)
   {
      mws_sp<mws_tree_model_node> kv = i_node->nodes[k];
      glm::vec2 dim = i_g->get_font()->get_text_dim(kv->data);

      if (i_elem_idx == selected_idx)
      {
         i_g->set_color(gfx_color(0xff, 0, 0));
         i_g->drawRect(r.x + margin + i_level * level_indentation, r.y + margin + i_elem_idx * dim.y, dim.x, dim.y);
      }

      i_g->drawText(kv->data, r.x + margin + i_level * level_indentation, r.y + margin + i_elem_idx * dim.y);
      i_elem_idx++;

      if (kv->nodes.size() > 0)
      {
         draw_tree_elem(i_g, kv, i_level + 1, i_elem_idx);
      }
   }
}

void mws_tree::calc_bounding_box_list(const mws_sp<mws_font>& i_fnt, const mws_sp<mws_tree_model_node> i_node, uint32_t i_level)
{
   for (uint32_t k = 0, size = i_node->nodes.size(); k < size; k++)
   {
      mws_sp<mws_tree_model_node> kv = i_node->nodes[k];
      glm::vec2 dim = i_fnt->get_text_dim(kv->data);
      float y_off = margin;

      if (bounding_box_list.size() > 0)
      {
         mws_rect& bbx = bounding_box_list.back().bounding_box;
         y_off = bbx.y + bbx.h;
      }

      bounding_box_list.emplace_back(node_bounding_box{ kv, {margin + i_level * level_indentation, y_off, dim.x, dim.y}, k });

      if (kv->nodes.size() > 0)
      {
         calc_bounding_box_list(i_fnt, kv, i_level + 1);
      }
   }
}
