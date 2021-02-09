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


public class main_glsv extends GLSurfaceView
{
    public main_glsv(main context)
	{
		super(context);
        Log.i("activity_life_cycle", "main_glsv.construct()");

		main_rend_inst = new main_rend(context, this);
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

		//setDebugFlags(DEBUG_CHECK_GL_ERROR);// | DEBUG_LOG_GL_CALLS);
		setRenderer(main_rend_inst);
		setRenderMode(RENDERMODE_CONTINUOUSLY);
        setPreserveEGLContextOnPause(true);
    }

	@Override
	public boolean onTouchEvent(final MotionEvent event)
	{
		int action = event.getAction();
		int flags = action & MotionEvent.ACTION_MASK;
		final int touch_type;

		switch (flags)
		{
			case MotionEvent.ACTION_DOWN:
			case MotionEvent.ACTION_POINTER_DOWN:
				touch_type = mws_ptr_evt_touch_began;
				break;

			case MotionEvent.ACTION_UP:
			case MotionEvent.ACTION_POINTER_UP:
				touch_type = mws_ptr_evt_touch_ended;
				break;

			case MotionEvent.ACTION_MOVE:
				touch_type = mws_ptr_evt_touch_moved;
				break;

            case MotionEvent.ACTION_OUTSIDE:
			case MotionEvent.ACTION_CANCEL:
				touch_type = mws_ptr_evt_touch_cancelled;
				break;

			default:
				return false;
		}

		int touch_count = event.getPointerCount();
        if (touch_count > mws_ptr_evt_max_touch_points) { touch_count = mws_ptr_evt_max_touch_points; }
		final int pointer_index = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
		int idx = 0;

        touch_data[idx++] = touch_type;
        touch_data[idx++] = touch_count;

        for (int k = 0; k < touch_count; k++)
        {
            touch_data[idx++] = event.getPointerId(k);
            touch_data[idx++] = Float.floatToRawIntBits(event.getX(k));
            touch_data[idx++] = Float.floatToRawIntBits(event.getY(k));
            touch_data[idx++] = (k == pointer_index) ? 1 : 0;
        }

        main.native_touch_event(touch_data);

		return true;
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event)
	{
		return false;
	}

	@Override
	public boolean onKeyUp(int keyCode, KeyEvent event)
	{
		return false;
	}

	@Override
	public void onPause()
	{
        Log.i("activity_life_cycle", "main_glsv.onPause()");
		main_rend_inst.onPause();
	}

	@Override
	public void onResume()
	{
        Log.i("activity_life_cycle", "main_glsv.onResume()");
		main_rend_inst.onResume();
	}

	public void onDestroy()
	{
		Log.i("activity_life_cycle", "main_glsv.onDestroy()");
		main_rend_inst.appDestroy();
	}

    public main_rend main_rend_inst;

    public static final int mws_ptr_evt_max_touch_points = 8;
    public static final int mws_ptr_evt_touch_invalid = 0;
    public static final int mws_ptr_evt_touch_began = 1;
    public static final int mws_ptr_evt_touch_moved = 2;
    public static final int mws_ptr_evt_touch_ended = 3;
    public static final int mws_ptr_evt_touch_cancelled = 4;
    // type + count + 8 * (id, x, y, is_changed)
    public static final int touch_data_size = 1 + 1 + mws_ptr_evt_max_touch_points * 4;
    private static int[] touch_data = new int[touch_data_size];
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
					EGL10.EGL_RED_SIZE, red_size, EGL10.EGL_GREEN_SIZE, green_size, EGL10.EGL_BLUE_SIZE, blue_size,
                    EGL10.EGL_DEPTH_SIZE, depth_size, EGL10.EGL_RENDERABLE_TYPE, 4 /* EGL_OPENGL_ES2_BIT */,
                    EGL_COVERAGE_BUFFERS_NV, 1 /* true */, EGL_COVERAGE_SAMPLES_NV, 2, EGL10.EGL_NONE
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

    private int[] mValue;
}
