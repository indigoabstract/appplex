#pragma once


#ifdef __cplusplus
extern "C"
{
#endif
    
    // this function provides the texture id of the video frame
    void render_video_frame_to_fbo(int fb_width, int fb_height, int tex_gl_id);
    // this function is bound to an fbo at the time of calling
    // anything drawn in it becomes part of the video frame fed to the encoder
    void render_video_frame_to_fbo_2();
    
#ifdef __cplusplus
}
#endif
