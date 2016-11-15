package com.indigoabstract.appplex;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLDisplay;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

public class mainGLSurfaceView extends GLSurfaceView
{

	// static public boolean allowTouchMove = true;
	private static final int	INVALID_POINTER_ID	= -1;
	private static final int	MOUSE_DOWN			= 0;
	private static final int	MOUSE_UP			= 1;
	private static final int	MOUSE_MOVE			= 2;
	public mainRenderer			mRenderer;

	public mainGLSurfaceView(main context)
	{
		super(context);
		mRenderer = new mainRenderer(context, this);
		setFocusable(true);
		setFocusableInTouchMode(true);
		setKeepScreenOn(true);

		final ActivityManager activityManager = (ActivityManager) this.getContext().getSystemService(Context.ACTIVITY_SERVICE);
		final ConfigurationInfo configurationInfo = activityManager.getDeviceConfigurationInfo();
		final boolean supportsEs3 = configurationInfo.reqGlEsVersion >= 0x30000;

		if (supportsEs3)
		{
			setEGLContextClientVersion(3);
			Log.i("mainGLSurfaceView", "supports es 3.0");
		}

		//setEGLConfigChooser(5, 6, 5, 0, 16, 0);
		//setEGLConfigChooser(8, 8, 8, 8, 24, 0);
		setEGLConfigChooser(new MultisampleConfigChooser());
		//setId(2234);

		//setDebugFlags(DEBUG_CHECK_GL_ERROR);// | DEBUG_LOG_GL_CALLS);
		setRenderer(mRenderer);
		setRenderMode(RENDERMODE_CONTINUOUSLY);
		Log.i("mainGLSurfaceView", "construct()");
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event)
	{
		int action = event.getAction();
		float x;
		float y;

		switch (action & MotionEvent.ACTION_MASK)
		{
			case MotionEvent.ACTION_DOWN:
			{
				x = event.getX();
				y = event.getY();
				mRenderer.nativeTouchEvent(event.getPointerId(0), MOUSE_DOWN, x, y);

				break;
			}

			case MotionEvent.ACTION_CANCEL:
			case MotionEvent.ACTION_UP:
			{
				x = event.getX();
				y = event.getY();
				mRenderer.nativeTouchEvent(event.getPointerId(0), MOUSE_UP, x, y);

				break;
			}

			case MotionEvent.ACTION_MOVE:
			{
				for (int index = event.getPointerCount() - 1; index >= 0; --index)
				{
					int pointer_id = event.getPointerId(index);

					if (pointer_id == INVALID_POINTER_ID)
					{
						continue;
					}

					x = event.getX(index);
					y = event.getY(index);
					mRenderer.nativeTouchEvent(pointer_id, MOUSE_MOVE, x, y);
				}

				break;
			}

			case MotionEvent.ACTION_POINTER_DOWN:
			{
				int pointer_index = (action & MotionEvent.ACTION_POINTER_ID_MASK) >> MotionEvent.ACTION_POINTER_ID_SHIFT;
				int pointer_id = event.getPointerId(pointer_index);

				if (pointer_id != INVALID_POINTER_ID)
				{
					x = (int) event.getX(pointer_index);
					y = (int) event.getY(pointer_index);
					mRenderer.nativeTouchEvent(pointer_id, MOUSE_DOWN, x, y);
				}

				break;
			}

			case MotionEvent.ACTION_POINTER_UP:
			{
				int pointer_index = (action & MotionEvent.ACTION_POINTER_ID_MASK) >> MotionEvent.ACTION_POINTER_ID_SHIFT;
				int pointer_id = event.getPointerId(pointer_index);

				if (pointer_id != INVALID_POINTER_ID)
				{
					x = (int) event.getX(pointer_index);
					y = (int) event.getY(pointer_index);
					mRenderer.nativeTouchEvent(pointer_id, MOUSE_UP, x, y);
				}

				break;
			}

			default:
				break;
		}

		return true;
	}

	/**
	 * KeyCodes sent to FMK are Windows key codes.
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		// //Send Event to FMK but let it send to System also
		// if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN )
		// {
		// mRenderer.nativeKeyboardText(0xAE + 1024, 0, true);
		// return false;
		// }
		// //Send Event to FMK but let it send to System also
		// if (keyCode == KeyEvent.KEYCODE_VOLUME_UP )
		// {
		// mRenderer.nativeKeyboardText(0xAF + 1024, 0, true);
		// return false;
		// }
		//
		//
		//
		// Configuration config = getResources().getConfiguration();
		//
		// if (mRenderer.keyboardShown || config.keyboard !=
		// Configuration.KEYBOARD_NOKEYS)
		// {
		// switch (keyCode)
		// {
		// case KeyEvent.KEYCODE_BACK:
		// {
		//
		// return true;
		// }
		//
		// //case KeyEvent.KEYCODE_CLEAR:
		// case KeyEvent.KEYCODE_DEL:
		// {
		// mRenderer.nativeKeyboardText(8 + 1024, 0, true);
		//
		// return super.onKeyDown(keyCode, event);
		// }
		//
		// case KeyEvent.KEYCODE_ENTER:
		// {
		// mRenderer.nativeKeyboardText(13 + 1024, 0, true);
		//
		// return true;
		// }
		//
		// case KeyEvent.KEYCODE_DPAD_UP:
		// mRenderer.nativeKeyboardText(0x26 + 1024, 0, true);
		// case KeyEvent.KEYCODE_DPAD_DOWN:
		// mRenderer.nativeKeyboardText(0x28 + 1024, 0, true);
		// case KeyEvent.KEYCODE_DPAD_LEFT:
		// mRenderer.nativeKeyboardText(0x25 + 1024, 0, true);
		// case KeyEvent.KEYCODE_DPAD_RIGHT:
		// mRenderer.nativeKeyboardText(0x27 + 1024, 0, true);
		//
		// default: {
		// mRenderer.nativeKeyboardText(0, (int) event.getUnicodeChar(), true);
		//
		// return true;
		// }
		// }
		// }
		// if (keyCode == KeyEvent.KEYCODE_BACK)
		// {
		// mRenderer.nativeKeyboardText(0x08 + 1024, 0, true);
		//
		// return true;
		// }

		return false;
	}

	/**
	 * KeyCodes sent to FMK are Windows key codes.
	 */
	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{

		// //Send Event to FMK but let it send to System also
		// if (keyCode == KeyEvent.KEYCODE_VOLUME_DOWN )
		// {
		// mRenderer.nativeKeyboardText(0xAE + 1024, 0, false);
		// return false;
		// }
		// //Send Event to FMK but let it send to System also
		// if (keyCode == KeyEvent.KEYCODE_VOLUME_UP )
		// {
		// mRenderer.nativeKeyboardText(0xAF + 1024, 0, false);
		// return false;
		// }
		//
		//
		//
		// Configuration config = getResources().getConfiguration();
		//
		// if (mRenderer.keyboardShown || config.keyboard !=
		// Configuration.KEYBOARD_NOKEYS)
		// {
		// switch (keyCode)
		// {
		// case KeyEvent.KEYCODE_BACK:
		// {
		// mRenderer.keyboardShown = false;
		//
		// return true;
		// }
		//
		// //case KeyEvent.KEYCODE_CLEAR:
		// case KeyEvent.KEYCODE_DEL:
		// {
		// mRenderer.nativeKeyboardText(8 + 1024, 0, false);
		//
		// return super.onKeyDown(keyCode, event);
		// }
		//
		// case KeyEvent.KEYCODE_ENTER:
		// {
		// mRenderer.nativeKeyboardText(13 + 1024, 0, false);
		// mRenderer.keyboardShown = false;
		//
		// return true;
		// }
		//
		// case KeyEvent.KEYCODE_DPAD_UP:
		// mRenderer.nativeKeyboardText(0x26 + 1024, 0, false);
		// case KeyEvent.KEYCODE_DPAD_DOWN:
		// mRenderer.nativeKeyboardText(0x28 + 1024, 0, false);
		// case KeyEvent.KEYCODE_DPAD_LEFT:
		// mRenderer.nativeKeyboardText(0x25 + 1024, 0, false);
		// case KeyEvent.KEYCODE_DPAD_RIGHT:
		// mRenderer.nativeKeyboardText(0x27 + 1024, 0, false);
		//
		// default: {
		// mRenderer.nativeKeyboardText(0, (int) event.getUnicodeChar(), false);
		//
		// return true;
		// }
		// }
		// }
		// if (keyCode == KeyEvent.KEYCODE_BACK)
		// {
		// mRenderer.nativeKeyboardText(0x08 + 1024, 0, false);
		//
		// return true;
		// }

		return false;
	}

	@Override
	public void onPause()
	{
		mRenderer.onPause();
		Log.i("mainGLSurfaceView", "onPause()");
	}

	@Override
	public void onResume()
	{
		mRenderer.onResume();
		Log.i("mainGLSurfaceView", "onResume()");
	}

	public void onDestroy(boolean closeAppClean)
	{
		Log.i("mainGLSurfaceView", "onDestroy()");
		mRenderer.appDestroy(closeAppClean);
	}
}

class MultisampleConfigChooser implements GLSurfaceView.EGLConfigChooser
{
	static private final String	kTag	= "GDC11";

	@Override
	public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display)
	{
		boolean multisampling_available = false;
		int red_size = 8;
		int green_size = 8;
		int blue_size = 8;
		int alpha_size = 8;
		int depth_size = 24;
		int stencil_size = 0;
		int egl_sample_count = 2;
		mValue = new int[1];

		// Try to find a normal multisample configuration first.
		int[] configSpec =
		{
				EGL10.EGL_RED_SIZE, red_size, EGL10.EGL_GREEN_SIZE, green_size, EGL10.EGL_BLUE_SIZE, blue_size, EGL10.EGL_ALPHA_SIZE, alpha_size, EGL10.EGL_DEPTH_SIZE, depth_size,
				EGL10.EGL_STENCIL_SIZE, stencil_size,
				// Requires that setEGLContextClientVersion(2) is called on the
				// view.
				EGL10.EGL_RENDERABLE_TYPE, 4 /* EGL_OPENGL_ES2_BIT */, EGL10.EGL_SAMPLE_BUFFERS, 1 /* true */, EGL10.EGL_SAMPLES, egl_sample_count, EGL10.EGL_NONE
		};

		if (!egl.eglChooseConfig(display, configSpec, null, 0, mValue))
		{
			throw new IllegalArgumentException("eglChooseConfig failed");
		}
		int numConfigs = mValue[0];
		Log.w(kTag, "try nr 1. found " + numConfigs + " eglconfigs");

		if (numConfigs > 0)
		{
			multisampling_available = true;
		}
		else
		{
			// No normal multisampling config was found. Try to create a
			// converage multisampling configuration, for the nVidia Tegra2.

			final int EGL_COVERAGE_BUFFERS_NV = 0x30E0;
			final int EGL_COVERAGE_SAMPLES_NV = 0x30E1;

			configSpec = new int[]
			{
					EGL10.EGL_RED_SIZE, red_size, EGL10.EGL_GREEN_SIZE, green_size, EGL10.EGL_BLUE_SIZE, blue_size, EGL10.EGL_DEPTH_SIZE, depth_size, EGL10.EGL_RENDERABLE_TYPE,
					4 /* EGL_OPENGL_ES2_BIT */, EGL_COVERAGE_BUFFERS_NV, 1 /* true */, EGL_COVERAGE_SAMPLES_NV, 2, // always
																												// 5
																												// in
																												// practice
																												// on
																												// tegra
																												// 2
					EGL10.EGL_NONE
			};

			if (!egl.eglChooseConfig(display, configSpec, null, 0, mValue))
			{
				throw new IllegalArgumentException("2nd eglChooseConfig failed");
			}
			numConfigs = mValue[0];
			Log.w(kTag, "try nr2 " + numConfigs + " eglconfigs");

			if (numConfigs <= 0)
			{
				// Give up, try without multisampling.
				configSpec = new int[]
				{
						EGL10.EGL_RED_SIZE, red_size, EGL10.EGL_GREEN_SIZE, green_size, EGL10.EGL_BLUE_SIZE, blue_size, EGL10.EGL_ALPHA_SIZE, alpha_size, EGL10.EGL_DEPTH_SIZE,
						depth_size, EGL10.EGL_STENCIL_SIZE, stencil_size, EGL10.EGL_RENDERABLE_TYPE, 4 /* EGL_OPENGL_ES2_BIT */, EGL10.EGL_NONE
				};

				if (!egl.eglChooseConfig(display, configSpec, null, 0, mValue))
				{
					throw new IllegalArgumentException("3rd eglChooseConfig failed");
				}
				numConfigs = mValue[0];
				Log.w(kTag, "try nr3 " + numConfigs + " eglconfigs");

				if (numConfigs <= 0)
				{
					throw new IllegalArgumentException("No eglconfigs match configSpec");
				}
			}
		}

		// Get all matching configurations.
		EGLConfig[] configs = new EGLConfig[numConfigs];

		if (!egl.eglChooseConfig(display, configSpec, configs, numConfigs, mValue))
		{
			throw new IllegalArgumentException("data eglChooseConfig failed");
		}

		for (int i = 0; i < configs.length; ++i)
		{
			Log.w(kTag, "---");
			Log.w(kTag, "---eglconfig index: " + i);
			Log.w(kTag, "EGL_RED_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_RED_SIZE, 0));
			Log.w(kTag, "EGL_GREEN_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_GREEN_SIZE, 0));
			Log.w(kTag, "EGL_BLUE_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_BLUE_SIZE, 0));
			Log.w(kTag, "EGL_ALPHA_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_ALPHA_SIZE, 0));
			Log.w(kTag, "EGL_DEPTH_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_DEPTH_SIZE, 0));
			Log.w(kTag, "EGL_STENCIL_SIZE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_STENCIL_SIZE, 0));
			Log.w(kTag, "EGL_RENDERABLE_TYPE: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_RENDERABLE_TYPE, 0));
			Log.w(kTag, "EGL_SAMPLE_BUFFERS: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_SAMPLE_BUFFERS, 0));
			Log.w(kTag, "EGL_SAMPLES: " + findConfigAttrib(egl, display, configs[i], EGL10.EGL_SAMPLES, 0));
			Log.w(kTag, "---");
		}

		// CAUTION! eglChooseConfigs returns configs with higher bit depth
		// first: Even though we asked for rgb565 configurations, rgb888
		// configurations are considered to be "better" and returned first.
		// You need to explicitly filter the data returned by eglChooseConfig!
		int index = -1;

		if (multisampling_available)
		// filter by number of samples
		{
			int min_sample_count = 0xffff;

			for (int i = 0; i < configs.length; ++i)
			{
				int sample_count = findConfigAttrib(egl, display, configs[i], EGL10.EGL_SAMPLES, 0);

				if (sample_count < min_sample_count)
				{
					min_sample_count = sample_count;
					index = i;
					break;
				}
			}
		}
		else
		// since multisampling isn't available, pick the first entry
		{
			index = 0;
		}

		if (index == -1)
		{
			index = 0;
			Log.w(kTag, "Did not find sane eglconfig, using first");
		}

		EGLConfig config = configs.length > 0 ? configs[index] : null;

		if (config == null)
		{
			throw new IllegalArgumentException("No eglconfig chosen");
		}

		Log.w(kTag, "finished eglconfig. " + configs.length + " matching configs found. selected index in list: " + index);

		return config;
	}

	private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue)
	{
		if (egl.eglGetConfigAttrib(display, config, attribute, mValue))
		{
			return mValue[0];
		}
		return defaultValue;
	}

	private int[]	mValue;
}
