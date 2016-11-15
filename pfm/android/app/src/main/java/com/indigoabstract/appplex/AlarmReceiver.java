package com.indigoabstract.appplex;


import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AlarmReceiver extends BroadcastReceiver {

    private static final String TAG = "alarm-receiver";
    Intent intent;
    PendingIntent pendingIntent;
    NotificationManager notificationManager;

    // This method will be called when the alarm fires.
    @Override
    public void onReceive(Context context, Intent intent) {
        // Extract the message and notification id from the original intent.
        android.os.Bundle extras = intent.getExtras();
        String message = extras.getString("message");
        int tagId = extras.getInt("tagId");

        //Log.i(TAG,"BroadcastReceiver has received alarm intent msg " + message + " id " + tagId);

        Intent service1 = new Intent(context, AlarmService.class);

        // And pass them to the newly created intent.
        service1.putExtra("message", message);
        service1.putExtra("tagId", tagId);
        // The notification will fire in the application background.
        context.startService(service1);
    }
}
