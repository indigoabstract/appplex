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
   int delay_in_seconds = (int)delay.count();

   schedule_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_by_delay(std::string message, int delay_in_seconds, int tag)
{
}

void local_notification::cancel(int tag)
{
}

void local_notification::cancel_interval(int i_start_tag, int i_stop_tag)
{
}
