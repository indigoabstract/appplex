target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-dec.mm"
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-enc.mm"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-dec.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-enc.hpp"
) 

include("${CMAKE_CURRENT_LIST_DIR}/dec/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/enc/CMakeLists.txt")
