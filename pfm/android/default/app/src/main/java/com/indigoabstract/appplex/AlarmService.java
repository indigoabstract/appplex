package com.indigoabstract.appplex;


import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.res.Resources;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.app.NotificationChannel;
import androidx.core.app.NotificationCompat;
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
    public int onStartCommand(Intent intent, int flags, int startId)
    {
        return super.onStartCommand(intent,flags,startId);
    }

    // This will be started after the alarm is received.
    @Override
    protected void onHandleIntent(Intent intent)
    {
        /*
        Log.i("com.indigoabstract", "AlarmService.onHandleIntent");
        // Extract the notification message and id.
        android.os.Bundle extras = intent.getExtras();
        String message = extras.getString("message");
        int tagId = extras.getInt("tagId");
        String channel_id = "default";

        //Log.i(TAG,"Alarm Service has started msg " + message + " id " + tagId);
        Context context = getApplicationContext();
        notificationManager = (NotificationManager)context.getSystemService(Context.NOTIFICATION_SERVICE);

        // create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            Log.i("com.indigoabstract", "AlarmService.onHandleIntent Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 0");
            CharSequence name = "default";
            String description = "default";
            int importance = NotificationManager.IMPORTANCE_HIGH;
            // register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationChannel channel = new NotificationChannel(channel_id, name, importance);
            channel.setDescription(description);
            notificationManager.createNotificationChannel(channel);
        }

        Intent mIntent = null;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            Log.i("com.indigoabstract", "AlarmService.onHandleIntent Build.VERSION.SDK_INT >= Build.VERSION_CODES.O 1");
            mIntent = new Intent(this, main.class);
            mIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            pendingIntent = PendingIntent.getActivity(context, tagId, mIntent, PendingIntent.FLAG_ONE_SHOT);
        }
        else
        {
            Log.i("com.indigoabstract", "AlarmService.onHandleIntent Build.VERSION.SDK_INT < Build.VERSION_CODES.O 2");
//            mIntent = new Intent(this, main.class);
//            pendingIntent = PendingIntent.getActivity(context, tagId, mIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        }

        Resources res = getResources();
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, channel_id);

        // Build the notification (using default preferences).
        builder.setSmallIcon(R.drawable.icon)
                .setLargeIcon(BitmapFactory.decodeResource(res, R.drawable.icon))
                .setTicker("break-0")
                .setAutoCancel(true)
                .setContentTitle("break-1")
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .setCategory(NotificationCompat.CATEGORY_ALARM)
                //.setSound(Uri.parse("android.resource://" + getPackageName() + "/" + R.raw.notification))
                .setDefaults(Notification.DEFAULT_LIGHTS)
                .setContentIntent(pendingIntent)
                .setFullScreenIntent(pendingIntent, true);

        // Fire it.
        //notificationManager = (NotificationManager)getSystemService(NOTIFICATION_SERVICE);
        notificationManager.notify(tagId, builder.build());
        Log.i("com.indigoabstract", "AlarmService.onHandleIntent finished");
        */
    }
}
