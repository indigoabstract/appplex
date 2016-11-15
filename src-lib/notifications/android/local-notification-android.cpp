#include "stdafx.h"

#include "notifications/local-notification.hpp"
#include "jni-helper.hpp"
#include <jni.h>
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

    JNIEnv* env = JniHelper::getEnv();
    jstring jstr1 = env->NewStringUTF(message.c_str());
    jclass clazz = env->FindClass("com/indigoabstract/appplex/main");
    jmethodID mid = env->GetStaticMethodID(clazz, "schedule_notification", "(Ljava/lang/String;II)V");

    env->CallStaticVoidMethod(clazz, mid, jstr1, delay_in_seconds, tag);

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
        JNIEnv* env = JniHelper::getEnv();
        jclass clazz = env->FindClass("com/indigoabstract/appplex/main");
        jmethodID mid = env->GetStaticMethodID(clazz, "cancel_notification", "(I)V");

        env->CallStaticVoidMethod(clazz, mid, tag);
        active_notif_tag_list.erase(idx);
    }
}

void local_notification::cancell_all()
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass("com/indigoabstract/appplex/main");
    jmethodID mid = env->GetStaticMethodID(clazz, "cancel_all_notifications", "(II)V");

    env->CallStaticVoidMethod(clazz, mid, e8hNotifTag, eNotificationCount);

    // clear all active notifications.
    active_notif_tag_list.clear();
}

std::vector<int> local_notification::active_notif_tag_list;

std::vector<int> local_notification::notification_list =
{
        0,
        1,
        2,
        3,
        4,
        5,
};
