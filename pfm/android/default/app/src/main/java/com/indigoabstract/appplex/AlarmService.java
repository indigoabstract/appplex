package com.indigoabstract.appplex;


import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.support.v4.app.NotificationCompat;
import android.util.Log;

public class AlarmService extends IntentService
{
    private static final String TAG = "alarm-service";
    private NotificationManager notificationManager;
    private PendingIntent pendingIntent;


    public AlarmService() {
        super("AlarmService");
    }


    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent,flags,startId);
    }

    // This will be started after the alarm is received.
    @Override
    protected void onHandleIntent(Intent intent) {
        // Extract the notification message and id.
        android.os.Bundle extras = intent.getExtras();
        String message = extras.getString("message");
        int tagId = extras.getInt("tagId");

        //Log.i(TAG,"Alarm Service has started msg " + message + " id " + tagId);
        Context context = this.getApplicationContext();
        notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);
        Intent mIntent = new Intent(this, main.class);
        pendingIntent = PendingIntent.getActivity(context, tagId, mIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        Resources res = this.getResources();
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this);

        // Build the notification (using default preferences).
        builder.setContentIntent(pendingIntent)
                .setSmallIcon(R.drawable.icon)
                .setLargeIcon(BitmapFactory.decodeResource(res, R.drawable.icon))
                .setTicker("haiku1break")
                .setAutoCancel(true)
                .setContentTitle("haiku2break")
                .setContentText(message)
                .setSound(Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.notification))
                .setDefaults(Notification.DEFAULT_LIGHTS);

        // Fire it.
        notificationManager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        notificationManager.notify(tagId, builder.build());
    }
}
