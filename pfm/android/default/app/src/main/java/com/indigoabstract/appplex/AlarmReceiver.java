package com.indigoabstract.appplex;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class AlarmReceiver extends BroadcastReceiver {

    private static final String TAG = "alarm-receiver";

    // This method will be called when the alarm fires.
    @Override
    public void onReceive(Context context, Intent intent) {
        // Extract the message and notification id from the original intent.
        android.os.Bundle extras = intent.getExtras();
        String alarm_type = extras.getString("type");
        String message = extras.getString("message");
        int tagId = extras.getInt("tagId");

        //Log.i("omnirecall", "AlarmReceiver:onReceive. alarm_type " + alarm_type + " message " + message + " tagId " + tagId);
        if(alarm_type.equals("wakeup"))
        {
            Log.i("omnirecall", "started main activity 0");
            Intent in = new Intent(context, main.class);
            in.putExtra("wakeup", true);
            in.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);// | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(in);
            Log.i("omnirecall", "started main activity 1");
        }
        else if(alarm_type.equals("notification"))
        {
            //Log.i(TAG,"BroadcastReceiver has received alarm intent msg " + message + " id " + tagId);

            Intent service1 = new Intent(context, AlarmService.class);

            // And pass them to the newly created intent.
            service1.putExtra("message", message);
            service1.putExtra("tagId", tagId);
            // The notification will fire in the application background.
            context.startService(service1);
        }
    }
}
