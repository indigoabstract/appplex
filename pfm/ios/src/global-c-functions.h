#pragma once


#ifdef __cplusplus
extern "C"
{
#endif
    
    int render_video_frame_to_fbo(int fb_width, int fb_height, int tex_gl_id);
    void render_video_frame_to_fbo_2();
    
#ifdef __cplusplus
}
#endif
