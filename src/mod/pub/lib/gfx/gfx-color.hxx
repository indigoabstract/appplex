#pragma once

#include "pfm-def.h"
#include "min.hxx"
mws_push_disable_all_warnings
#include <glm/fwd.hpp>
#include <string>
#include <vector>
mws_pop_disable_all_warnings


class gfx_color
{
public:
   union
   {
      // 0xaabbggrr is stored in memory as 0xrrggbbaa, because little endian
      // we need this order to be compatible with opengl's RGBA8 format
      uint32 abgr;
      struct
      {
         uint8 r;
         uint8 g;
         uint8 b;
         uint8 a;
      };
   };

   gfx_color();
   gfx_color(uint8 i_r, uint8 i_g, uint8 i_b, uint8 i_a = 0xff);
   gfx_color(const uint8* i_c);
   gfx_color(const gfx_color& i_c);

   static gfx_color from_abgr(uint32 i_abgr);
   static gfx_color from_argb(uint32 i_argb);
   static gfx_color from_rgba(uint32 i_rgba);
   static gfx_color from_vec4(const glm::vec4& i_vec4);
   static gfx_color from_float(float i_r, float i_g, float i_b, float i_a = 1.f);

   void to_float(float& i_r, float& i_g, float& i_b, float& i_a);

   gfx_color operator = (gfx_color const& i_c);
   gfx_color operator + (gfx_color const& i_c);
   gfx_color operator * (gfx_color const& i_c);
   gfx_color operator * (float i_f);
   operator uint8* ();
   bool operator == (gfx_color const& i_c);
   bool operator != (gfx_color const& i_c);
   gfx_color operator += (gfx_color const& i_c);
   const uint8* rgba_array() const { return &r; }
   uint32 to_rgba() const { return abgr; };
   uint32 to_argb() const;
   glm::vec4 to_vec4() const;
   uint8 intensity();
   void to_hsv(float& i_hue, float& i_saturation, float& i_value);
   void hsv2rgb_smooth(float& i_hue, float& i_saturation, float& i_value);
   void from_hsv(float hue, float i_saturation, float i_value);
   void to_hsb(float& i_hue, float& i_saturation, float& i_brightness);
   void from_hsb(float i_hue, float i_saturation, float i_brightness);
   std::string to_str() const;
   static gfx_color mix(const gfx_color& i_c0, const gfx_color& i_c1, float i_mixf);

   // color list
   struct colors
   {
      static gfx_color black;
      static gfx_color blue;
      static gfx_color blue_violet;
      static gfx_color cyan;
      static gfx_color dark_orange;
      static gfx_color dark_orchid;
      static gfx_color dark_red;
      static gfx_color deep_pink;
      static gfx_color dodger_blue;
      static gfx_color gold;
      static gfx_color gray;
      static gfx_color green;
      static gfx_color indigo;
      static gfx_color lavender;
      static gfx_color magenta;
      static gfx_color papaya_whip;
      static gfx_color pink;
      static gfx_color plum;
      static gfx_color orange;
      static gfx_color orchid;
      static gfx_color red;
      static gfx_color saddle_brown;
      static gfx_color salmon;
      static gfx_color slate_gray;
      static gfx_color spring_green;
      static gfx_color thistle;
      static gfx_color wheat;
      static gfx_color white;
      static gfx_color yellow;
   };

private:
   gfx_color(int i_abgr) { abgr = i_abgr; }

   static void clamp(int& i_r, int& i_g, int& i_b, int& i_a);
};


class gfx_color_mixer
{
public:
   gfx_color_mixer();

   void clear();
   int set_color_at(gfx_color i_color, float i_position);
   gfx_color get_color_at(float i_position);
   bool remove_idx(uint32 i_idx);

protected:
   struct mix_f { gfx_color operator()(const gfx_color& i_c0, const gfx_color& i_c1, float i_f) { return gfx_color::mix(i_c0, i_c1, i_f); } };
   // this maps positions from [0, 1] to colors in the list
   using pos_to_color_mixer = mws_val_mixer<gfx_color, mix_f>;
   pos_to_color_mixer mixer;
};
