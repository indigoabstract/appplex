#include "stdafx.h"

#include "input-ctrl.hpp"
#include "pfm-def.h"
#include "unit.hpp"
#include <glm/inc.hpp>


const std::string pointer_evt::TOUCHSYM_EVT_TYPE = "ts-";


pointer_evt::pointer_evt() : iadp("ts-")
{
   type = touch_invalid;
   time = 0;
   touch_count = 0;
   mouse_wheel_delta = 0;
}

void pointer_evt::process()
{
   iadp::process();

   //if (type == touch_ended)
   //{
   //   int x = 3;
   //}
   //mws_print("%s\n", get_name().c_str());
}

mws_sp<pointer_evt> pointer_evt::as_pointer_evt(shared_ptr<iadp> idp)
{
   return static_pointer_cast<pointer_evt>(idp);
}

bool pointer_evt::is_multitouch()
{
   return touch_count > 1;
}

// pointer_down_count
// params:
// return:
//    return number of fingers currently pressed when this event was generated, and plus one if mouse button is down
int pointer_evt::pointer_down_count()
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
const pointer_evt::touch_point* pointer_evt::get_pointer_press_by_index(uint32 PointerIndex)
{
   if (touch_count > PointerIndex)
   {
      return &points[PointerIndex];
   }
   //else if (PointerIndex == 0 && mMousePressed)
   //{
   //   return mMousePressed.get();
   //}

   return nullptr;
}

const pointer_evt::touch_point* pointer_evt::find_point(uintptr_t touch_id) const
{
   for (uint32 i = 0; i < touch_count; i++)
   {
      if (points[i].identifier == touch_id)
      {
         return &points[i];
      }
   }

   return nullptr;
}

bool pointer_evt::same_touches(const pointer_evt& other) const
{
   if (other.touch_count == this->touch_count)
   {
      for (uint32 i = 0; i < this->touch_count; i++)
      {
         if (nullptr == this->find_point(other.points[i].identifier))
         {
            return false;
         }
      }

      return true;
   }

   return false;
}

glm::vec2 pointer_evt::touch_pos(uintptr_t touch_id) const
{
   const touch_point* p = this->find_point(touch_id);
   mws_assert(p);

   return glm::vec2(p->x, p->y);
}

std::string pointer_evt::get_evt_type()
{
   switch (type)
   {
   case touch_invalid:
      return "touch_invalid";
   case touch_began:
      return "touch_began";
   case touch_moved:
      return "touch_moved";
   case touch_ended:
      return "touch_ended";
   case touch_cancelled:
      return "touch_cancelled";
   case mouse_wheel:
      return "mouse_wheel";
   }

   return "n/a";
}


bool touchctrl::is_pointer_down = false;

touchctrl::touchctrl()
{
   queue_tab.resize(2);
   queue_idx = 0;
   queue_ptr = &queue_tab[queue_idx];
}

shared_ptr<touchctrl> touchctrl::nwi()
{
   return shared_ptr<touchctrl>(new touchctrl());
}

shared_ptr<touchctrl> touchctrl::get_instance()
{
   return shared_from_this();
}

bool touchctrl::is_pointer_released()
{
   return !is_pointer_down;
}

void touchctrl::update()
{
   // set the current input queue as the queue for processing the input
   std::vector<std::shared_ptr<pointer_evt> >* input_queue_ptr = &queue_tab[queue_idx];

   // switch queues, so the currently empty queue is used for taking input events
   queue_idx = (queue_idx + 1) % 2;
   queue_tab[queue_idx].clear();
   queue_ptr = &queue_tab[queue_idx];

   if (!input_queue_ptr->empty())
   {
      for (auto pa : *input_queue_ptr)
      {
         switch (pa->type)
         {
         case pointer_evt::touch_began:
            on_pointer_action_pressed(pa);
            break;

         case pointer_evt::touch_ended:
            on_pointer_action_released(pa);
            break;

         case pointer_evt::touch_moved:
            on_pointer_action_dragged(pa);
            break;

         case pointer_evt::mouse_wheel:
            on_pointer_action_mouse_wheel(pa);
            break;
         }
      }
   }
}

void touchctrl::enqueue_pointer_event(std::shared_ptr<pointer_evt> ite)
{
   (*queue_ptr).push_back(ite);
}

shared_ptr<ia_sender> touchctrl::sender_inst()
{
   return get_instance();
}

void touchctrl::on_pointer_action_pressed(std::shared_ptr<pointer_evt> pa)
{
   is_pointer_down = true;
   broadcast(get_instance(), pa);
}

void touchctrl::on_pointer_action_dragged(std::shared_ptr<pointer_evt> pa)
{
   if (is_pointer_down)
   {
      broadcast(get_instance(), pa);
   }
}

void touchctrl::on_pointer_action_released(std::shared_ptr<pointer_evt> pa)
{
   is_pointer_down = false;
   broadcast(get_instance(), pa);
}

void touchctrl::on_pointer_action_mouse_wheel(std::shared_ptr<pointer_evt> pa)
{
   broadcast(get_instance(), pa);
}


const std::string key_evt::KEYEVT_EVT_TYPE = "ke-";
const std::string key_evt::KEYEVT_PRESSED = "ke-pressed";
const std::string key_evt::KEYEVT_REPEATED = "ke-repeated";
const std::string key_evt::KEYEVT_RELEASED = "ke-released";


key_evt::key_evt(std::weak_ptr<key_ctrl> isrc, key_evt::key_evt_types itype, key_types i_key) : iadp(get_type_name(itype))
{
   src = isrc;
   type = itype;
   key = i_key;
   //trx("newkeyevt %x") % this;
}

shared_ptr<key_evt> key_evt::as_key_evt(shared_ptr<iadp> idp)
{
   return static_pointer_cast<key_evt>(idp);
}

shared_ptr<key_evt> key_evt::nwi(std::weak_ptr<key_ctrl> isrc, key_evt::key_evt_types itype, key_types i_key)
{
   return shared_ptr<key_evt>(new key_evt(isrc, itype, i_key));
}

shared_ptr<key_evt> key_evt::get_instance()
{
   return shared_from_this();
}

const std::string& key_evt::get_type_name(key_evt_types tstype)
{
   static const std::string types[] =
   {
      KEYEVT_PRESSED,
      KEYEVT_REPEATED,
      KEYEVT_RELEASED,
   };

   return types[tstype];
}

std::shared_ptr<key_ctrl> key_evt::get_src()
{
   return src.lock();
}

bool key_evt::is_pressed() const
{
   return type == KE_PRESSED;
}

bool key_evt::is_repeated() const
{
   return type == KE_REPEATED;
}

bool key_evt::is_released() const
{
   return type == KE_RELEASED;
}

key_evt::key_evt_types key_evt::get_type() const
{
   return type;
}

key_types key_evt::get_key() const
{
   return key;
}

void key_evt::process()
{
   //trx("keyevt process %x") % getInst().get();
   iadp::process();
}


enum key_status
{
   KEY_IDLE,
   KEY_PRESSED,
   KEY_FIRST_PRESSED,
   KEY_REPEATED,
   KEY_RELEASED,
   KEY_RELEASED_IDLE,
};


uint8 key_ctrl::keys_status[KEY_COUNT] = { KEY_IDLE };
uint32 key_ctrl::keys_status_time[KEY_COUNT] = { 0 };

key_ctrl::key_ctrl()
{
   events_pending = false;
}

shared_ptr<key_ctrl> key_ctrl::nwi()
{
   return shared_ptr<key_ctrl>(new key_ctrl());
}

shared_ptr<key_ctrl> key_ctrl::get_instance()
{
   return shared_from_this();
}

void key_ctrl::update()
{
   if (events_pending)
   {
      auto inst = get_instance();
      uint32 crt_time = pfm::time::get_time_millis();
      bool events_still_pending = false;

      for (int k = KEY_INVALID; k < KEY_COUNT; k++)
      {
         switch (keys_status[k])
         {
         case KEY_PRESSED:
            new_key_event(key_evt::nwi(inst, key_evt::KE_PRESSED, (key_types)k));
            keys_status[k] = KEY_FIRST_PRESSED;
            events_still_pending = true;
            break;

         case KEY_FIRST_PRESSED:
            if (crt_time - keys_status_time[k] > 400)
            {
               new_key_event(key_evt::nwi(inst, key_evt::KE_REPEATED, (key_types)k));
               keys_status[k] = KEY_REPEATED;
               keys_status_time[k] = crt_time;
            }

            events_still_pending = true;
            break;

         case KEY_REPEATED:
            if (crt_time - keys_status_time[k] > 25)
            {
               new_key_event(key_evt::nwi(inst, key_evt::KE_REPEATED, (key_types)k));
               keys_status_time[k] = crt_time;
            }

            events_still_pending = true;
            break;

         case KEY_RELEASED:
            keys_status[k] = KEY_RELEASED_IDLE;
            new_key_event(key_evt::nwi(inst, key_evt::KE_RELEASED, (key_types)k));
            events_still_pending = true;
            break;

         case KEY_RELEASED_IDLE:
            keys_status[k] = KEY_IDLE;
            break;
         }
      }

      events_pending = events_still_pending;
   }
}

bool key_ctrl::key_is_held(key_types i_key)
{
   mws_assert(i_key >= KEY_INVALID && i_key < KEY_COUNT);

   return keys_status[i_key] != KEY_IDLE;
}

void key_ctrl::key_pressed(int i_key)
{
   mws_assert(i_key >= KEY_INVALID && i_key < KEY_COUNT);
   events_pending = true;

   if (keys_status[i_key] != KEY_FIRST_PRESSED && keys_status[i_key] != KEY_REPEATED)
   {
      keys_status[i_key] = KEY_PRESSED;
      keys_status_time[i_key] = pfm::time::get_time_millis();
   }
}

void key_ctrl::key_released(int i_key)
{
   mws_assert(i_key >= KEY_INVALID && i_key < KEY_COUNT);
   events_pending = true;
   keys_status[i_key] = KEY_RELEASED;
}

shared_ptr<ia_sender> key_ctrl::sender_inst()
{
   return get_instance();
}

void key_ctrl::new_key_event(shared_ptr<key_evt> ke)
{
   //trx("keyevt type %1%) ke->getName();
   broadcast(ke->get_src(), ke);
}
