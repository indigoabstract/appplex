#include "stdafx.hxx"

#include "gfx-color.hxx"
#include <glm/inc.hpp>


gfx_color gfx_color::colors::black = gfx_color(0x000000);
gfx_color gfx_color::colors::blue = gfx_color(0x0000ff);
gfx_color gfx_color::colors::blue_violet = gfx_color(0x8a2be2);
gfx_color gfx_color::colors::cyan = gfx_color(0x00ffff);
gfx_color gfx_color::colors::dark_orange = gfx_color(0xff8c00);
gfx_color gfx_color::colors::dark_orchid = gfx_color(0x9932cc);
gfx_color gfx_color::colors::dark_red = gfx_color(0x8b0000);
gfx_color gfx_color::colors::deep_pink = gfx_color(0xff1493);
gfx_color gfx_color::colors::dodger_blue = gfx_color(0x1e90ff);
gfx_color gfx_color::colors::gold = gfx_color(0xffd700);
gfx_color gfx_color::colors::gray = gfx_color(0x808080);
gfx_color gfx_color::colors::green = gfx_color(0x00ff00);
gfx_color gfx_color::colors::indigo = gfx_color(0x4b0082);
gfx_color gfx_color::colors::lavender = gfx_color(0xe6e6fa);
gfx_color gfx_color::colors::magenta = gfx_color(0xff00ff);
gfx_color gfx_color::colors::orange = gfx_color(0xffa500);
gfx_color gfx_color::colors::orchid = gfx_color(0xda70d6);
gfx_color gfx_color::colors::papaya_whip = gfx_color(0xffefd5);
gfx_color gfx_color::colors::pink = gfx_color(0xffc0cb);
gfx_color gfx_color::colors::plum = gfx_color(0xdda0dd);
gfx_color gfx_color::colors::red = gfx_color(0xff0000);
gfx_color gfx_color::colors::saddle_brown = gfx_color(0x8b4513);
gfx_color gfx_color::colors::salmon = gfx_color(0xfa8072);
gfx_color gfx_color::colors::slate_gray = gfx_color(0x708090);
gfx_color gfx_color::colors::spring_green = gfx_color(0x00ff7f);
gfx_color gfx_color::colors::thistle = gfx_color(0xd8bfd8);
gfx_color gfx_color::colors::wheat = gfx_color(0xf5deb3);
gfx_color gfx_color::colors::white = gfx_color(0xffffff);
gfx_color gfx_color::colors::yellow = gfx_color(0xffff00);


#define ALPHA_CHANNEL(color)	((color & 0xff000000) >> 24)
#define RED_CHANNEL(color)		((color & 0x00ff0000) >> 16)
#define GREEN_CHANNEL(color)	((color & 0x0000ff00) >> 8)
#define BLUE_CHANNEL(color)		((color & 0x000000ff) >> 0)

gfx_color::gfx_color() { r = g = b = a = 255; }
gfx_color::gfx_color(uint8 ir, uint8 ig, uint8 ib, uint8 ia) { this->r = ir; this->g = ig; this->b = ib; this->a = ia; }
gfx_color::gfx_color(const uint8* c) { r = c[0]; g = c[1]; b = c[2]; a = c[3]; }
gfx_color::gfx_color(const gfx_color& c) { r = c.r; g = c.g; b = c.b; a = c.a; }

gfx_color::gfx_color(int argb) //rgba
{
   r = RED_CHANNEL(argb);
   g = GREEN_CHANNEL(argb);
   b = BLUE_CHANNEL(argb);
   a = ALPHA_CHANNEL(argb);

   if (a == 0)
      // if a == 0, consider it as not set
   {
      a = 255;
   }
}

void gfx_color::from_float(float fr, float fg, float fb, float fa)
{
   int rr = (int)(fr * 255.0f);
   int gg = (int)(fg * 255.0f);
   int bb = (int)(fb * 255.0f);
   int aa = (int)(fa * 255.0f);

   clamp(rr, gg, bb, aa);
   r = rr;
   g = gg;
   b = bb;
   a = aa;
}

void gfx_color::to_float(float& fr, float& fg, float& fb, float& fa)
{
   fr = (float)r / 255.0f;
   fg = (float)g / 255.0f;
   fb = (float)b / 255.0f;
   fa = (float)a / 255.0f;
}

void gfx_color::from_float(float fr, float fg, float fb)
{
   int rr = (int)(fr * 255.0f);
   int gg = (int)(fg * 255.0f);
   int bb = (int)(fb * 255.0f);
   int aa = 255;

   clamp(rr, gg, bb, aa);
   r = rr;
   g = gg;
   b = bb;
   a = aa;
}

gfx_color gfx_color::operator = (gfx_color const & c)
{
   r = c.r;
   g = c.g;
   b = c.b;
   a = c.a;

   return *this;
}

gfx_color gfx_color::operator + (gfx_color const & c)
{
   int rr = (c.r + r);
   int gg = (c.g + g);
   int bb = (c.b + b);
   int aa = (c.a + a);

   clamp(rr, gg, bb, aa);

   return gfx_color((uint8)rr, (uint8)gg, (uint8)bb, (uint8)aa);
}

gfx_color gfx_color::operator * (gfx_color const & c)
{
   int rr = (c.r * r) / 255;
   int gg = (c.g * g) / 255;
   int bb = (c.b * b) / 255;
   int aa = (c.a * a) / 255;

   return gfx_color((uint8)rr, (uint8)gg, (uint8)bb, (uint8)aa);
}


gfx_color gfx_color::operator * (float f)
{
   int rr = (int)(r * f);
   int gg = (int)(g * f);
   int bb = (int)(b * f);
   int aa = (int)(a);// * f);

   clamp(rr, gg, bb, aa);

   return gfx_color((uint8)rr, (uint8)gg, (uint8)bb, (uint8)aa);
}

gfx_color::operator uint8* ()
{
   return &r;
}

bool gfx_color::operator == (gfx_color const & c)
{
   const long* l = (long*)&r;
   const long* l2 = (long*)&c.r;

   return *l == *l2;
}

bool gfx_color::operator != (gfx_color const & c)
{
   const long* l = (long*)&r;
   const long* l2 = (long*)&c.r;

   return *l != *l2;
}

gfx_color gfx_color::operator += (gfx_color const & c)
{
   int rr = (c.r + r);
   int gg = (c.g + g);
   int bb = (c.b + b);
   int aa = (c.a + a);

   clamp(rr, gg, bb, aa);

   r = (uint8)rr;
   g = (uint8)gg;
   b = (uint8)bb;
   a = (uint8)aa;

   return *this;
}

glm::vec4 gfx_color::to_vec4()const
{
   return glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
}

bool gfx_color::is_white()
{
   const long* l = (long*)&r;

   return *l == 0xffffffff;
}

uint8 gfx_color::intensity()
{
   float h, s, b;
   to_hsb(h, s, b);

   return (uint8)(b * 255.0f);
}

void gfx_color::to_hsv(float& hue, float& saturation, float& value)
{
   glm::vec4 c = to_vec4();
   glm::vec3 rgb = glm::clamp(glm::abs(glm::mod(c.x*6.f + glm::vec3(0.f, 4.f, 2.f), 6.f) - 3.f) - 1.f, 0.f, 1.f);

   glm::vec3 res = c.z * glm::mix(glm::vec3(1.0), rgb, c.y);
   hue = res.x;
   saturation = res.y;
   value = res.z;
}

void gfx_color::hsv2rgb_smooth(float& hue, float& saturation, float& value)
{
   glm::vec4 c = to_vec4();
   glm::vec3 rgb = glm::clamp(glm::abs(glm::mod(c.x*6.f + glm::vec3(0.f, 4.f, 2.f), 6.f) - 3.f) - 1.f, 0.f, 1.f);
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
   int i = floor(h);
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

gfx_color gfx_color::interpolate(const gfx_color& c1, const gfx_color& c2, float f)
{
   int r = (c2.r - c1.r) * f + c1.r;
   int g = (c2.g - c1.g) * f + c1.g;
   int b = (c2.b - c1.b) * f + c1.b;
   int a = (c2.a - c1.a) * f + c1.a;
   return gfx_color(r, g, b, a);
}

void gfx_color::clamp(int& r, int& g, int& b, int& a)
{
   if (r > 255) r = 255;
   if (g > 255) g = 255;
   if (b > 255) b = 255;
   if (a > 255) a = 255;
}
