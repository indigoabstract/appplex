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
      e_node,
      e_cam,
      e_mat,
      e_rt,
      e_shader,
      e_tex,
      e_vxo,
      e_mws,
   };

   //const std::string& get_id();
   virtual e_gfx_obj_type get_type()const = 0;
   virtual bool is_valid()const { return true; }
   std::shared_ptr<gfx> gfx_ref();

protected:
   gfx_obj(std::shared_ptr<gfx> i_gi);
   std::shared_ptr<gfx_obj> get_inst();

   //std::string oid;

private:
   friend class gfx;

   std::weak_ptr<gfx> g;
   static uint32 obj_idx;
};
