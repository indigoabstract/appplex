#include "stdafx.h"

#include "notifications/local-notification.hpp"
#include <algorithm>

void local_notification::register_for_notifications()
{
}

/**
* Schedules a local notification
* @param message - message to be shown in notification
* @param time_point_date - the notification will be fired when system clock reaches the specified date.
* @param tag - tag to specify notification
*/
void local_notification::schedule_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag)
{
   std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
   std::chrono::seconds delay = std::chrono::duration_cast<std::chrono::seconds>(time_point_date - now);
   int delay_in_seconds = delay.count();

   schedule_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_by_delay(std::string message, int delay_in_seconds, int tag)
{
   if (std::find(active_notif_tag_list.begin(), active_notif_tag_list.end(), tag) != active_notif_tag_list.end())
   {
      // don't schedule any notifications using this tag if they are not cancelled first.
      return;
   }

   // the notification is scheduled now, so add the tag to the list.
   active_notif_tag_list.push_back(tag);
}

bool local_notification::is_active(int tag)
{
    return std::find(active_notif_tag_list.begin(), active_notif_tag_list.end(), tag) != active_notif_tag_list.end();
}

void local_notification::cancel(int tag)
{
    auto idx = std::find(active_notif_tag_list.begin(), active_notif_tag_list.end(), tag);

    // remove tag from the active notifications list.
    if(idx != active_notif_tag_list.end())
    {
        active_notif_tag_list.erase(idx);
    }
}

void local_notification::cancell_all()
{
    // clear all active notifications.
    active_notif_tag_list.clear();
}

std::vector<int> local_notification::active_notif_tag_list;
