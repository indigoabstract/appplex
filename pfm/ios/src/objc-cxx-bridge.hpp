#pragma once

#include <string>
#include <memory>

class gfx_tex;


#ifdef __cplusplus
extern "C"
{
#endif
    
    void cxx_2_objc_open_image_picker();
    void cxx_2_objc_open_video_picker();
    void objc_2_cxx_set_bg_tex(std::shared_ptr<gfx_tex> i_tex);
    void objc_2_cxx_set_video_path_and_play(std::string i_video_path);
    
#ifdef __cplusplus
}
#endif
