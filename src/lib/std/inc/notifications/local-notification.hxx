#pragma once

#include <chrono>
#include <string>
#include <vector>


class local_notification
{
public:
    /**
     * Registers the types of notifications the app is using
     */
    static void register_for_notifications();

    /**
    * Schedules a local notification
    * @param message - message to be shown in notification
    * @param delay_in_seconds - the notification will be fired after this number of seconds have elapsed (since 'schedule_by_delay()' is called).
    * @param tag - tag to specify notification
    */
    static void schedule_wakeup_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag);

    /**
    * Schedules a local notification
    * @param message - message to be shown in notification
    * @param delay_in_seconds - the notification will be fired after this number of seconds have elapsed (since 'schedule_by_delay()' is called).
    * @param tag - tag to specify notification
    */
    static void schedule_wakeup_by_delay(std::string message, int delay_in_seconds, int tag);

    /**
    * Schedules a local notification
    * @param message - message to be shown in notification
    * @param time_point_date - the notification will be fired when system clock reaches the specified date.
    * @param tag - tag to specify notification
    */
    static void schedule_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag);

    /**
    * Schedules a local notification
    * @param message - message to be shown in notification
    * @param delay_in_seconds - the notification will be fired after this number of seconds have elapsed (since 'schedule_by_delay()' is called).
    * @param tag - tag to specify notification
    */
    static void schedule_by_delay(std::string message, int delay_in_seconds, int tag);
    
    /**
     * Cancel local notification specified by ID
     * @param tag Tag of local notification
     */
    static void cancel(int tag);
    
    /**
     * Cancel all scheduled local notifications in the set interval
     */
    static void cancel_interval(int i_start_tag, int i_stop_tag);
};
