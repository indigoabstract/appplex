#pragma once

#include "pfm.hpp"
#include "min.hpp"
#include "gfx-shader.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

class gfx_tex;


class vx_attribute : public gfx_input
{
public:
   vx_attribute(const vx_attribute& iattribute)
   {
      name = iattribute.name;
      data_type = iattribute.data_type;
      component_count = iattribute.component_count;
      aligned_size = iattribute.aligned_size;
   }

   vx_attribute(std::string iattribute_name, e_data_type itype)
   {
      name = iattribute_name;
      data_type = itype;

      switch (data_type)
      {
      case ivec1:
         aligned_size = 4;
         component_count = 1;
         break;
      case ivec2:
         aligned_size = 4;
         component_count = 2;
         break;

      case ivec3:
         aligned_size = 4;
         component_count = 3;
         break;

      case ivec4:
         aligned_size = 4;
         component_count = 4;
         break;

      case vec1:
         aligned_size = 4;
         component_count = 1;
         break;

      case vec2:
         aligned_size = 8;
         component_count = 2;
         break;

      case vec3:
         aligned_size = 12;
         component_count = 3;
         break;

      case vec4:
         aligned_size = 16;
         component_count = 4;
         break;

      default:
         throw ia_exception("unknown value");
      }
   }

   gfx_int get_component_count()const
   {
      return component_count;
   }

   int get_aligned_size()const
   {
      return aligned_size;
   }

private:
   gfx_int component_count;
   int aligned_size;
};


class gfx_uniform : public gfx_input
{
public:
   gfx_uniform(const gfx_uniform& iuniform)
   {
      name = iuniform.name;
      data_type = iuniform.data_type;
   }

   gfx_uniform(std::string iuniform_name, e_data_type itype)
   {
      name = iuniform_name;
      data_type = itype;
   }
};


class gfx_util
{
public:
   static std::shared_ptr<vx_attribute> parse_attribute(std::string iattribute);
   static std::vector<std::shared_ptr<vx_attribute> > parse_attribute_list(std::string iattr_list);
   static std::shared_ptr<gfx_uniform> parse_uniform(std::string iuniform);
   static void draw_tex(std::shared_ptr<gfx_tex> itex, float itx, float ity);
   static void draw_tex(std::shared_ptr<gfx_tex> itex, float itx, float ity, float iw, float ih);
   static const glm::vec4& get_pos_from_tf_mx(const glm::mat4& i_tranform);
   static const glm::quat get_orientation_from_tf_mx(const glm::mat4& i_tranform);
   static const glm::vec3 get_scale_from_tf_mx(const glm::mat4& i_tranform);
   static glm::mat4 mat4_from_trs(const glm::vec3& translate, const glm::quat& rotate, const glm::vec3& scale);
   static glm::quat quat_from_two_vectors(glm::vec3 u, glm::vec3 v);
   static glm::quat look_at(glm::vec3 direction, glm::vec3 desiredUp);
   static int get_tex_2d_mipmap_count(int iwidth, int iheight);
   static uint32 next_power_of_2(uint32 in);

   static void init();
};
