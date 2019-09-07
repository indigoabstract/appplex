package com.indigoabstract.appplex;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.content.res.AssetManager;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.inputmethod.InputMethodManager;


enum app_state_types
{
    e_invalid,
    e_created,
    e_running,
    e_paused,
}


public class main_rend implements GLSurfaceView.Renderer
{
	public main_rend(main c, main_glsv m)
	{
        Log.i("activity_life_cycle", "main_rend.construct()");
		main_inst = c;
        main_glsv_inst = m;
		main_inst.ApplicationInputManager = (InputMethodManager) main_inst.getSystemService(Context.INPUT_METHOD_SERVICE);
		asset_manager = main.inst().getAssets();
	}

	public void onSurfaceCreated(GL10 gl, EGLConfig config)
	{
		Log.i("activity_life_cycle", "main_rend.onSurfaceCreated(GL)");
        android.util.DisplayMetrics dm = main.inst().get_display_metrix();

		app_state = app_state_types.e_created;
        main.native_init_renderer(asset_manager, main.apk_file_path, dm.widthPixels, dm.heightPixels, dm.xdpi, dm.ydpi);
	}

	public void onSurfaceChanged(GL10 gl, int w, int h)
	{
        Log.i("activity_life_cycle", "main_rend.onSurfaceChanged(GL): " + w + " , " + h);
        main.native_resize(w, h);

        if(app_state == app_state_types.e_created)
        {
            app_state = app_state_types.e_running;
            main.native_start_app();
        }
	}

	public void onDrawFrame(GL10 gl)
	{
        main.native_render();
	}

	public void onPause()
	{
        Log.i("activity_life_cycle", "main_rend.onPause()");

        if(app_state != app_state_types.e_paused)
        {
            app_state = app_state_types.e_paused;
            main.native_pause();
        }
	}

	public void onResume()
	{
		Log.i("activity_life_cycle", "main_rend.onResume()");

        if(app_state == app_state_types.e_paused)
        {
            app_state = app_state_types.e_running;
            main.native_resume();
        }
	}

	public void appDestroy()
	{
		Log.i("activity_life_cycle", "main_rend.appDestroy()");
		main.native_destroy();
        app_state = app_state_types.e_invalid;

		if (main_inst.isFinishing() == false)
		{
			main_inst.finish();
			Log.i("main_rend", "Activity finish request");
		}
	}

    static main main_inst;
    main_glsv main_glsv_inst;
    private AssetManager asset_manager;
    private app_state_types app_state = app_state_types.e_invalid;
}
