#include "stdafx.h"

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "mws-vkb.hpp"
#include "mws-vkb-diagram.hpp"
#include "mws-vkb-visual.hpp"
#include "unit.hpp"
#include <kxmd/kxmd.hpp>


void mws_vkb::load(std::string i_filename)
{
   mws_sp<voronoi_main> vk;
   std::vector<int> key_vect;
   mws_sp<mws_font> key_font;
   mws_sp<mws_font> selected_key_font;
   mws_sp<std::vector<uint8> > res = get_unit()->storage.load_unit_byte_vect(i_filename);
   mws_sp<std::string> src(new std::string((const char*)begin_ptr(res), res->size()));
   kxmd_parser parser;

   mws_sp<kxmd_elem> kxmdi = parser.parse(src);
   auto kernel_point_chars = kxmd::get_elem("kernel-point-chars", kxmdi);
   auto kernel_points = kxmd::get_elem("kernel-points", kxmdi);
   std::vector<glm::vec2> kernel_points_pos;

   key_vect.resize(kernel_point_chars->elem_count());

   for (size_t k = 0; k < kernel_point_chars->elem_count(); k++)
   {
      int key_id = std::stoi(kernel_point_chars->vect[k]->val);
      key_vect[k] = key_id;
   }

   for (size_t k = 0; k < kernel_points->elem_count(); k++)
   {
      auto pos_pair = kernel_points->vect[k];
      float pos_0 = std::stof(pos_pair->vect[0]->val);
      float pos_1 = std::stof(pos_pair->vect[1]->val);
      kernel_points_pos.push_back(glm::vec2(pos_0, pos_1));
   }

   vk->set_kernel_points(kernel_points_pos);
   mws_println("finished loading keyboard from [ {} ]", i_filename);
}

#endif
