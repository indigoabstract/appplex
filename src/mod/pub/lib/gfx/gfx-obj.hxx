#pragma once

#include "pfm-def.h"
#include <memory>
#include <string>

class gfx;


class gfx_obj : public std::enable_shared_from_this<gfx_obj>
{
public:
   enum e_gfx_obj_type
   {
      // generic gfx obj
      e_obj,
      e_mat,
      e_rt,
      e_shader,
      e_tex,

      // nodes in the scene
      e_node,
      e_cam,
      e_vxo,
      e_mws,
   };

   virtual ~gfx_obj();
   virtual e_gfx_obj_type get_type()const = 0;
   virtual bool is_valid()const { return true; }
   mws_sp<gfx> gi();

protected:
   gfx_obj(mws_sp<gfx> i_gi);
   mws_sp<gfx_obj> get_inst();

private:
   friend class gfx;

   mws_wp<gfx> g;
   static inline uint32_t obj_idx = 0;
};
