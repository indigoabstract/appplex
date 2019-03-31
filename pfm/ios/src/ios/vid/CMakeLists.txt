target_sources(app_plex_main
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-dec.hxx"
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-enc.hxx"
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-dec.mm"
	"${CMAKE_CURRENT_LIST_DIR}/ios-video-enc.mm"
) 

include("${CMAKE_CURRENT_LIST_DIR}/dec/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/enc/CMakeLists.txt")
