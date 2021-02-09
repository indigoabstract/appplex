package com.indigoabstract.appplex;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.KeyguardManager;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.AssetManager;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.RelativeLayout;

import java.io.File;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.TimeZone;
import java.util.regex.Pattern;

import androidx.appcompat.app.AppCompatActivity;


public class main extends Activity
{
	static
	{
		System.loadLibrary("freetype2-static");
		System.loadLibrary("app_plex_main");
	}

    private static final Pattern DIR_SEPARATOR = Pattern.compile("/");

    /**
     * Raturns all available SD-Cards in the system (include emulated)
     *
     * Warning: Hack! Based on Android source code of version 4.3 (API 18)
     * Because there is no standart way to get it.
     * TODO: Test on future Android versions 4.4+
     *
     * @return paths to all available SD-Cards in the system (include emulated)
     */
    public static String[] getStorageDirectories()
    {
        // Final set of paths
        final Set<String> rv = new HashSet<String>();
        // Primary physical SD-CARD (not emulated)
        final String rawExternalStorage = System.getenv("EXTERNAL_STORAGE");
        // All Secondary SD-CARDs (all exclude primary) separated by ":"
        final String rawSecondaryStoragesStr = System.getenv("SECONDARY_STORAGE");
        // Primary emulated SD-CARD
        final String rawEmulatedStorageTarget = System.getenv("EMULATED_STORAGE_TARGET");

        if(TextUtils.isEmpty(rawEmulatedStorageTarget))
        {
            // Device has physical external storage; use plain paths.
            if(TextUtils.isEmpty(rawExternalStorage))
            {
                // EXTERNAL_STORAGE undefined; falling back to default.
                rv.add("/storage/sdcard0");
            }
            else
            {
                rv.add(rawExternalStorage);
            }
        }
        else
        {
            // Device has emulated storage; external storage paths should have
            // userId burned into them.
            final String rawUserId;

            if(Build.VERSION.SDK_INT < Build.VERSION_CODES.JELLY_BEAN_MR1)
            {
                rawUserId = "";
            }
            else
            {
                final String path = Environment.getExternalStorageDirectory().getAbsolutePath();
                final String[] folders = DIR_SEPARATOR.split(path);
                final String lastFolder = folders[folders.length - 1];
                boolean isDigit = false;

                try
                {
                    Integer.valueOf(lastFolder);
                    isDigit = true;
                }
                catch(NumberFormatException ignored)
                {
                    ignored.printStackTrace();
                }

                rawUserId = isDigit ? lastFolder : "";
            }
            // /storage/emulated/0[1,2,...]
            if(TextUtils.isEmpty(rawUserId))
            {
                rv.add(rawEmulatedStorageTarget);
            }
            else
            {
                rv.add(rawEmulatedStorageTarget + File.separator + rawUserId);
            }
        }
        // Add all secondary storages
        if(!TextUtils.isEmpty(rawSecondaryStoragesStr))
        {
            // All Secondary SD-CARDs splited into array
            final String[] rawSecondaryStorages = rawSecondaryStoragesStr.split(File.pathSeparator);

            Collections.addAll(rv, rawSecondaryStorages);
        }

        return rv.toArray(new String[rv.size()]);
    }

	public main_glsv main_glsv_inst;
	public InputMethodManager	ApplicationInputManager;
	RelativeLayout				rl					= null;

	public static String		sInternalDirectory	= null;

	public static String		PACKAGE_NAME;
	static String apk_file_path = null;
	float original_screen_brightness = 0.f;

	static main inst()
	{
		return instance;
	}

	@Override
	protected void onCreate(Bundle i_saved_instance_state)
	{
        Log.i("activity_life_cycle", "main.onCreate()");

		instance = this;

		super.onCreate(i_saved_instance_state);

        original_screen_brightness = get_screen_brightness();

        String prv_dir = getFilesDir().getPath();
        String tmp_dir = getCacheDir().getPath();
        native_set_prv_tmp_dirs(prv_dir, tmp_dir);

		PACKAGE_NAME = getApplicationContext().getPackageName();
		ApplicationInfo appInfo = null;
		PackageManager packMgmr = this.getPackageManager();
		
		try
		{
			appInfo = packMgmr.getApplicationInfo(PACKAGE_NAME, 0);
		}
		catch (NameNotFoundException e)
		{
			e.printStackTrace();
			throw new RuntimeException("Unable to locate assets, aborting...");
		}
		
		apk_file_path = appInfo.sourceDir;
        int window_flags = WindowManager.LayoutParams.FLAG_FULLSCREEN;
        int window_mask = WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD |
                WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED | WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON;
            window_flags |= WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD | WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED |
                    WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON;

		if (i_saved_instance_state == null)
		{
			sInternalDirectory = getFilesDir().getPath();
			rl = new RelativeLayout(this);
            requestWindowFeature(Window.FEATURE_NO_TITLE);
            getWindow().setFlags(window_flags, window_mask);

			getWindow().setBackgroundDrawable(null);

			main_glsv_inst = new main_glsv(this);
			rl.addView(main_glsv_inst);
			setContentView(rl);
			ApplicationInputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		}

        turnScreenOnAndKeyguardOff();
	}

	@Override
	protected void onStart()
	{
		Log.i("activity_life_cycle", "main.onStart()");
		super.onStart();
        createNotificationChannel();
        opensl_conf params = opensl_conf.createInstance(this);
		native_snd_init(params.get_sample_rate(), params.get_buffer_size());
    }

	@Override
	protected void onStop()
	{
		Log.i("activity_life_cycle", "main.onStop()");
		super.onStop();
        native_snd_close();
	}

	/**
	 * You should use the onPause() method to write any persistent data (such as
	 * user edits) to storage. In addition, the method
	 * onSaveInstanceState(Bundle) is called before placing the activity in such
	 * a background state, allowing you to save away any dynamic instance state
	 * in your activity into the given Bundle, to be later received in
	 * onCreate(Bundle) if the activity needs to be re-created. Note that it is
	 * important to save persistent data in onPause() instead of
	 * onSaveInstanceState(Bundle) because the later is not part of the
	 * lifecycle callbacks, so will not be called in every situation as
	 * described in its documentation.
	 */
	@Override
	protected void onPause()
	{
		Log.i("activity_life_cycle", "main.onPause()");
		super.onPause();

        if(main_glsv_inst != null)
        {
            main_glsv_inst.onPause();
        }
	}

	@Override
	protected void onResume()
	{
        Log.i("activity_life_cycle", "main.onResume()");
		super.onResume();

        if(main_glsv_inst != null)
        {
            main_glsv_inst.onResume();
        }
	}

	@Override
	protected void onRestart()
	{
        Log.i("activity_life_cycle", "main.onRestart()");
		super.onRestart();
	}

	@Override
	protected void onDestroy()
	{
		Log.i("activity_life_cycle", "main.onDestroy()");
		super.onDestroy();
        turnScreenOffAndKeyguardOn();

        if(main_glsv_inst != null)
        {
            main_glsv_inst.onDestroy();
        }
	}

    private void hide_system_ui()
    {
        // Enables regular immersive mode.
        // For "lean back" mode, remove SYSTEM_UI_FLAG_IMMERSIVE.
        // Or for "sticky immersive," replace it with SYSTEM_UI_FLAG_IMMERSIVE_STICKY
        View decor_view = getWindow().getDecorView();
        int flags = View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                // Set the content to appear under the system bars so that the
                // content doesn't resize when the system bars hide and show.
                | View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                // Hide the nav bar and status bar
                | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN;
        decor_view.setSystemUiVisibility(flags);
    }

	@Override
	public void onWindowFocusChanged(boolean i_has_focus)
	{
		Log.i("activity_life_cycle", "main.onWindowFocusChanged. hasFocus: " + i_has_focus);
		super.onWindowFocusChanged(i_has_focus);

        if (i_has_focus)
        {
            hide_system_ui();
        }
    }

	@Override
	public void onBackPressed()
	{
		if(native_back_evt())
		{
            back_press();
		}
	}

	@Override
	public void onNewIntent(Intent intent)
	{
        Log.d("activity_life_cycle", "main.intent: " + intent);
		super.onNewIntent(intent);
	}

	private void back_press()
    {
        set_screen_brightness_ui(original_screen_brightness);
        super.onBackPressed();
        // app's gonna crash on restart without this line. todo: fix this issue before release
        System.exit(0);
    }

	public static void exit_application()
    {
        inst().runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                inst().back_press();
            }
        });
    }

    private void createNotificationChannel()
    {
        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            CharSequence name = "channel_name";
            String description = "channel_description";
            String channel_id = "channel_id";
            int importance = NotificationManager.IMPORTANCE_HIGH;
            NotificationChannel channel = new NotificationChannel(channel_id, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    private static void schedule_alarm(int i_alarm_type, long i_trigger_at_millis, PendingIntent i_operation)
    {
        Context ctx = inst();
        AlarmManager alarm_manager = (AlarmManager) ctx.getSystemService(ALARM_SERVICE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            alarm_manager.setExactAndAllowWhileIdle(i_alarm_type, i_trigger_at_millis, i_operation);
        }
        else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
        {
            alarm_manager.setExact(i_alarm_type, i_trigger_at_millis, i_operation);
        }
        else
        {
            alarm_manager.set(i_alarm_type, i_trigger_at_millis, i_operation);
        }
        //Log.i(TAG,"alarm msg " + message + " delay " + delayInSeconds + " ms " + millis + " id " + tag);
    }

    /**
     * This will schedule an alarm, which, when fired by the system, will trigger a notification.
     * @param message Message to display in notification.
     * @param delay_in_seconds Time (in seconds from now) after which the notification is fired.
     * @param tag Identifier for the alarm/notification.
     */
    public static void schedule_wakeup(String message, int delay_in_seconds, int tag)
    {
        Context ctx = inst();
        Intent alarm_intent = new Intent(ctx, AlarmReceiver.class);

        alarm_intent.setAction("mws-alarm");
        alarm_intent.putExtra("type", "wakeup");
        alarm_intent.putExtra("message", message);
        alarm_intent.putExtra("tagId", tag);

        PendingIntent pending_intent = PendingIntent.getBroadcast(ctx, tag, alarm_intent, PendingIntent.FLAG_UPDATE_CURRENT);

        long uptime_millis =  SystemClock.elapsedRealtime();
        long millis = uptime_millis + delay_in_seconds * 1000;
        int alarm_type = AlarmManager.ELAPSED_REALTIME_WAKEUP;

        String msg = "ERW wake app in " + delay_in_seconds + " seconds";
        //native_log(msg);
        schedule_alarm(alarm_type, millis, pending_intent);
    }

	/**
	 * This will schedule an alarm, which, when fired by the system, will trigger a notification.
	 * @param message Message to display in notification.
	 * @param delay_in_seconds Time (in seconds from now) after which the notification is fired.
	 * @param tag Identifier for the alarm/notification.
	 */
	public static void schedule_notification(String message, int delay_in_seconds, int tag)
    {
		Context ctx = inst();
		Intent alarm_intent = new Intent(ctx, AlarmReceiver.class);

        alarm_intent.putExtra("type", "notification");
		alarm_intent.putExtra("message", message);
		alarm_intent.putExtra("tagId", tag);

		PendingIntent pending_intent = PendingIntent.getBroadcast(ctx, tag, alarm_intent, PendingIntent.FLAG_UPDATE_CURRENT);

        long uptime_millis =  SystemClock.elapsedRealtime();
		long millis = uptime_millis + delay_in_seconds * 1000;
        int alarm_type = AlarmManager.ELAPSED_REALTIME_WAKEUP;

        schedule_alarm(alarm_type, millis, pending_intent);
	}

	/**
	 * Cancel all pending alarms/notifications between i_start_tag and i_stop_tag.
	 * @param i_start_tag first identifier for the cancelled alarms/notifications.
	 * @param i_stop_tag last identifier for the cancelled alarms/notifications.
	 */
	public static void cancel_notification_interval(int i_start_tag, int i_stop_tag)
    {
		Context ctx = inst();
		AlarmManager alarmManager = (AlarmManager) ctx.getSystemService(ALARM_SERVICE);
		Intent alarmIntent = new Intent(ctx, AlarmReceiver.class);
		NotificationManager notificationManager = (NotificationManager)ctx.getSystemService(Context.NOTIFICATION_SERVICE);

		// Cancel any pending alarms.
		for (int k = i_start_tag; k <= i_stop_tag; k++)
		{
			PendingIntent pendingIntent = PendingIntent.getBroadcast(ctx, k, alarmIntent, 0);

			if (pendingIntent != null)
			{
                Log.i("alarm","canceling alarm tag " + k);
				alarmManager.cancel(pendingIntent);
			}
		}

		// Cancel any notifications.
		notificationManager.cancelAll();
	}

	/**
	 * Cancel one pending alarm/notification identified by tag.
	 * @param tag Identifier for the cancelled alarm/notification.
	 */
	public static void cancel_notification(int tag)
    {
		Context ctx = inst();
		AlarmManager alarmManager = (AlarmManager) ctx.getSystemService(ALARM_SERVICE);
		Intent alarmIntent = new Intent(ctx, AlarmReceiver.class);
		PendingIntent pendingIntent = PendingIntent.getBroadcast(ctx, tag, alarmIntent, 0);
		NotificationManager notificationManager = (NotificationManager)ctx.getSystemService(Context.NOTIFICATION_SERVICE);

		// Cancel an alarm and notification identified by tag.
		if (pendingIntent != null)
		{
			alarmManager.cancel(pendingIntent);
			notificationManager.cancel(tag);
		}
	}

	// net
    public static boolean is_wifi_multicast_lock_enabled()
    {
        return (multicast_lock != null) && multicast_lock.isHeld();
    }

    public static void set_wifi_multicast_lock_enabled(boolean i_enabled)
    {
        if(i_enabled)
        {
            if(multicast_lock == null)
            {
                Context ctx = inst();
                WifiManager wifi = (WifiManager) ctx.getSystemService(WIFI_SERVICE);

                multicast_lock = wifi.createMulticastLock("multicast_lock");
                multicast_lock.setReferenceCounted(false);
            }

            if(!multicast_lock.isHeld()) { multicast_lock.acquire(); }
        }
        else if(multicast_lock != null)
        {
            multicast_lock.release();
            multicast_lock = null;
        }
    }

    // screen
	public static float get_screen_brightness()
    {
        WindowManager.LayoutParams layoutParams = instance.getWindow().getAttributes();

        return layoutParams.screenBrightness;
    }

    void set_screen_brightness_ui(float i_brightness)
    {
        WindowManager.LayoutParams layoutParams = getWindow().getAttributes();
        layoutParams.screenBrightness = i_brightness;
        getWindow().setAttributes(layoutParams);
    }

    public static void set_screen_brightness(final float i_brightness)
    {
        inst().runOnUiThread(new Runnable()
        {
            @Override
            public void run()
            {
                inst().set_screen_brightness_ui(i_brightness);
            }
        });
    }

    public DisplayMetrics get_display_metrix()
    {
        if(display_metrics == null)
        {
            display_metrics = new DisplayMetrics();
            getWindowManager().getDefaultDisplay().getRealMetrics(display_metrics);
        }

        return display_metrics;
    }

    public static String get_timezone_id()
    {
        String tz_id = TimeZone.getDefault().getID();
        return tz_id;
    }

    private void turnScreenOnAndKeyguardOff()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1)
        {
            setShowWhenLocked(true);
            setTurnScreenOn(true);
        }
        else
        {
            getWindow().addFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                            | WindowManager.LayoutParams.FLAG_ALLOW_LOCK_WHILE_SCREEN_ON
            );
        }

        KeyguardManager keyguardLock = ((KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE));

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
        {
            keyguardLock.requestDismissKeyguard(this, null);
        }
    }

    private void turnScreenOffAndKeyguardOn()
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1)
        {
            setShowWhenLocked(false);
            setTurnScreenOn(false);
        }
        else
        {
            getWindow().clearFlags(
                    WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
                            | WindowManager.LayoutParams.FLAG_ALLOW_LOCK_WHILE_SCREEN_ON
            );
        }
    }

    static native void native_log(String i_msg);

    static native void native_resume();

    static native void native_destroy();

    static native void native_pause();

    // pass asset manager inst, apk path and screen metrix (width, height, horizontal and vertical dpi)
    static native void native_init_renderer(AssetManager i_asset_mgr, String i_apk_path, int i_w, int i_h, float i_xdpi, float i_ydpi);

    static native void native_start_app();

    static native void native_render();

    static native void native_resize(int i_w, int i_h);

    static native void native_touch_event(int[] i_touch_data);

	private static native boolean native_back_evt();
	private static native void native_snd_init(int i_sample_rate, int i_buffer_size);
	private static native void native_snd_close();
	private native void native_set_prv_tmp_dirs(String i_prv_dir, String i_tmp_dir);

    private static WifiManager.MulticastLock multicast_lock = null;
    private static DisplayMetrics display_metrics = null;
	private static main instance = null;
}
