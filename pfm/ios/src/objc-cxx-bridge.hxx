#pragma once

#include <string>
#include <memory>

class gfx_tex;


#ifdef __cplusplus
extern "C"
{
#endif
    
#ifdef UNIT_VIDEO_EDITOR
    
    void cxx_2_objc_open_image_picker();
    void cxx_2_objc_open_video_picker();
	
    std::shared_ptr<gfx_tex> cxx_2_objc_load_tex_by_name(std::string i_filename, std::shared_ptr<gfx> i_gi);
    void objc_2_cxx_set_bg_tex(std::shared_ptr<gfx_tex> i_tex);
    void objc_2_cxx_set_video_path_and_play(std::string i_video_path);
    
#endif
    
#ifdef __cplusplus
}
#endif
