#include "stdafx.h"

#include "notifications/local-notification.hpp"
#include "main.hpp"
#include "jni-helper.hpp"
#include <jni.h>
#include <algorithm>

void local_notification::register_for_notifications()
{
}

void local_notification::schedule_wakeup_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::seconds delay = std::chrono::duration_cast<std::chrono::seconds>(time_point_date - now);
    int delay_in_seconds = delay.count();

    schedule_wakeup_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_wakeup_by_delay(std::string message, int delay_in_seconds, int tag)
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "schedule_wakeup", "(Ljava/lang/String;II)V");
    jstring jstr1 = env->NewStringUTF(message.c_str());

    env->CallStaticVoidMethod(clazz, mid, jstr1, delay_in_seconds, tag);
}

void local_notification::schedule_by_date(std::string message, const std::chrono::system_clock::time_point& time_point_date, int tag)
{
    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::chrono::seconds delay = std::chrono::duration_cast<std::chrono::seconds>(time_point_date - now);
    int delay_in_seconds = delay.count();

    schedule_by_delay(message, delay_in_seconds, tag);
}

void local_notification::schedule_by_delay(std::string message, int delay_in_seconds, int tag)
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "schedule_notification", "(Ljava/lang/String;II)V");
    jstring jstr1 = env->NewStringUTF(message.c_str());

    env->CallStaticVoidMethod(clazz, mid, jstr1, delay_in_seconds, tag);
}

void local_notification::cancel(int tag)
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "cancel_notification", "(I)V");
    env->CallStaticVoidMethod(clazz, mid, tag);
}

void local_notification::cancel_interval(int i_start_tag, int i_stop_tag)
{
    JNIEnv* env = JniHelper::getEnv();
    jclass clazz = env->FindClass(CLASS_MAIN_PATH);
    jmethodID mid = env->GetStaticMethodID(clazz, "cancel_notification_interval", "(II)V");
    env->CallStaticVoidMethod(clazz, mid, i_start_tag, i_stop_tag);
}
