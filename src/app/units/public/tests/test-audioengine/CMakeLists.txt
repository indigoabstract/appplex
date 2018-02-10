target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-audioengine.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/unit-test-audioengine.hpp"
)
