package com.indigoabstract.appplex;

import android.app.Activity;
import android.app.AlarmManager;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.SystemClock;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

import java.io.File;
import java.util.Calendar;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import java.util.regex.Pattern;

// class Loader
// {
    // private static boolean done = false;

    // protected static synchronized void load()
	// {
        // if (done)
            // return;

        // System.loadLibrary("library_name");

        // done = true;
    // }
// }
public class main extends Activity
{
	static
	{
		System.loadLibrary("zip");
		System.loadLibrary("png_renamed");
		System.loadLibrary("freetype2-static");
		System.loadLibrary("ffmpeg");
		System.loadLibrary("app_plex_main");
	}

    private static final Pattern DIR_SEPORATOR = Pattern.compile("/");

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
                final String[] folders = DIR_SEPORATOR.split(path);
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

	public mainGLSurfaceView	mGLView;
	public LinearLayout			ApplicationTextView;
	public FrameLayout			ApplicationLayout;
	public EditText				ApplicationEditText;
	public InputMethodManager	ApplicationInputManager;
	RelativeLayout				rl					= null;
	public static Handler		mainHandler			= new Handler();

	public static long			lastTimeResumed		= -1;
	public static String		sInternalDirectory	= null;

	public static String		PACKAGE_NAME;
	static String apkFilePath	= null;

	static main inst()
	{
		return instance;
	}
	atest ainst;
	
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		ainst = new atest();
		instance = this;
        Log.i("main", "onCreate()");

		super.onCreate(savedInstanceState);

		// test getting external storage directories
        File primaryExtSd=Environment.getExternalStorageDirectory();
        File[] files1 = primaryExtSd.listFiles();
        File parentDir=new File(primaryExtSd.getParent());
        File[] files2 = parentDir.listFiles();

        String[] dirs = getStorageDirectories();
		File ff=new File(dirs[0]);
		File[] files3 = ff.listFiles();
//        File[][] listf = new File[dirs.length][];
//
//        for (int k = 0; k < dirs.length; k++)
//        {
//            File f = new File(dirs[k]);
//            File[] lst = f.listFiles();
//            listf[k] = lst;
//        }

        String files_dir = getFilesDir().getPath();
		File file = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_PICTURES), "dk");
		String str = file.getPath();
		native_set_writable_path(files_dir);
//		if(dirs != null && dirs.length > 1)
//		{
//			native_set_writable_path(dirs[1]);
//		}

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
		
		apkFilePath = appInfo.sourceDir;
		
		if (savedInstanceState == null)
		{

			sInternalDirectory = getFilesDir().getPath();
			rl = new RelativeLayout(this);
			//rl.setId(4322);

			requestWindowFeature(Window.FEATURE_NO_TITLE);
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

			getWindow().setBackgroundDrawable(null);

			mGLView = new mainGLSurfaceView(this);
			//mGLView.setId(2234);

			// mGLView.setFocusable(true);
			// mGLView.setFocusableInTouchMode(true);

			// setContentView(mGLView);
			rl.addView(mGLView);

			// invisible edit text
			ApplicationEditText = new EditText(this);
			ApplicationEditText.setWidth(0);
			ApplicationEditText.setHeight(0);
			ApplicationEditText.setVisibility(View.INVISIBLE);

			// for text
			ApplicationTextView = new LinearLayout(this);
			ApplicationTextView.addView(ApplicationEditText);
			// The application text view holds editText object

			// rl.addView(ApplicationTextView);

			setContentView(rl);

			// mGLView.setFocusable(true);
			// mGLView.setFocusableInTouchMode(true);
			ApplicationInputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		}
	}

	public void startNLTHandler()
	{
	}

	@Override
	protected void onStart()
	{
		Log.i("main", "onStart()");
		super.onStart();
        opensl_conf params = opensl_conf.createInstance(this);
		native_snd_init(params.get_sample_rate(), params.get_buffer_size());

        WindowManager.LayoutParams layoutParams = getWindow().getAttributes();
        layoutParams.screenBrightness = 1.f;
        getWindow().setAttributes(layoutParams);
    }

	@Override
	protected void onStop()
	{
		Log.i("main", "onStop()");
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
		Log.i("main", "onPause()");
		super.onPause();
		if (mGLView == null) return;
		// if (mGLView.mRenderer.m_bNativeRunning == false)
		// {
		// return;
		// }

		mGLView.onPause();
	}

	@Override
	protected void onResume()
	{
		super.onResume();
		/**
		 * Orientation blocked in landscape
		 */
		// setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		if (mGLView == null)
		{
			Log.e("main", "NULL glview");
			return;
		}
		mGLView.onResume();
		Log.i("main", "onResume()");

		Display display = getWindowManager().getDefaultDisplay();
		mGLView.mRenderer.d = display; // av should see if there is any problem
										// with this
	}

	@Override
	protected void onRestart()
	{
		super.onRestart();
		Log.i("main", "onRestart()");
	}

	@Override
	protected void onDestroy()
	{
		Log.i("main", "onDestroy()");
		super.onDestroy();
		if (mGLView == null) return;
		// if (mGLView.mRenderer.m_bNativeRunning == true) {
		// mGLView.onDestroy(false);
		// }
	}

	public void ReinitDisplay(boolean hasFocus)
	{
		Log.i("main", "onWindowFocusChanged START : " + hasFocus);
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus)
		{
			Log.i("main", "onWindowFocusChanged - onResume()");

			if (mGLView == null) return;
			mGLView.onResume();
			lastTimeResumed = System.currentTimeMillis();
			Display display = getWindowManager().getDefaultDisplay();
			mGLView.mRenderer.d = display; // av should see if there is any
											// problem with this

			Log.i("Display", "Initialized");
			try
			{
				Thread.sleep(50);
			}
			catch (InterruptedException e)
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		else
		{
			Log.i("main", "onWindowFocusChanged - onPause()");
			if (mGLView == null) return;
			// if(mGLView.mRenderer.m_bNativeRunning == false)
			// return;
			// //mGLView.onPause();
		}
	}

	@Override
	public void onWindowFocusChanged(boolean hasFocus)
	{
		Log.i("main", "onWindowFocusChanged START : " + hasFocus);
		super.onWindowFocusChanged(hasFocus);
		if (hasFocus)
		{

			if (mGLView == null) return;
			mGLView.onResume();
			lastTimeResumed = System.currentTimeMillis();
			Display display = getWindowManager().getDefaultDisplay();
			mGLView.mRenderer.d = display; // av should see if there is any
											// problem with this

			Log.i("Display", "Initialized");
			try
			{
				Thread.sleep(50);
			}
			catch (InterruptedException e)
			{
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

		}
		else
		{
			Log.i("main", "onWindowFocusChanged - onPause()");
			if (mGLView == null) return;
			// if(mGLView.mRenderer.m_bNativeRunning == false)
			// return;
			// mGLView.onPause();
		}
	}

	@Override
	public void onBackPressed()
	{
		if(native_back_evt())
		{
			super.onBackPressed();
			System.exit(0);
		}
	}

	@Override
	public void onNewIntent(Intent intent)
	{
		super.onNewIntent(intent);
		Log.d("main", "intent: " + intent);
	}

	/**
	 * This will schedule an alarm, which, when fired by the system, will trigger a notification.
	 * @param message Message to display in notification.
	 * @param delayInSeconds Time (in seconds from now) after which the notification is fired.
	 * @param tag Identifier for the alarm/notification.
	 */
	public static void schedule_notification(String message, int delayInSeconds, int tag){
		Context ctx = inst();
		AlarmManager alarm_manager = (AlarmManager) ctx.getSystemService(ALARM_SERVICE);
		Intent alarm_intent = new Intent(ctx, AlarmReceiver.class);

		alarm_intent.putExtra("message", message);
		alarm_intent.putExtra("tagId", tag);

		PendingIntent pending_intent = PendingIntent.getBroadcast(ctx, tag, alarm_intent, PendingIntent.FLAG_UPDATE_CURRENT);

        long uptime_millis =  SystemClock.elapsedRealtime();
		long millis = uptime_millis + delayInSeconds * 1000;
        int alarm_type = AlarmManager.ELAPSED_REALTIME_WAKEUP;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            alarm_manager.setExactAndAllowWhileIdle(alarm_type, millis, pending_intent);
        }
        else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT)
        {
            alarm_manager.setExact(alarm_type, millis, pending_intent);
        }
        else
        {
            alarm_manager.set(alarm_type, millis, pending_intent);
        }
        //Log.i(TAG,"alarm msg " + message + " delay " + delayInSeconds + " ms " + millis + " id " + tag);
	}

	/**
	 * Cancel all pending alarms/notifications between startId and endId.
	 * @param startId First identifier for the cancelled alarms/notifications.
	 * @param endId Last identifier for the cancelled alarms/notifications.
	 */
	public static void cancel_all_notifications(int startId, int endId){
		Context ctx = inst();
		AlarmManager alarmManager = (AlarmManager) ctx.getSystemService(ALARM_SERVICE);
		Intent alarmIntent = new Intent(ctx, AlarmReceiver.class);
		NotificationManager notificationManager = (NotificationManager)ctx.getSystemService(Context.NOTIFICATION_SERVICE);

		// Cancel any pending alarms.
		for (int k = startId; k < endId; k++)
		{
			PendingIntent pendingIntent = PendingIntent.getBroadcast(ctx, k, alarmIntent, 0);

			if (pendingIntent != null)
			{
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
	public static void cancel_notification(int tag){
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

	public static int get_screen_dpi()
    {
        if(inst().display_metrics == null)
        {
            inst().display_metrics = new DisplayMetrics();
            inst().getWindowManager().getDefaultDisplay().getRealMetrics(inst().display_metrics);
        }

        return inst().display_metrics.densityDpi;
    }

	private static native boolean native_back_evt();
	private static native void native_snd_init(int isample_rate, int ibuffer_size);
	private static native void native_snd_close();
	private native void native_set_writable_path(String iwritable_path);

    private DisplayMetrics display_metrics;
	private static main instance = null;
}
