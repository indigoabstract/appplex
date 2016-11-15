#pragma once

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
     * Schedules local notification
     * @param message  Message should be shown in notification
     * @param delayInSeconds The notification will be fired after this number of seconds have elapsed (since 'schedule()' is called).
     * @param tag      Tag to specify notification
     */
    static void schedule(std::string message, int delay_in_seconds, int tag);
    
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
