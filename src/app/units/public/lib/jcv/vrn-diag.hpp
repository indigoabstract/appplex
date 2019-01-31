#pragma once

#include "appplex-conf.hpp"

#ifdef MOD_JCV

#include <vector>

class mws_vrn_data;


class mws_vrn_diag
{
public:
   struct idx_dist
   {
      int idx;
      float dist;
   };


   static mws_sp<mws_vrn_diag> nwi();
   virtual void init_data(mws_sp<mws_vrn_data> i_vdata, std::vector<float>& vx, std::vector<float>& vy) = 0;
   virtual const mws_sp<mws_vrn_data> get_data() const = 0;
   virtual idx_dist get_kernel_idx_at(float i_x, float i_y) const = 0;
   virtual void update_data() = 0;
   virtual void move_kernel_to(int i_idx, float i_x, float i_y) = 0;
   virtual void insert_kernel_at(float i_x, float i_y) = 0;
   virtual void remove_kernel(int i_idx) = 0;

protected:
   mws_vrn_diag() {}
};

#endif
