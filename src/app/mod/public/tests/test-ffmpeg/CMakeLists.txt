target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-ffmpeg.cxx"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/mod-test-ffmpeg.hxx"
)

include("${CMAKE_CURRENT_LIST_DIR}/ffmpeg/CMakeLists.txt")
