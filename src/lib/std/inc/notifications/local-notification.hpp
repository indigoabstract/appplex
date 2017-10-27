#pragma once

#include <chrono>
#include <string>
#include <vector>


class local_notification
{
public:
    enum e_notification_tags
    {
        e15sNotifTag = 0, // for testing
        e8hNotifTag,
        e24hNotifTag,
        e48hNotifTag,
        eNotificationCount,
    };
    
    /**
     * Registers the types of notifications the app is using
     */
    static void register_for_notifications();
    
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
     * Check if notification specified by ID is active
     * @param tag Tag of local notification
     */
    static bool is_active(int tag);
    
    /**
     * Cancel local notification specified by ID
     * @param tag Tag of local notification
     */
    static void cancel(int tag);
    
    /**
     * Cancel all scheduled local notifications
     */
    static void cancell_all();
    
    // a list with the tags of the currently scheduled(active) notifications
    static std::vector<int> active_notif_tag_list;
    // a list with the string ids of possible messages in a local notification.
    static std::vector<int> notification_list;
};
