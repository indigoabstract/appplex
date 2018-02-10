target_sources(app_plex_main
    PRIVATE
	"${CMAKE_CURRENT_LIST_DIR}/gap-buffer.cpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-texxed.cpp"
    PUBLIC
	"${CMAKE_CURRENT_LIST_DIR}/gap-buffer.hpp"
	"${CMAKE_CURRENT_LIST_DIR}/unit-texxed.hpp"
)
