#pragma once

#include "pfm.hxx"
#include <glm/vec2.hpp>
#include <string>
#include <vector>
#include <cmath>


class ms_linear_transition;
class mws_font;


template <typename T> class basic_time_slider
{
public:
   basic_time_slider(T i_slide_time = 5.f);

   bool is_enabled() const;
   T get_value() const;
   int get_loop_count() const;
   void start(T i_slide_time = 0.f);
   void start_int(uint32 i_slide_time);
   void stop();
   void update();

private:

   static uint32 fp_2_int_time(T i_seconds);

   bool enabled;
   uint32 start_time;
   uint32 slide_time;
   T slider;
   int loop_count;
};

template <typename T> basic_time_slider<T>::basic_time_slider(T i_slide_time)
{
   slide_time = fp_2_int_time(i_slide_time);
   enabled = false;
   start_time = 0;
   slider = 0.f;
   loop_count = 0;
}

template <typename T> bool basic_time_slider<T>::is_enabled() const
{
   return enabled;
}

template <typename T> T basic_time_slider<T>::get_value() const
{
   return slider;
}

template <typename T> int basic_time_slider<T>::get_loop_count() const
{
   return loop_count;
}

template <typename T> void basic_time_slider<T>::start(T i_slide_time)
{
   uint32 st = 0;

   if (i_slide_time > 0.f)
   {
      st = fp_2_int_time(i_slide_time);
   }

   start_int(st);
}

template <typename T> void basic_time_slider<T>::start_int(uint32 i_slide_time)
{
   if (i_slide_time > 0)
   {
      slide_time = i_slide_time;
   }

   enabled = true;
   start_time = mws::time::get_time_millis();
   slider = 0.f;
   loop_count = 0;
}

template <typename T> void basic_time_slider<T>::stop()
{
   enabled = false;
   slider = 0.f;
}

template <typename T> void basic_time_slider<T>::update()
{
   if (!enabled)
   {
      return;
   }

   uint32 now = mws::time::get_time_millis();
   uint32 start_delta = now - start_time;

   if (start_delta < slide_time)
   {
      slider = T(start_delta) / slide_time;
      loop_count++;
   }
   else
   {
      enabled = false;
      slider = 1.f;
   }
}

template <typename T> uint32 basic_time_slider<T>::fp_2_int_time(T i_seconds)
{
   T int_part = std::floor(i_seconds);
   uint32 sec = int(int_part) * 1000;
   T fract_part = i_seconds - int_part;
   uint32 ms = int(fract_part * 1000.f);
   uint32 total_time = sec + ms;

   return total_time;
}


template <typename T> class ping_pong_time_slider
{
public:
   ping_pong_time_slider(T i_slide_time = 5.f);

   bool is_enabled() const;
   T get_value() const;
   int get_loop_count() const;
   void start(T i_slide_time = 0.f);
   void start_int(uint32 i_slide_time);
   void stop();
   void update();

private:

   static uint32 fp_2_int_time(T i_seconds);

   bool enabled;
   bool forward;
   uint32 start_time;
   uint32 last_start_delta;
   uint32 slide_time;
   T slider;
   int loop_count;
};

template <typename T> ping_pong_time_slider<T>::ping_pong_time_slider(T i_slide_time)
{
   slide_time = fp_2_int_time(i_slide_time);
   enabled = false;
   forward = true;
   start_time = 0;
   slider = 0.f;
   loop_count = 0;
}

template <typename T> bool ping_pong_time_slider<T>::is_enabled() const
{
   return enabled;
}

template <typename T> T ping_pong_time_slider<T>::get_value() const
{
   return slider;
}

template <typename T> int ping_pong_time_slider<T>::get_loop_count() const
{
   return loop_count;
}

template <typename T> void ping_pong_time_slider<T>::start(T i_slide_time)
{
   uint32 st = 0;

   if (i_slide_time > 0.f)
   {
      st = fp_2_int_time(i_slide_time);
   }

   start_int(st);
}

template <typename T> void ping_pong_time_slider<T>::start_int(uint32 i_slide_time)
{
   if (i_slide_time > 0)
   {
      slide_time = i_slide_time;
   }

   forward = enabled = true;
   start_time = mws::time::get_time_millis();
   last_start_delta = 0;
   slider = 0.f;
   loop_count = 0;
}

template <typename T> void ping_pong_time_slider<T>::stop()
{
   enabled = false;
   slider = 0.f;
}

template <typename T> void ping_pong_time_slider<T>::update()
{
   if (!enabled)
   {
      return;
   }

   uint32 now = mws::time::get_time_millis();
   uint32 start_delta = (now - start_time) % slide_time;

   if (start_delta < last_start_delta)
   {
      // reverse slider direction
      forward = !forward;

      if (forward)
      {
         loop_count++;
      }
   }

   if (forward)
   {
      slider = T(start_delta) / slide_time;
   }
   else
   {
      slider = T(slide_time - start_delta) / slide_time;
   }

   last_start_delta = start_delta;
}

template <typename T> uint32 ping_pong_time_slider<T>::fp_2_int_time(T i_seconds)
{
   T int_part = std::floor(i_seconds);
   uint32 sec = int(int_part) * 1000;
   T fract_part = i_seconds - int_part;
   uint32 ms = int(fract_part * 1000.f);
   uint32 total_time = sec + ms;

   return total_time;
}


inline float inverse(float v, float vmax)
{
   float i = v / vmax;
   float r = 1 + powf((1 - i), 3) * -1;
   return r;
}


class kinetic_scrolling
{
public:
   kinetic_scrolling() { reset(); }

   // decrease scrolling speed once mouse is released
   glm::vec2 update()
   {
      glm::vec2 d(0.f);

      if (active)
      {
         // if there is still some scrolling energy
         if (decay > 0)
         {
            uint32 delta_t = mws::time::get_time_millis() - decay_start;
            decay = 1.f - inverse((float)delta_t, (float)decay_maxmillis);

            //decay = 1 - inverse(mws_mod_ctrl::get_current_mod()->update_ctrl->getTime() - decay_start, decay_maxmillis);
            //decay = 1 - sigmoid(mws::getCurrentTime() - decay_start, decay_maxmillis);
            //decay=decay*.9;
            d.x = speedx * decay;
            d.y = speedy * decay;
         }
         else
         {
            decay = 0;
            active = false;
         }
      }

      return d;
   }

   // the initial scrolling speed is the speed it was being dragged
   void begin(float i_x, float i_y) { speedx = i_x - startx; speedy = i_y - starty; startx = i_x; starty = i_y; }
   // while grabbing is occuring, the scrolling link to mouse movement
   void grab(float i_x, float i_y) { reset(); startx = i_x; starty = i_y; }

   void start_slowdown()
   {
      // the mouse has been released and we can start slowing the scroll
      // the speed starts at 100% of the current scroll speed
      // record the time so we can calualte the decay rate

      if (speedx != 0 || speedy != 0)
      {
         decay = 1;
         decay_start = mws::time::get_time_millis();

         //decay_start = mws_mod_ctrl::get_current_mod()->update_ctrl->getTime();
         active = true;
      }
   }

   void reset() { speedx = 0; speedy = 0; decay_maxmillis = 3500; decay = 0; active = false; }
   bool is_active() const { return active; }
   glm::vec2 get_speed() const { return glm::vec2(speedx, speedy); }

private:
   float speedx;
   float speedy;
   float startx;
   float starty;
   float decay;
   uint32 decay_start;
   int decay_maxmillis;
   bool active;
};


class slide_scrolling
{
public:
   enum scroll_dir
   {
      SD_LEFT_RIGHT,
      SD_RIGHT_LEFT,
      SD_UP_DOWN,
      SD_DOWN_UP,
   };

   slide_scrolling(int transitionms);

   bool is_finished();
   void start();
   void stop();
   void update();

   const mws_sp<ms_linear_transition> get_transition();
   void set_scroll_dir(scroll_dir itype);

   glm::vec2 srcpos;
   glm::vec2 dstpos;

private:
   mws_sp<ms_linear_transition> mslt;
   scroll_dir type;
   float pstart, pstop;
};


float sigmoid(float v, float vmax);

inline float smooth_step(float x)
{
   return sqrtf(x);
   //return (x * x * (3 - 2 * x));
}

float interpolate_smooth_step(float intervalPosition, float start, float end, int octaveCount);
//void draw_border_rect(mws_sp<mws_camera> g, float x, float y, float w, float h, float borderSize, int borderColor);
//void fill_border_rect(mws_sp<mws_camera> g, float x, float y, float w, float h, float borderSize, int fillColor, int borderColor);
//void draw_bar(mws_sp<mws_camera> g, int x, int y, int w, int h, int color);
