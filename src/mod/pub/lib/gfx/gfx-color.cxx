#include "stdafx.hxx"

#include "gfx-color.hxx"
#include "pfm.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
mws_pop_disable_all_warnings


gfx_color gfx_color::colors::black = gfx_color::from_rgba(0x000000ff);
gfx_color gfx_color::colors::blue = gfx_color::from_rgba(0x0000ffff);
gfx_color gfx_color::colors::blue_violet = gfx_color::from_rgba(0x8a2be2ff);
gfx_color gfx_color::colors::cyan = gfx_color::from_rgba(0x00ffffff);
gfx_color gfx_color::colors::dark_orange = gfx_color::from_rgba(0xff8c00ff);
gfx_color gfx_color::colors::dark_orchid = gfx_color::from_rgba(0x9932ccff);
gfx_color gfx_color::colors::dark_red = gfx_color::from_rgba(0x8b0000ff);
gfx_color gfx_color::colors::deep_pink = gfx_color::from_rgba(0xff1493ff);
gfx_color gfx_color::colors::dodger_blue = gfx_color::from_rgba(0x1e90ffff);
gfx_color gfx_color::colors::gold = gfx_color::from_rgba(0xffd700ff);
gfx_color gfx_color::colors::gray = gfx_color::from_rgba(0x808080ff);
gfx_color gfx_color::colors::green = gfx_color::from_rgba(0x00ff00ff);
gfx_color gfx_color::colors::indigo = gfx_color::from_rgba(0x4b0082ff);
gfx_color gfx_color::colors::lavender = gfx_color::from_rgba(0xe6e6faff);
gfx_color gfx_color::colors::magenta = gfx_color::from_rgba(0xff00ffff);
gfx_color gfx_color::colors::orange = gfx_color::from_rgba(0xffa500ff);
gfx_color gfx_color::colors::orchid = gfx_color::from_rgba(0xda70d6ff);
gfx_color gfx_color::colors::papaya_whip = gfx_color::from_rgba(0xffefd5ff);
gfx_color gfx_color::colors::pink = gfx_color::from_rgba(0xffc0cbff);
gfx_color gfx_color::colors::plum = gfx_color::from_rgba(0xdda0ddff);
gfx_color gfx_color::colors::red = gfx_color::from_rgba(0xff0000ff);
gfx_color gfx_color::colors::saddle_brown = gfx_color::from_rgba(0x8b4513ff);
gfx_color gfx_color::colors::salmon = gfx_color::from_rgba(0xfa8072ff);
gfx_color gfx_color::colors::slate_gray = gfx_color::from_rgba(0x708090ff);
gfx_color gfx_color::colors::spring_green = gfx_color::from_rgba(0x00ff7fff);
gfx_color gfx_color::colors::thistle = gfx_color::from_rgba(0xd8bfd8ff);
gfx_color gfx_color::colors::wheat = gfx_color::from_rgba(0xf5deb3ff);
gfx_color gfx_color::colors::white = gfx_color::from_rgba(0xffffffff);
gfx_color gfx_color::colors::yellow = gfx_color::from_rgba(0xffff00ff);


gfx_color::gfx_color() { abgr = 0; }
gfx_color::gfx_color(uint8_t i_r, uint8_t i_g, uint8_t i_b, uint8_t i_a) { r = i_r; g = i_g; b = i_b; a = i_a; }
gfx_color::gfx_color(const uint8_t* i_c) { r = i_c[0]; g = i_c[1]; b = i_c[2]; a = i_c[3]; }
gfx_color::gfx_color(const gfx_color& i_c) { abgr = i_c.abgr; }

gfx_color gfx_color::from_abgr(uint32_t i_abgr)
{
   return gfx_color(i_abgr);
}

gfx_color gfx_color::from_argb(uint32_t i_argb)
{
   return gfx_color((i_argb >> 16) & 0x000000ff, (i_argb >> 8) & 0x000000ff, (i_argb >> 0) & 0x000000ff, (i_argb >> 24) & 0x000000ff);
}

gfx_color gfx_color::from_rgba(uint32_t i_rgba)
{
   return gfx_color((i_rgba >> 24) & 0x000000ff, (i_rgba >> 16) & 0x000000ff, (i_rgba >> 8) & 0x000000ff, (i_rgba >> 0) & 0x000000ff);
}

gfx_color gfx_color::from_vec4(const glm::vec4& i_vec4)
{
   return from_float(i_vec4.r, i_vec4.g, i_vec4.b, i_vec4.a);
}

gfx_color gfx_color::from_float(float i_r, float i_g, float i_b, float i_a)
{
   int r = int(i_r * 255.0f);
   int g = int(i_g * 255.0f);
   int b = int(i_b * 255.0f);
   int a = int(i_a * 255.0f);

   clamp(r, g, b, a);

   return gfx_color(r, g, b, a);
}

void gfx_color::to_float(float& fr, float& fg, float& fb, float& fa)
{
   fr = (float)r / 255.0f;
   fg = (float)g / 255.0f;
   fb = (float)b / 255.0f;
   fa = (float)a / 255.0f;
}

gfx_color gfx_color::operator = (gfx_color const& c)
{
   r = c.r;
   g = c.g;
   b = c.b;
   a = c.a;

   return *this;
}

gfx_color gfx_color::operator + (gfx_color const& c)
{
   int rr = (c.r + r);
   int gg = (c.g + g);
   int bb = (c.b + b);
   int aa = (c.a + a);

   clamp(rr, gg, bb, aa);

   return gfx_color((uint8_t)rr, (uint8_t)gg, (uint8_t)bb, (uint8_t)aa);
}

gfx_color gfx_color::operator * (gfx_color const& c)
{
   int rr = (c.r * r) / 255;
   int gg = (c.g * g) / 255;
   int bb = (c.b * b) / 255;
   int aa = (c.a * a) / 255;

   return gfx_color((uint8_t)rr, (uint8_t)gg, (uint8_t)bb, (uint8_t)aa);
}


gfx_color gfx_color::operator * (float f)
{
   int rr = (int)(r * f);
   int gg = (int)(g * f);
   int bb = (int)(b * f);
   int aa = (int)(a);// * f);

   clamp(rr, gg, bb, aa);

   return gfx_color((uint8_t)rr, (uint8_t)gg, (uint8_t)bb, (uint8_t)aa);
}

gfx_color::operator uint8_t* ()
{
   return &r;
}

bool gfx_color::operator == (gfx_color const& c)
{
   const long* l = (long*)&r;
   const long* l2 = (long*)&c.r;

   return *l == *l2;
}

bool gfx_color::operator != (gfx_color const& c)
{
   const long* l = (long*)&r;
   const long* l2 = (long*)&c.r;

   return *l != *l2;
}

gfx_color gfx_color::operator += (gfx_color const& c)
{
   int rr = (c.r + r);
   int gg = (c.g + g);
   int bb = (c.b + b);
   int aa = (c.a + a);

   clamp(rr, gg, bb, aa);

   r = (uint8_t)rr;
   g = (uint8_t)gg;
   b = (uint8_t)bb;
   a = (uint8_t)aa;

   return *this;
}

uint32_t gfx_color::to_argb() const { return (a << 24) | (r << 16) | (g << 8) | (b << 0); };

glm::vec4 gfx_color::to_vec4() const
{
   return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

uint8_t gfx_color::intensity()
{
   float h, s, b;
   to_hsb(h, s, b);

   return (uint8_t)(b * 255.0f);
}

void gfx_color::to_hsv(float& hue, float& saturation, float& value)
{
   glm::vec4 c = to_vec4();
   glm::vec3 rgb = glm::clamp(glm::abs(glm::mod(c.x * 6.f + glm::vec3(0.f, 4.f, 2.f), 6.f) - 3.f) - 1.f, 0.f, 1.f);

   glm::vec3 res = c.z * glm::mix(glm::vec3(1.0), rgb, c.y);
   hue = res.x;
   saturation = res.y;
   value = res.z;
}

void gfx_color::hsv2rgb_smooth(float& hue, float& saturation, float& value)
{
   glm::vec4 c = to_vec4();
   glm::vec3 rgb = glm::clamp(glm::abs(glm::mod(c.x * 6.f + glm::vec3(0.f, 4.f, 2.f), 6.f) - 3.f) - 1.f, 0.f, 1.f);
   rgb = rgb * rgb * (3.f - 2.f * rgb); // cubic smoothing	
   glm::vec3 res = c.z * glm::mix(glm::vec3(1.0), rgb, c.y);
   hue = res.x;
   saturation = res.y;
   value = res.z;
}

void gfx_color::from_hsv(float hue, float saturation, float value)
{
   glm::vec3 c(hue, saturation, value);
   c = glm::mix(glm::vec3(1.f), glm::clamp((abs(glm::fract(c.x + glm::vec3(3.f, 2.f, 1.f) / 3.f) * 6.f - 3.f) - 1.f), 0.f, 1.f), c.y) * c.z;
   from_float(c.r, c.g, c.b);
}

void gfx_color::to_hsb(float& hue, float& saturation, float& brightness)
{
   float fr, fg, fb, fa;
   to_float(fr, fg, fb, fa);
   const float max = glm::max(glm::max(fr, fg), fb);
   const float min = glm::min(glm::min(fr, fg), fb);
   float delta;

   brightness = max;				// v

   delta = max - min;

   if (max != 0)
   {
      saturation = delta / max;		// s
   }
   else
   {
      // fr = fg = fb = 0		// s = 0, v is undefined
      saturation = 0;
      hue = 0.0f;
      return;
   }

   if (fr == max)
   {
      hue = (fg - fb) / delta;		// between yellow & magenta
   }
   else if (fg == max)
   {
      hue = 2 + (fb - fr) / delta;	// between cyan & yellow
   }
   else
   {
      hue = 4 + (fr - fg) / delta;	// between magenta & cyan
   }

   hue /= 6.0f;				// degrees

   if (hue < 0.0f)
   {
      hue += 1.0f;
   }

   if (saturation == 0.0f)
      hue = 0.0f;
}

void gfx_color::from_hsb(float hue, float saturation, float brightness)
{
   if (saturation == 0.0f)
   {
      from_float(brightness, brightness, brightness);
      return;
   }

   float h = hue * 6.0f;
   int i = (int)glm::floor(h);
   float f = h - i;
   float p = brightness * (1 - saturation);
   float q = brightness * (1 - saturation * f);
   float t = brightness * (1 - saturation * (1 - f));

   float fr, fg, fb;
   switch (i)
   {
   case 0:
      fr = brightness;
      fg = t;
      fb = p;
      break;

   case 1:
      fr = q;
      fg = brightness;
      fb = p;
      break;

   case 2:
      fr = p;
      fg = brightness;
      fb = t;
      break;

   case 3:
      fr = p;
      fg = q;
      fb = brightness;
      break;

   case 4:
      fr = t;
      fg = p;
      fb = brightness;
      break;

   default:
      fr = brightness;
      fg = p;
      fb = q;
      break;
   }

   from_float(fr, fg, fb);
}

std::string gfx_color::to_str() const
{
   glm::vec4 v4 = to_vec4();

   return mws_to_str_fmt("[ %1.2f, %1.2f, %1.2f, %1.2f ]", v4.r, v4.g, v4.b, v4.a);
}

gfx_color gfx_color::mix(const gfx_color& i_c0, const gfx_color& i_c1, float i_mixf)
{
   float one_minus_mixf = 1.f - i_mixf;
   return gfx_color(
      uint8_t(one_minus_mixf * i_c0.r + i_mixf * i_c1.r),
      uint8_t(one_minus_mixf * i_c0.g + i_mixf * i_c1.g),
      uint8_t(one_minus_mixf * i_c0.b + i_mixf * i_c1.b),
      uint8_t(one_minus_mixf * i_c0.a + i_mixf * i_c1.a));
}

void gfx_color::clamp(int& i_r, int& i_g, int& i_b, int& i_a)
{
   if (i_r > 255) { i_r = 255; }
   if (i_g > 255) { i_g = 255; }
   if (i_b > 255) { i_b = 255; }
   if (i_a > 255) { i_a = 255; }
}


gfx_color_mixer::gfx_color_mixer()
{
   clear();
}

void gfx_color_mixer::clear()
{
   mixer.clear();
   mixer.set_edges(gfx_color::colors::black, gfx_color::colors::white);
}

int gfx_color_mixer::set_color_at(gfx_color i_color, float i_position)
{
   return mixer.set_val_at(i_color, i_position);
}

gfx_color gfx_color_mixer::get_color_at(float i_position)
{
   return mixer.get_val_at(i_position);
}

bool gfx_color_mixer::remove_idx(uint32_t i_idx)
{
   return mixer.remove_idx(i_idx);
}
