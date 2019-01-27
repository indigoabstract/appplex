#pragma once

#include "appplex-conf.hpp"

#if defined MOD_MWS && defined MOD_VKB

#include "../mws.hpp"
#include <unordered_map>


class mws_font;
class text_vxo;
class vkb_voronoi_main;


class mws_vkb : public mws_virtual_keyboard
{
public:
   static mws_sp<mws_vkb> gi();
   virtual ~mws_vkb() {}
   virtual void receive(mws_sp<mws_dp> i_dp) override;
   virtual void update_state() override;
   virtual void update_view(mws_sp<mws_camera> g) override;
   virtual void on_resize() override;
   virtual void set_target(mws_sp<mws_text_area> i_tbx) override;
   void load(std::string i_filename);

protected:
   mws_vkb() {}
   virtual void setup() override;
   // when finished, call this to hide the keyboard
   virtual void done();
   std::string get_key_name(key_types i_key_id) const;
   key_types get_key_type(const std::string& i_key_name) const;

   mws_sp<mws_text_area> ta;
   mws_sp<vkb_voronoi_main> vk;
   mws_sp<text_vxo> vk_keys;
   std::unordered_map<key_types, std::string> key_map;
   std::vector<key_types> key_vect;
   mws_sp<mws_font> key_font;
   mws_sp<mws_font> selected_key_font;
   int selected_kernel_idx = -1;
   int current_key_idx = -1;
   std::string vkb_filename;
   static mws_sp<mws_vkb> inst;
};

#endif
