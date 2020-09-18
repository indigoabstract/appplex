#include "stdafx.hxx"

#include "notifications/local-notification.hxx"
#include "pfm-def.h"
#include <algorithm>


void local_notification::schedule_wakeup_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag)
{
   std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
   std::chrono::seconds delay = std::chrono::duration_cast<std::chrono::seconds>(time_point_date - now);
   int delay_in_seconds = (int)delay.count();

   schedule_wakeup_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_wakeup_by_delay(std::string message, int delay_in_seconds, int tag)
{
   mws_print("local_notification::schedule_wakeup_by_delay not implemented\n");
}

void local_notification::schedule_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag)
{
   std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
   std::chrono::seconds delay = std::chrono::duration_cast<std::chrono::seconds>(time_point_date - now);
   int delay_in_seconds = (int)delay.count();

   schedule_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_by_delay(std::string message, int delay_in_seconds, int tag)
{
   mws_print("local_notification::schedule_by_delay not implemented\n");
}

void local_notification::cancel(int tag)
{
   mws_print("local_notification::cancel not implemented\n");
}

void local_notification::cancel_interval(int i_start_tag, int i_stop_tag)
{
   mws_print("local_notification::cancel_interval not implemented\n");
}
