#include "stdafx.h"

#include "notifications/local-notification.hpp"
#include <algorithm>

void local_notification::register_for_notifications()
{
}

void local_notification::schedule(std::string message, int delay_in_seconds, int tag)
{
    if(std::find(active_notif_tag_list.begin(), active_notif_tag_list.end(), tag) != active_notif_tag_list.end())
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
