package com.indigoabstract.appplex;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import androidx.core.app.NotificationCompat;


public class AlarmReceiver extends BroadcastReceiver
{
    private static final String CHANNEL_ID = "channelId";
    private PendingIntent getFullScreenIntent(Context context)
    {
        Intent intent = new Intent(context, main.class);

    // flags and request code are 0 for the purpose of demonstration
        return PendingIntent.getActivity(context, 0, intent, 0);
    }
    // This method will be called when the alarm fires.
    @Override
    public void onReceive(Context context, Intent intent)
    {
        if (intent.getAction() != "mws-alarm")
        {
            return;
        }

        Notification notification = new NotificationCompat.Builder(context, CHANNEL_ID)
                .setSmallIcon(android.R.drawable.arrow_up_float)
                .setContentTitle("setContentTitle")
                .setContentText("setContentText")
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .setAutoCancel(true)
                .setCategory(NotificationCompat.CATEGORY_ALARM)
                //.setTimeoutAfter(5 * 1000)
                .setFullScreenIntent(getFullScreenIntent(context), true)
                .addAction(new NotificationCompat.Action.Builder(R.drawable.icon, "Dismiss", null).build())
                .addAction(new NotificationCompat.Action.Builder(R.drawable.icon, "Snooze", null).build())
                .build();
        NotificationManager notificationManager =
                (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            String name = "Example Notification Channel";
            String descriptionText = "This is used to demonstrate the Full Screen Intent";
            int importance = NotificationManager.IMPORTANCE_HIGH;
            NotificationChannel channel = new NotificationChannel(CHANNEL_ID, name, importance);

            channel.setDescription(descriptionText);
            notificationManager.createNotificationChannel(channel);
        }

        notificationManager.notify(0, notification);

        /*
        Log.i("com.indigoabstract", "AlarmReceiver.onReceive");
        // Extract the message and notification id from the original intent.
        android.os.Bundle extras = intent.getExtras();
        String alarm_type = extras.getString("type");
        String message = extras.getString("message");
        int tagId = extras.getInt("tagId");

        //Log.i("com.indigoabstract", "AlarmReceiver:onReceive. alarm_type " + alarm_type + " message " + message + " tagId " + tagId);
        Log.i("com.indigoabstract", "started main activity 0");
        if(alarm_type.equals("wakeup"))
        {
            Log.i("com.indigoabstract", "AlarmReceiver.onReceive wakeup");
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q)
            {
                Log.i("com.indigoabstract", "AlarmReceiver.onReceive Build.VERSION.SDK_INT >= Build.VERSION_CODES.O");
                Intent service1 = new Intent(context, AlarmService.class);

                // And pass them to the newly created intent.
                service1.putExtra("message", message);
                service1.putExtra("tagId", tagId);
                // The notification will fire in the application background.
                context.startService(service1);
            }
            else
            {
                Log.i("com.indigoabstract", "AlarmReceiver.onReceive Build.VERSION.SDK_INT < Build.VERSION_CODES.O");
//                Intent in = new Intent(context, main.class);
//                in.putExtra("wakeup", true);
//                in.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);// | Intent.FLAG_ACTIVITY_CLEAR_TOP);
//                context.startActivity(in);
            }
            Log.i("com.indigoabstract", "started main activity 1");
        }
        else if(alarm_type.equals("notification"))
        {
            Log.i("com.indigoabstract", "AlarmReceiver.onReceive notification msg " + message + " id " + tagId);

//            Intent service1 = new Intent(context, AlarmService.class);
//
//            // And pass them to the newly created intent.
//            service1.putExtra("message", message);
//            service1.putExtra("tagId", tagId);
//            // The notification will fire in the application background.
//            context.startService(service1);
        }
        */
    }
}
