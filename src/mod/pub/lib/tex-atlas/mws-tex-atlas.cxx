#include "stdafx.hxx"

#include "mws-tex-atlas.hxx"
#include "mws/mws-camera.hxx"
#include "mws/mws-com.hxx"
#include "gfx.hxx"
#include "gfx-quad-2d.hxx"
#include "gfx-tex.hxx"


mws_sp<mws_tex_atlas> mws_tex_atlas::nwi(uint32 i_width, uint32 i_height, uint32 i_depth)
{
   mws_sp<mws_tex_atlas> inst = mws_sp<mws_tex_atlas>(new mws_tex_atlas());

   mws_assert((i_depth == 1) || (i_depth == 3) || (i_depth == 4));
   inst->used = 0;
   inst->width = i_width;
   inst->height = i_height;
   inst->depth = i_depth;
   inst->clear();

   return inst;
}

mws_sp<gfx_tex> mws_tex_atlas::get_tex() const { return tex; }

void mws_tex_atlas::upload()
{
   mws_assert(!data.empty());

   if (!tex)
   {
      std::string tex_id = mws_to_str_fmt("mws-tex-atlas-%d", inst_count);
      gfx_tex_params prm;

      prm.set_rt_params();
      tex = gfx::i()->tex.nwi(tex_id, width, height, &prm);
   }

   tex->update(0, (const char*)data.data());
}

glm::ivec4 mws_tex_atlas::get_region(uint32 i_width, uint32 i_height)
{
   int best_index = -1;
   glm::ivec4 region = { 0, 0, i_width, i_height };

   {
      uint32 best_height = INT_MAX;
      int best_width = INT_MAX;

      for (uint32 i = 0; i < nodes.size(); ++i)
      {
         int y = fit(i, i_width, i_height);

         if (y >= 0)
         {
            glm::ivec3* node = &nodes[i];

            if (((y + i_height) < best_height) || (((y + i_height) == best_height) && (node->z < best_width)))
            {
               best_height = y + i_height;
               best_index = i;
               best_width = node->z;
               region.x = node->x;
               region.y = y;
            }
         }
      }
   }
   if (best_index == -1)
   {
      region = { -1, -1, 0, 0 };

      return region;
   }
   {
      glm::ivec3 t_node = { region.x, region.y + i_height, i_width };
      nodes.insert(nodes.begin() + best_index, t_node);
   }

   for (uint32 i = best_index + 1; i < nodes.size(); ++i)
   {
      glm::ivec3* node = &nodes[i];
      glm::ivec3* prev = &nodes[i - 1];

      if (node->x < (prev->x + prev->z))
      {
         int shrink = prev->x + prev->z - node->x;

         node->x += shrink;
         node->z -= shrink;

         if (node->z <= 0)
         {
            nodes.erase(nodes.begin() + i);
            --i;
         }
         else
         {
            break;
         }
      }
      else
      {
         break;
      }
   }

   merge();
   used += i_width * i_height;

   return region;
}

void mws_tex_atlas::set_region(uint32 i_x, uint32 i_y, uint32 i_width, uint32 i_height, const uint8* i_data, uint32 i_stride)
{
   mws_assert(i_x > 0);
   mws_assert(i_y > 0);
   mws_assert(i_x < (width - 1));
   mws_assert((i_x + i_width) <= (width - 1));
   mws_assert(i_y < (height - 1));
   mws_assert((i_y + i_height) <= (height - 1));

   for (uint32 i = 0; i < i_height; ++i)
   {
      memcpy(data.data() + ((i_y + i) * width + i_x) * depth, i_data + (i * i_stride), i_width * depth);
   }
}

void mws_tex_atlas::clear()
{
   // we want a one pixel border around the whole atlas to avoid any artefact when sampling texture
   glm::ivec3 node = { 1, 1, width - 2 };

   nodes.clear();
   nodes.push_back(node);
   used = 0;
   data.assign(width * height * depth, uint8(0));
}

int mws_tex_atlas::fit(uint32 i_index, uint32 i_width, uint32 i_height)
{
   glm::ivec3* node = &nodes[i_index];
   int x = node->x;
   int y = node->y;
   int width_left = i_width;
   uint32 i = i_index;

   if ((x + i_width) > (width - 1))
   {
      return -1;
   }

   while (width_left > 0)
   {
      node = &nodes[i];

      if (node->y > y)
      {
         y = node->y;
      }

      if ((y + i_height) > (height - 1))
      {
         return -1;
      }

      width_left -= node->z;
      ++i;
   }

   return y;
}

void mws_tex_atlas::merge()
{
   for (uint32 i = 0; i < nodes.size() - 1; ++i)
   {
      glm::ivec3* node = &nodes[i];
      glm::ivec3* next = &nodes[i + 1];

      if (node->y == next->y)
      {
         node->z += next->z;
         nodes.erase(nodes.begin() + i + 1);
         --i;
      }
   }
}
