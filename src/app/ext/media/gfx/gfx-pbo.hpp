#pragma once

#include "pfm.hpp"
#include "gfx-tex.hpp"
#include <functional>


class gfx_rt;
class gfx_quad_2d;
class gfx_camera;
class gfx_tex_info;


enum class mws_read_method
{
   e_map_buff,
   e_map_buff_pixels_buff,
   e_get_buff,
};

class gfx_readback : public gfx_obj
{
public:
   static std::shared_ptr<gfx_readback> nwi(std::shared_ptr<gfx> i_gi = nullptr);
   e_gfx_obj_type get_type()const override;
   int get_pbo_size() const;
   const std::vector<uint8>& get_pbo_pixels() const;
   void rewind();
   void set_params(int i_width, int i_height, std::string i_format);
   void set_read_method(mws_read_method i_read_method);
   mws_read_method get_read_method() const;
   void update();

   std::function<void(const gfx_readback* i_rb, gfx_ubyte* i_data, int i_size)> on_data_recv_handler;

private:
   gfx_readback(std::shared_ptr<gfx> i_gi);
   void set_dimensions(int i_width, int i_height);
   void set_pbo_count(int i_pbo_count);

   int width = 0;
   int height = 0;
   int pbo_count = 0;
   std::vector<gfx_uint> pbo_id_vect;
   int pbo_index = 0;
   std::vector<uint8> pbo_pixels;
   int pbo_data_size = 0;
   int frame_idx = 0;
   mws_sp<gfx_tex_info> ti;
   mws_read_method read_method = mws_read_method::e_map_buff;
};


class mws_pbo_bundle
{
public:
   mws_pbo_bundle(mws_sp<gfx> i_gfx_inst, int i_width, int i_height, std::string i_format);
   void set_on_data_recv_handler(std::function<void(const gfx_readback* i_rb, gfx_ubyte* i_data, int i_size)> i_handler);
   void set_tex(std::shared_ptr<gfx_tex> i_tex);
   void update(std::shared_ptr<gfx_camera> i_cam);

   mws_sp<gfx_rt> rt;
   mws_sp<gfx_tex> rt_tex;
   mws_sp<gfx_quad_2d> rt_quad;
   mws_sp<gfx_readback> readback;
};