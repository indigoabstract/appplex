#pragma once

#include "pfm.hpp"
#include <memory>
#include <string>

class gfx;


class gfx_obj : public std::enable_shared_from_this<gfx_obj>
{
public:
   enum e_gfx_obj_type
   {
      e_gfx_node,
      e_gfx_cam,
      e_gfx_mat,
      e_gfx_rt,
      e_gfx_shader,
      e_gfx_tex,
      e_gfx_vxo,
   };

   const std::string& get_id();
   virtual e_gfx_obj_type get_type()const = 0;
   virtual bool is_valid()const { return true; }
   std::shared_ptr<gfx> gfx_ref();

protected:
   gfx_obj(std::shared_ptr<gfx> i_gi);
   std::shared_ptr<gfx_obj> get_inst();

   std::string oid;

private:
   friend class gfx;

   std::weak_ptr<gfx> g;
   static uint32 obj_idx;
};
