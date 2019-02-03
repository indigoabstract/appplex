#pragma once

#include "pfm-def.h"
#include <glm/fwd.hpp>


class gfx_color
{
public:
   uint8 r, g, b, a;

   gfx_color();
   gfx_color(uint8 ir, uint8 ig, uint8 ib, uint8 ia = 255);
   gfx_color(const uint8* c);
   gfx_color(const gfx_color& c);
   gfx_color(int argb);
   ~gfx_color() {}

   void from_float(float fr, float fg, float fb);
   void from_float(float fr, float fg, float fb, float fa);
   void to_float(float& fr, float& fg, float& fb, float& fa);

   gfx_color operator = (gfx_color const & c);
   gfx_color operator + (gfx_color const & c);
   gfx_color operator * (gfx_color const & c);
   gfx_color operator * (float f);
   operator uint8* ();
   bool operator == (gfx_color const & c);
   bool operator != (gfx_color const & c);
   gfx_color operator += (gfx_color const & c);
   uint8* rgba_array() { return &r; }
   int rgba() { return (r << 24) | (g << 16) | (b << 8) | a; };
   int argb() { return (a << 24) | (r << 16) | (g << 8) | (b << 0); };
   glm::vec4 to_vec4()const;
   bool is_white();
   uint8 intensity();
   void to_hsv(float& hue, float& saturation, float& value);
   void hsv2rgb_smooth(float& hue, float& saturation, float& value);
   void from_hsv(float hue, float saturation, float value);
   void to_hsb(float& hue, float& saturation, float& brightness);
   void from_hsb(float hue, float saturation, float brightness);

   static gfx_color interpolate(const gfx_color& c1, const gfx_color& c2, float f);

   // color list
   class colors
   {
   public:
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
   void clamp(int& r, int& g, int& b, int& a);
};
