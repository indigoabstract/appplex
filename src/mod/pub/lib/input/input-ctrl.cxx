#include "stdafx.hxx"

#include "input-ctrl.hxx"
#include "pfm-def.h"
#include "mws-mod.hxx"
mws_push_disable_all_warnings
#include <glm/inc.hpp>
mws_pop_disable_all_warnings


const std::string mws_ptr_evt::ptr_evt_type = "ts-";

mws_sp<mws_ptr_evt> mws_ptr_evt::nwi() { return mws_sp<mws_ptr_evt>(new mws_ptr_evt()); }

void mws_ptr_evt::process(mws_sp<mws_receiver> i_dst)
{
   mws_dp::process(i_dst);

   //if (type == touch_ended)
   //{
   //   int x = 3;
   //}
   //mws_print("%s\n", get_name().c_str());
}

mws_sp<mws_ptr_evt> mws_ptr_evt::as_pointer_evt(mws_sp<mws_dp> i_dp)
{
   return static_pointer_cast<mws_ptr_evt>(i_dp);
}

bool mws_ptr_evt::is_multitouch()
{
   return touch_count > 1;
}

// pointer_down_count
// params:
// return:
//    return number of fingers currently pressed when this event was generated, and plus one if mouse button is down
int mws_ptr_evt::pointer_down_count()
{
   int count = touch_count;

   //if (mMousePressed)
   //{
   //   count++;
   //}

   return count;
}

// get_pointer_press_by_index
// params:
//    PointerIndex... pointer index
// return:
//    returns a touch point at PointerIndex if it exists, else if PointerIndex is 0 return mouse press if it exists, else return null
const mws_ptr_evt::touch_point* mws_ptr_evt::get_pointer_press_by_index(uint32 i_pointer_index)
{
   if (touch_count > i_pointer_index)
   {
      return &points[i_pointer_index];
   }
   //else if (PointerIndex == 0 && mMousePressed)
   //{
   //   return mMousePressed.get();
   //}

   return nullptr;
}

const mws_ptr_evt::touch_point* mws_ptr_evt::find_point(uintptr_t i_touch_id) const
{
   for (uint32 i = 0; i < touch_count; i++)
   {
      if (points[i].identifier == i_touch_id)
      {
         return &points[i];
      }
   }

   return nullptr;
}

bool mws_ptr_evt::same_touches(const mws_ptr_evt& i_other) const
{
   if (i_other.touch_count == this->touch_count)
   {
      for (uint32 i = 0; i < this->touch_count; i++)
      {
         if (nullptr == this->find_point(i_other.points[i].identifier))
         {
            return false;
         }
      }

      return true;
   }

   return false;
}

glm::vec2 mws_ptr_evt::touch_pos(uintptr_t i_touch_id) const
{
   const touch_point* p = this->find_point(i_touch_id);
   mws_assert(p);

   return glm::vec2(p->x, p->y);
}

std::string mws_ptr_evt::get_evt_type()
{
   switch (type)
   {
   case touch_invalid: return "touch_invalid";
   case touch_began: return "touch_began";
   case touch_moved: return "touch_moved";
   case touch_ended: return "touch_ended";
   case touch_cancelled: return "touch_cancelled";
   case mouse_wheel: return "mouse_wheel";
   }

   return "n/a";
}

glm::vec2 mws_ptr_evt::get_pos(const mws_ptr_evt::touch_point& i_tp)
{
   return glm::vec2(i_tp.x, i_tp.y);
}


mws_touch_ctrl::mws_touch_ctrl()
{
   //if (mws_debug_enabled) { mws_dbg::set_flags(mws_dbg::pfm_touch); }
   queue_tab.resize(2);
   queue_ptr = &queue_tab[queue_idx];
}

mws_sp<mws_touch_ctrl> mws_touch_ctrl::nwi()
{
   return mws_sp<mws_touch_ctrl>(new mws_touch_ctrl());
}

mws_sp<mws_touch_ctrl> mws_touch_ctrl::get_instance()
{
   return shared_from_this();
}

bool mws_touch_ctrl::is_pointer_released()
{
   return !is_pointer_down;
}

void mws_touch_ctrl::update()
{
   // set the current input queue as the queue for processing the input
   std::vector<mws_sp<mws_ptr_evt>>* input_queue_ptr = &queue_tab[queue_idx];

   // switch queues, so the currently empty queue is used for taking input events
   queue_idx = (queue_idx + 1) % 2;
   queue_tab[queue_idx].clear();
   queue_ptr = &queue_tab[queue_idx];

   if (!input_queue_ptr->empty())
   {
      for (auto i_pe : *input_queue_ptr)
      {
         if (mws_dbg::enabled(mws_dbg::pfm_touch))
         {
            const char* evt_type = nullptr;

            switch (i_pe->type)
            {
            case mws_ptr_evt::touch_invalid:
               break;

            case mws_ptr_evt::touch_began:
               if (!evt_type) { evt_type = "began"; }
            case mws_ptr_evt::touch_moved:
               if (!evt_type) { evt_type = "moved"; }
            case mws_ptr_evt::touch_ended:
            {
               if (!evt_type) { evt_type = "ended"; }
               std::string msg = mws_to_str_fmt("ptr-%s[ ", evt_type);

               for (uint32 k = 0; k < i_pe->touch_count; k++)
               {
                  const mws_ptr_evt::touch_point& pt = i_pe->points[k];

                  if (pt.is_changed)
                  {
                     msg += mws_to_str_fmt("%d[%4.2f, %4.2f] !CHANGED!, ", pt.identifier, pt.x, pt.y);
                  }
                  else
                  {
                     msg += mws_to_str_fmt("%d[%4.2f, %4.2f], ", pt.identifier, pt.x, pt.y);
                  }
               }
               mws_println("%s]", msg.c_str());
               break;
            }

            case mws_ptr_evt::touch_cancelled:
               mws_println("ptr-cancelled[  ]");
               break;

            case mws_ptr_evt::mouse_wheel:
               mws_println("ptr-mouse-wheel[ %4.2f ]", i_pe->mouse_wheel_delta);
               break;
            }
         }

         switch (i_pe->type)
         {
         case mws_ptr_evt::touch_invalid:
            break;

         case mws_ptr_evt::touch_began:
         {
            if (i_pe->touch_count == 1)
            {
               prev_ptr_evt = nullptr;
            }

            is_pointer_down = true;
            broadcast(get_instance(), i_pe);
            prev_ptr_evt = i_pe;
            break;
         }

         case mws_ptr_evt::touch_moved:
         {
            if (is_pointer_down)
            {
               // calculate 'is_changed' for pointer move events
               if (prev_ptr_evt)
               {
                  for (uint32 k = 0; k < i_pe->touch_count; k++)
                  {
                     mws_ptr_evt::touch_point& pt = i_pe->points[k];

                     if (!pt.is_changed)
                     {
                        const mws_ptr_evt::touch_point* same_id = prev_ptr_evt->find_point(pt.identifier);

                        if (same_id)
                        {
                           if ((pt.x != same_id->x) || (pt.y != same_id->y))
                           {
                              pt.is_changed = true;
                           }
                        }
                     }
                  }
               }

               broadcast(get_instance(), i_pe);
               prev_ptr_evt = i_pe;
            }

            break;
         }

         case mws_ptr_evt::touch_ended:
         {
            is_pointer_down = false;
            broadcast(get_instance(), i_pe);
            prev_ptr_evt = i_pe;
            break;
         }

         case mws_ptr_evt::touch_cancelled:
         {
            broadcast(get_instance(), i_pe);
            prev_ptr_evt = i_pe;
            break;
         }

         case mws_ptr_evt::mouse_wheel:
         {
            broadcast(get_instance(), i_pe);
            break;
         }
         }
      }
   }
}

void mws_touch_ctrl::enqueue_pointer_event(mws_sp<mws_ptr_evt_base> i_te)
{
   mws_sp<mws_ptr_evt> te = std::static_pointer_cast<mws_ptr_evt>(i_te);
   (*queue_ptr).push_back(te);
}

mws_sp<mws_sender> mws_touch_ctrl::sender_inst()
{
   return get_instance();
}


const std::string mws_key_evt::key_evt_type = "ke-";
const std::string mws_key_evt::key_evt_pressed = "ke-pressed";
const std::string mws_key_evt::key_evt_repeated = "ke-repeated";
const std::string mws_key_evt::key_evt_released = "ke-released";


mws_key_evt::mws_key_evt(mws_wp<mws_key_ctrl> i_src, mws_key_evt::key_evt_types i_type, mws_key_types i_key) : mws_dp(get_type_name(i_type))
{
   src = i_src;
   type = i_type;
   key = i_key;
}

mws_sp<mws_key_evt> mws_key_evt::as_key_evt(mws_sp<mws_dp> i_dp)
{
   return static_pointer_cast<mws_key_evt>(i_dp);
}

mws_sp<mws_key_evt> mws_key_evt::nwi(mws_wp<mws_key_ctrl> i_src, mws_key_evt::key_evt_types i_type, mws_key_types i_key)
{
   return mws_sp<mws_key_evt>(new mws_key_evt(i_src, i_type, i_key));
}

mws_sp<mws_key_evt> mws_key_evt::get_instance()
{
   return shared_from_this();
}

bool mws_key_evt::is_ascii(int i_key_id)
{
   return i_key_id >= mws_key_space && i_key_id <= mws_key_tilde_sign;
}

const std::string& mws_key_evt::get_type_name(key_evt_types i_key_evt)
{
   static const std::string types[] =
   {
      key_evt_pressed,
      key_evt_repeated,
      key_evt_released,
   };

   return types[i_key_evt];
}

mws_sp<mws_key_ctrl> mws_key_evt::get_src()
{
   return src.lock();
}

bool mws_key_evt::is_letter() const
{
   return (key >= mws_key_a_upper_case && key <= mws_key_z_upper_case) || (key >= mws_key_a && key <= mws_key_z);
}

bool mws_key_evt::is_pressed() const
{
   return type == ke_pressed;
}

bool mws_key_evt::is_repeated() const
{
   return type == ke_repeated;
}

bool mws_key_evt::is_released() const
{
   return type == ke_released;
}

mws_key_evt::key_evt_types mws_key_evt::get_type() const
{
   return type;
}

mws_key_types mws_key_evt::get_key() const
{
   return key;
}

void mws_key_evt::process(mws_sp<mws_receiver> i_dst)
{
   mws_dp::process(i_dst);
}


enum mws_key_status
{
   mws_key_idle,
   mws_key_pressed,
   mws_key_first_pressed,
   mws_key_repeated,
   mws_key_released,
   mws_key_released_idle,
};


// there are at most mws_key_count different keys that we're interested in handling
static struct { uint32 time; mws_key_status status; } key_list[mws_key_count];
uint32 mws_key_ctrl::time_until_first_key_repeat_ms = 600;
uint32 mws_key_ctrl::key_repeat_threshold_ms = 150;
const uint32 infinite_key_repeats = 0xffffffff;
uint32 mws_key_ctrl::max_key_repeat_count = infinite_key_repeats;

mws_key_ctrl::mws_key_ctrl()
{
   clear_keys();
}

mws_sp<mws_key_ctrl> mws_key_ctrl::nwi()
{
   return mws_sp<mws_key_ctrl>(new mws_key_ctrl());
}

mws_sp<mws_key_ctrl> mws_key_ctrl::get_instance()
{
   return shared_from_this();
}

void mws_key_ctrl::update()
{
   if (events_pending)
   {
      auto inst = get_instance();
      uint32 crt_time = mws::time::get_time_millis();
      bool events_still_pending = false;
      auto key_released = [&](int i_idx, mws_key_types i_key_id)
      {
         mws_key_types key_id = mws::input::apply_key_modifiers(i_key_id);

         key_list[i_idx].status = mws_key_released_idle;
         new_key_event(mws_key_evt::nwi(inst, mws_key_evt::ke_released, key_id));
         events_still_pending = true;
      };

      for (int k = mws_key_invalid + 1; k < (int)mws_key_count; k++)
      {
         mws_key_types key_id_no_mods = (mws_key_types)k;
         auto& kp = key_list[k];

         switch (kp.status)
         {
         case mws_key_pressed:
         {
            mws_key_types key_id = mws::input::apply_key_modifiers(key_id_no_mods);

            new_key_event(mws_key_evt::nwi(inst, mws_key_evt::ke_pressed, key_id));
            kp.status = mws_key_first_pressed;
            events_still_pending = true;
            break;
         }

         case mws_key_first_pressed:
            if (crt_time - kp.time > time_until_first_key_repeat_ms)
            {
               mws_key_types key_id = mws::input::apply_key_modifiers(key_id_no_mods);

               new_key_event(mws_key_evt::nwi(inst, mws_key_evt::ke_repeated, key_id));
               kp.status = mws_key_repeated;
               kp.time = crt_time;
            }

            events_still_pending = true;
            break;

         case mws_key_repeated:
         {
            uint32 dt = crt_time - kp.time;

            if (dt > key_repeat_threshold_ms)
            {
               const uint64 max_repeat_threshold_ms = (uint64)key_repeat_threshold_ms * max_key_repeat_count;
               mws_key_types key_id = mws::input::apply_key_modifiers(key_id_no_mods);

               // if the key repeat is past the max number of repeats for this key, force release it
               if (max_key_repeat_count != infinite_key_repeats && dt > max_repeat_threshold_ms)
               {
                  key_released(k, key_id);
               }
               else
               {
                  new_key_event(mws_key_evt::nwi(inst, mws_key_evt::ke_repeated, key_id));
                  events_still_pending = true;
               }

               kp.time = crt_time;
            }
            break;
         }

         case mws_key_released:
         {
            mws_key_types key_id = mws::input::apply_key_modifiers(key_id_no_mods);

            key_released(k, key_id);
            break;
         }

         case mws_key_released_idle:
            kp.status = mws_key_idle;
            break;
         }
      }

      events_pending = events_still_pending;
   }
}

bool mws_key_ctrl::key_is_held(mws_key_types i_key)
{
   mws_assert(i_key >= mws_key_invalid && i_key < mws_key_count);

   return key_list[i_key].status != mws_key_idle;
}

void mws_key_ctrl::key_pressed(mws_key_types i_key)
{
   mws_assert(i_key >= mws_key_invalid && i_key < mws_key_count);

   if (i_key > mws_key_invalid)
   {
      auto& kp = key_list[i_key];
      events_pending = true;

      if (kp.status != mws_key_first_pressed && kp.status != mws_key_repeated)
      {
         kp = { mws::time::get_time_millis(), mws_key_pressed };
      }
   }
}

void mws_key_ctrl::key_released(mws_key_types i_key)
{
   mws_assert(i_key >= mws_key_invalid && i_key < mws_key_count);

   if (i_key > mws_key_invalid)
   {
      events_pending = true;
      key_list[i_key].status = mws_key_released;
   }
}

void mws_key_ctrl::clear_keys()
{
   int size = sizeof(key_list);
   memset(key_list, 0, size);
   events_pending = false;
}

mws_sp<mws_sender> mws_key_ctrl::sender_inst()
{
   return get_instance();
}

void mws_key_ctrl::new_key_event(mws_sp<mws_key_evt> i_ke)
{
   broadcast(i_ke->get_src(), i_ke);
}
