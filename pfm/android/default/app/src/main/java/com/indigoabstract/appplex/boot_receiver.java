package com.indigoabstract.appplex;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class boot_receiver extends BroadcastReceiver
{
    @Override
    public void onReceive(Context context, Intent intent)
    {
        //WakefulIntentService.acquireStaticLock(context); //acquire a partial WakeLock
        //context.startService(new Intent(context, TaskButlerService.class)); //start TaskButlerService
    }
}
