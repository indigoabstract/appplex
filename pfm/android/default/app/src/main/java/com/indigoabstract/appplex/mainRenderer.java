package com.indigoabstract.appplex;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Display;
import android.view.inputmethod.InputMethodManager;

public class mainRenderer implements GLSurfaceView.Renderer
{
	static main				main_inst;
	public static String	m_sDownloadPath;
	mainGLSurfaceView		mGlSurfaceView;
	Display					d;
	TextWatcher				textListener;
	ApplicationInfo			appInfo		= null;

	public mainRenderer(main c, mainGLSurfaceView m)
	{
		main_inst = c;
		mGlSurfaceView = m;
		main_inst.ApplicationInputManager = (InputMethodManager) main_inst.getSystemService(Context.INPUT_METHOD_SERVICE);
		asset_manager = main.inst().getAssets();
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		Log.i("mainRenderer", "onSurfaceCreated(GL)");

		nativeInitRenderer(asset_manager, main.apkFilePath);
	}

	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
		//android.os.Debug.waitForDebugger();
		nativeResize(w, h);
		nativeResume();

		nativeStartApp();

		Log.i("mainRenderer", "onSurfaceChanged(GL): " + w + " , " + h);
	}

	public void onDrawFrame(GL10 gl)
	{
		//gl.glClearColor(0, 0, 1, 1);
		//gl.glClear(gl.GL_COLOR_BUFFER_BIT);
		nativeRender();
		// mGlSurfaceView.requestRender();
		//Log.i("mainRenderer", "onDrawFrame");
	}

	public void onPause()
	{
		nativePause();
		Log.i("mainRenderer", "onPause()");
	}

	public void onResume()
	{
		Log.i("mainRenderer", "onResume() native");

		nativeResume();
	}

	public void appDestroy(boolean closeAppClean)
	{
		/**
		 * This can be called from both java and C, but no matter what they will
		 * do a notifyDestroy in C code to be sure this wasn't forced.
		 */
		Log.i("mainRenderer", "appDestroy() " + closeAppClean);

		nativeDestroy(); // the application isn't closed from C code, but
							// somehow forced by the user
		closeAppClean = true; // we hope to do clean native destroy!

		if (((Activity) main_inst).isFinishing() == false)
		{
			((Activity) main_inst).finish();
			Log.i("mainRenderer", "Activity finish request");
		}

		Log.i("mainRenderer", "appDestroy()");
	}

	public native void nativeResume();

	public native void nativeDestroy();

	public native void nativePause();

	public native void nativeInitRenderer(AssetManager iasset_manager, String apk_path);

	public native void nativeStartApp();

	public native void nativeRender();

	public native void nativeResize(int x, int y);

	public native void native_touch_event
			(
					int itouch_type, int itouch_count, int[] itouch_points_identifier,
					boolean[] itouch_points_is_changed, float[] itouch_points_x, float[] itouch_points_y
			);

	public static void ExitApplication()
	{
		main_inst.finish();
	}
	
	private AssetManager asset_manager = null;
}
