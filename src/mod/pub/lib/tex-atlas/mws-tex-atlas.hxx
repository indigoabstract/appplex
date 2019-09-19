#pragma once


#include "pfm-def.h"
#include "gfx-quad-2d.hxx"
#include "gfx-vxo.hxx"
#include <glm/vec4.hpp>
#include <vector>


class gfx_tex;


// packs several small regions into a single texture. taken from freetype-gl's texture atlas
class mws_tex_atlas
{
public:
   struct region
   {
      glm::ivec4 rect;
      int id;
   };

   /**
    * Creates a new empty texture atlas.
    * @param width width of the atlas
    * @param height height of the atlas
    * @param depth bit depth of the atlas
    * @return a new empty texture atlas.
    */
   static mws_sp<mws_tex_atlas> nwi(uint32 i_width, uint32 i_height, uint32 i_depth);

   // get the underlying texture
   mws_sp<gfx_tex> get_tex() const;

   void create_tex();

   // transfers atlas data from client(CPU) to texture(GPU) memory
   void upload();

   const region& get_region_by_id(int i_region_id) const;

   /**
    *  Allocate a new region in the atlas.
    *  @param width width of the region to allocate
    *  @param height height of the region to allocate
    *  @return coordinates of the allocated region
    */
   const region& get_region(uint32 i_width, uint32 i_height);

   /**
    *  Upload data to the specified atlas region in CPU memory.
    *  @param x x coordinate the region
    *  @param y y coordinate the region
    *  @param width width of the region
    *  @param height height of the region
    *  @param data data to be uploaded into the specified region
    *  @param stride stride of the data
    */
   void set_region(const region& i_reg, const uint8* i_data, uint32 i_stride);

   // clear the atlas
   void clear();

protected:
   mws_tex_atlas() { inst_count++; }
   int fit(uint32 i_index, uint32 i_width, uint32 i_height);
   void merge();

   std::vector<glm::ivec3> nodes;
   std::vector<region> region_vect;
   // width, height (in pixels) and depth (in bytes) of the underlying texture
   uint32 width = 0;
   uint32 height = 0;
   uint32 depth = 0;
   // allocated surface size
   uint32 used = 0;
   // atlas data
   std::vector<uint8> data;
   mws_sp<gfx_tex> tex;
   static inline region out_of_space = { { -1, -1, 0, 0 }, -1 };
   static inline uint32 inst_count = 0;
};


class mws_atlas_sprite_list : public gfx_vxo
{
public:
   static mws_sp<mws_atlas_sprite_list> nwi(mws_sp<mws_tex_atlas> i_atlas);
   void add(int i_sprite_id, float i_x, float i_y, gfx_quad_2d::e_anchor_types i_anchor = gfx_quad_2d::e_top_left);
   void push_data();
   void clear();

protected:
   mws_atlas_sprite_list();

   mws_sp<mws_tex_atlas> atlas;
   struct vx_data { glm::vec3 pos; glm::vec2 tex; };
   std::vector<vx_data> vx_buff;
   std::vector<gfx_indices_type> idx_buff;
};
