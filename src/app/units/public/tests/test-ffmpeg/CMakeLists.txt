target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-ffmpeg.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-ffmpeg.hpp"
)

include("${CMAKE_CURRENT_LIST_DIR}/ffmpeg/CMakeLists.txt")
