target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/mod-stubs.cxx"
) 
include("${CMAKE_CURRENT_LIST_DIR}/prv/CMakeLists.txt")
include("${CMAKE_CURRENT_LIST_DIR}/pub/CMakeLists.txt")
