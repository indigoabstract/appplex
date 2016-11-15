package com.indigoabstract.appplex;

import android.annotation.TargetApi;
import android.content.Context;
import android.media.AudioManager;
import android.os.Build;
import android.util.Log;

/**
 * This class illustrates how to query OpenSL config parameters on Jelly Bean MR1 while maintaining
 * backward compatibility with older versions of Android. The trick is to place the new API calls in
 * an inner class that will only be loaded if we're running on JB MR1 or later.
 */
public abstract class opensl_conf
{
    private opensl_conf()
    {
        // Not meant to be instantiated except here.
    }

    /**
     * @param context, e.g., the current activity.
     * @return opensl_conf instance for the given context.
     */
    public static opensl_conf createInstance(Context context)
    {
        boolean gte_jbmr1 = Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1;

        return gte_jbmr1 ? new jelly_bean_mr1_opensl_conf(context) : new default_opensl_conf();
    }

    /**
     * @return The recommended sample rate in Hz.
     */
    public abstract int get_sample_rate();

    /**
     * @return The recommended buffer size in frames.
     */
    public abstract int get_buffer_size();

    // Implementation for Jelly Bean MR1 or later.
    @TargetApi(Build.VERSION_CODES.JELLY_BEAN_MR1)
    private static class jelly_bean_mr1_opensl_conf extends opensl_conf
    {
        private final int sample_rate;
        private final int buffer_size;

        private jelly_bean_mr1_opensl_conf(Context context)
        {
            AudioManager am = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
            // Provide default values in case config lookup fails.
            int sr = 44100;
            int bs = 64;

            try
            {
                // If possible, query the native sample rate and buffer size.
                sr = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_SAMPLE_RATE));
                bs = Integer.parseInt(am.getProperty(AudioManager.PROPERTY_OUTPUT_FRAMES_PER_BUFFER));
            }
            catch (NumberFormatException e)
            {
                Log.w(getClass().getName(), "Failed to read native OpenSL config: " + e);
            }

            sample_rate = sr;
            buffer_size = bs;
        }

        @Override
        public int get_sample_rate()
        {
            return sample_rate;
        }

        @Override
        public int get_buffer_size()
        {
            return buffer_size;
        }
    }

    // Default factory for Jelly Bean or older.
    private static class default_opensl_conf extends opensl_conf
    {
        @Override
        public int get_sample_rate()
        {
            return 44100;
        }

        @Override
        public int get_buffer_size()
        {
            return 64;
        }
    }
}
