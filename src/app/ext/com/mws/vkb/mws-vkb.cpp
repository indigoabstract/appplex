#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "mws-vkb.hpp"
#include "mws-vkb-diagram.hpp"
#include "mws-vkb-visual.hpp"
#include "../mws-camera.hpp"
#include "unit.hpp"
#include "gfx-quad-2d.hpp"
#include <kxmd/kxmd.hpp>


mws_sp<mws_vkb> mws_vkb::inst;
mws_sp<mws_vkb> mws_vkb::gi()
{
   if (!inst)
   {
      inst = mws_sp<mws_vkb>(new mws_vkb());
      inst->setup();
   }

   return inst;
}

void mws_vkb::receive(mws_sp<iadp> i_dp)
{
   if (i_dp->is_type(pointer_evt::TOUCHSYM_EVT_TYPE))
   {
      mws_sp<pointer_evt> ts = pointer_evt::as_pointer_evt(i_dp);

      switch (ts->type)
      {
      case pointer_evt::touch_began:
      {
         visible = false;
         tbx = nullptr;
         break;
      }

      case pointer_evt::touch_ended:
      {
         break;
      }
      }

      ts->process();
   }
}

void mws_vkb::on_resize()
{
   mws_r.x = 0;
   mws_r.y = 0;
   mws_r.w = (float)pfm::screen::get_width();
   mws_r.h = (float)pfm::screen::get_height();
}

void mws_vkb::set_target(mws_sp<mws_text_box> i_tbx)
{
   tbx = i_tbx;

   if (vkb_filename.empty())
   {
      load("vk-00.kxmd");
   }
}

void mws_vkb::load(std::string i_filename)
{
   if (!vk)
   {
      vk = vkb_voronoi_main::nwi(pfm::screen::get_width(), pfm::screen::get_height(), mws_cam.lock());
      attach(vk->vgeom);
   }

   vkb_filename = i_filename;
   std::vector<int> key_vect;
   mws_sp<mws_font> key_font;
   mws_sp<mws_font> selected_key_font;
   mws_sp<std::vector<uint8> > res = get_unit()->storage.load_unit_byte_vect(vkb_filename);
   mws_sp<std::string> src(new std::string((const char*)begin_ptr(res), res->size()));
   kxmd_parser parser;

   mws_sp<kxmd_elem> kxmdi = parser.parse(src);
   mws_sp<vkb_voronoi_data> diag_data = vk->diag_data;
   vkb_voronoi_data::vkb_voronoi_geometry_data& gd = diag_data->geom;

   {
      auto kernel_point_keys = kxmd::get_elem("kernel-point-keys", kxmdi);

      key_vect.resize(kernel_point_keys->elem_count());

      for (int k = 0; k < kernel_point_keys->elem_count(); k++)
      {
         int key_id = std::stoi(kernel_point_keys->vect[k]->val);
         key_vect[k] = key_id;
      }
   }

   // kernel-points
   {
      auto kernel_points = kxmd::get_elem("kernel-points", kxmdi);
      auto& kpv = gd.kernel_points;
      int size = kernel_points->elem_count();

      kpv.resize(size);
      kpv._first_idx = 0;
      kpv._last_idx = size - 1;

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = kernel_points->vect[k];
         auto& kp = kpv[k];

         kp.id = k;
         kp.position.x = std::stof(pos_pair->vect[0]->val);
         kp.position.y = std::stof(pos_pair->vect[1]->val);
      }
   }

   // nexus-points
   {
      auto nexus_points = kxmd::get_elem("nexus-points", kxmdi);
      auto& npv = gd.nexus_points;
      int size = nexus_points->elem_count();

      npv.resize(size);
      npv._first_idx = gd.kernel_points._last_idx + 1;
      npv._last_idx = npv._first_idx + size;

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = nexus_points->vect[k];
         auto& np = npv[k];

         np.id = npv._first_idx + k;
         np.position.x = std::stof(pos_pair->vect[0]->val);
         np.position.y = std::stof(pos_pair->vect[1]->val);
      }
   }

   // nexus-pairs
   {
      auto nexus_pairs = kxmd::get_elem("nexus-pairs", kxmdi);
      auto& npp = gd.nexus_pairs;
      int size = nexus_pairs->elem_count();
      npp.resize(size);

      for (int k = 0; k < size; k++)
      {
         auto pos_pair = nexus_pairs->vect[k];
         auto& np = npp[k];

         np.nexus0_id = std::stoi(pos_pair->vect[0]->val);
         np.nexus1_id = std::stoi(pos_pair->vect[1]->val);
      }
   }

   // cell-indices
   {
      auto cell_indices = kxmd::get_elem("cell-indices", kxmdi);
      auto& cpi = gd.cell_points_ids;
      auto& cpc = gd.cell_point_count;
      int cell_indices_size = cell_indices->elem_count();
      int cell_points_count = 0;

      cpc.resize(cell_indices_size);

      for (int k = 0; k < cell_indices_size; k++)
      {
         auto& triangle_indices = cell_indices->vect[k];
         int vx_count = triangle_indices->vect.size();

         cell_points_count += vx_count;
         cpc[k] = vx_count;
      }

      cpi.resize(cell_points_count);

      for (int k = 0, l = 0; k < cell_indices_size; k++)
      {
         auto& triangle_indices = cell_indices->vect[k];
         size_t triangle_indices_count = triangle_indices->vect.size();

         for (size_t i = 0; i < triangle_indices_count; i++, l++)
         {
            int idx = std::stoi(triangle_indices->vect[i]->val);
            cpi[l].point_id = idx;
         }
      }
   }

   vk->update_geometry();
   mws_println("finished loading keyboard from [ %s ]", vkb_filename.c_str());
}

void mws_vkb::setup()
{
   mws_virtual_keyboard::setup();
   position = glm::vec3(position().x, position().y, 1.f);
   on_resize();
}

#endif
