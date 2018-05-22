target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/video-dec.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/video-enc.hpp"
)
