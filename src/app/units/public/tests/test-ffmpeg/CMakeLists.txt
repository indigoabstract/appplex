target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-ffmpeg.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-ffmpeg.hxx"
)

include("${CMAKE_CURRENT_LIST_DIR}/ffmpeg/CMakeLists.txt")
